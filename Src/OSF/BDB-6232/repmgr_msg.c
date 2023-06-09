/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/db_page.h"
#include "dbinc/btree.h"
#include "dbinc/txn.h"
#include "dbinc_auto/repmgr_auto.h"

static int dispatch_app_message(ENV *, REPMGR_MESSAGE *);
static int finish_gmdb_update(ENV *, DB_THREAD_INFO *, DBT *, uint32, uint32, uint32, __repmgr_member_args *);
static int incr_gm_version(ENV *, DB_THREAD_INFO *, DB_TXN *);
static void marshal_site_data(ENV *, uint32, uint32, uint8 *, DBT *);
static void marshal_site_key(ENV *, repmgr_netaddr_t *, uint8 *, DBT *, __repmgr_member_args *);
static int message_loop(ENV *, REPMGR_RUNNABLE *);
static int preferred_master_takeover(ENV *);
static int process_message(ENV*, DBT*, DBT*, int);
static int reject_fwd(ENV *, REPMGR_CONNECTION *);
static int rejoin_connections(ENV *);
static int rejoin_deferred_election(ENV *);
static int rescind_pending(ENV *, DB_THREAD_INFO *, int, uint32, uint32);
static int resolve_limbo_int(ENV *, DB_THREAD_INFO *);
static int resolve_limbo_wrapper(ENV *, DB_THREAD_INFO *);
static int send_permlsn(ENV *, uint32, DB_LSN *);
static int send_permlsn_conn(ENV *, REPMGR_CONNECTION *, uint32, DB_LSN *);
static int serve_join_request(ENV *, DB_THREAD_INFO *, REPMGR_MESSAGE *);
static int serve_lsnhist_request(ENV *, DB_THREAD_INFO *, REPMGR_MESSAGE *);
static int serve_readonly_master_request(ENV *, REPMGR_MESSAGE *);
static int serve_remove_request(ENV *, DB_THREAD_INFO *, REPMGR_MESSAGE *);
static int serve_repmgr_request(ENV *, REPMGR_MESSAGE *);
static int serve_restart_client_request(ENV *, REPMGR_MESSAGE *);
/*
 * Map one of the phase-1/provisional membership status values to its
 * corresponding ultimate goal status: if "adding", the goal is to be fully
 * "present".  Otherwise ("deleting") the goal is to not even appear in the
 * database at all (0).
 */
#define NEXT_STATUS(s) (uint32)((s) == SITE_ADDING ? SITE_PRESENT : 0)

/*
 * PUBLIC: void *__repmgr_msg_thread __P((void *));
 */
void * __repmgr_msg_thread(void * argsp)
{
	int ret;
	REPMGR_RUNNABLE * th = (REPMGR_RUNNABLE *)argsp;
	ENV * env = th->env;
	if((ret = message_loop(env, th)) != 0) {
		__db_err(env, ret, "message thread failed");
		(void)__repmgr_thread_failure(env, ret);
	}
	return (NULL);
}

static int message_loop(ENV *env, REPMGR_RUNNABLE * th)
{
	DB_REP * db_rep;
	DB_THREAD_INFO * ip;
	REP * rep;
	REPMGR_MESSAGE * msg;
	REPMGR_CONNECTION * conn;
	REPMGR_SITE * site;
	__repmgr_permlsn_args permlsn;
	int incremented, ret, t_ret;
	uint32 membership;
	COMPQUIET(membership, 0);
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ENV_ENTER(env, ip);
	LOCK_MUTEX(db_rep->mutex);
	while((ret = __repmgr_queue_get(env, &msg, th)) == 0) {
		incremented = FALSE;
		if(IS_DEFERRABLE(msg->msg_hdr.type)) {
			/*
			 * Count threads currently processing channel requests
			 * or GMDB operations, so that we can limit the number
			 * of them, in order to avoid starving more important
			 * rep messages.
			 */
			db_rep->non_rep_th++;
			incremented = TRUE;
		}
		if(msg->msg_hdr.type == REPMGR_REP_MESSAGE) {
			DB_ASSERT(env, IS_VALID_EID(msg->v.repmsg.originating_eid));
			site = SITE_FROM_EID(msg->v.repmsg.originating_eid);
			membership = site->membership;
		}
		UNLOCK_MUTEX(db_rep->mutex);

		switch(msg->msg_hdr.type) {
			case REPMGR_REP_MESSAGE:
			    if(membership != SITE_PRESENT)
				    break;
			    while((ret = process_message(env,
				&msg->v.repmsg.control, &msg->v.repmsg.rec,
				msg->v.repmsg.originating_eid)) == DB_LOCK_DEADLOCK)
				    RPRINT(env, (env, DB_VERB_REPMGR_MISC,
					"repmgr deadlock retry"));
			    break;
			case REPMGR_APP_MESSAGE:
			    ret = dispatch_app_message(env, msg);
			    conn = msg->v.appmsg.conn;
			    if(conn != NULL) {
				    LOCK_MUTEX(db_rep->mutex);
				    t_ret = __repmgr_decr_conn_ref(env, conn);
				    UNLOCK_MUTEX(db_rep->mutex);
				    if(t_ret != 0 && ret == 0)
					    ret = t_ret;
			    }
			    break;
			case REPMGR_OWN_MSG:
			    ret = serve_repmgr_request(env, msg);
			    break;
			case REPMGR_HEARTBEAT:
			    if((ret = __repmgr_permlsn_unmarshal(env, &permlsn, (uint8 *)msg->v.repmsg.control.data, msg->v.repmsg.control.size, NULL)) != 0)
				    ret = DB_REP_UNAVAIL;
			    else if(rep->master_id == db_rep->self_eid) {
				    /*
				     * If a master receives a heartbeat, there
				     * may be a dupmaster.  Resend latest log
				     * message to prompt base replication to
				     * detect it without the need for application
				     * activity.
				     */
				    ret = __rep_flush_int(env);
			    }
			    else if(db_rep->prefmas_pending == prefmasMasterSwitch && IS_PREFMAS_MODE(env) && FLD_ISSET(rep->config, REP_C_PREFMAS_MASTER) &&
				F_ISSET(rep, REP_F_CLIENT)) {
				    RPRINT(env, (env, DB_VERB_REPMGR_MISC, "message_loop heartbeat preferred master switch"));
				    /*
				     * We are a preferred master site currently
				     * running as a client and we have finished
				     * syncing with the temporary master.  It is
				     * now time to take over as master.
				     */
				    db_rep->prefmas_pending = prefmasNoAction;
				    ret = preferred_master_takeover(env);
			    }
			    else {
				    /*
				     * Use heartbeat message to initiate rerequest
				     * processing.
				     */
				    ret = __rep_check_missing(env, permlsn.generation, &permlsn.lsn);
			    }
			    break;
			default:
			    ret = __db_unknown_path(env, "message loop");
			    break;
		}

		__os_free(env, msg);
		LOCK_MUTEX(db_rep->mutex);
		if(incremented)
			db_rep->non_rep_th--;
		if(ret != 0)
			goto out;
		if(db_rep->view_mismatch) {
			__db_errx(env, DB_STR("3699", "Site is not recorded as a view in the group membership database"));
			ret = EINVAL;
			goto out;
		}
	}
	/*
	 * A return of DB_REP_UNAVAIL from __repmgr_queue_get() merely means we
	 * should finish gracefully.
	 */
	if(ret == DB_REP_UNAVAIL)
		ret = 0;
out:
	UNLOCK_MUTEX(db_rep->mutex);
	ENV_LEAVE(env, ip);
	return ret;
}

static int dispatch_app_message(ENV *env, REPMGR_MESSAGE * msg)
{
	DB_REP * db_rep;
	DB_CHANNEL db_channel;
	CHANNEL channel;
	__repmgr_msg_metadata_args meta;
	DBT * dbt, * segment;
	uint32 flags, i, size, * uiptr;
	uint8 * data;
	void * ptr;
	int ret;
	COMPQUIET(size, 0);
	db_rep = env->rep_handle;
	db_channel.channel = &channel;
	db_channel.send_msg = __repmgr_send_response;

	/* Supply stub functions for methods inapplicable in msg disp func. */
	db_channel.close = __repmgr_channel_close_inval;
	db_channel.send_request = __repmgr_send_request_inval;
	db_channel.set_timeout = __repmgr_channel_timeout_inval;

	channel.msg = msg;
	channel.env = env;
	channel.c.conn = msg->v.appmsg.conn;
	channel.responded = FALSE;
	channel.meta = &meta;

	/*
	 * The user data is in a form similar to that of a bulk buffer.
	 * However, there's also our meta-data tacked on to the end of it.
	 * Fortunately, the meta-data is fixed length, so it's easy to peel it
	 * off.
	 *
	 * The user data "bulk buffer" lacks the usual "-1" end-marker.  But
	 * that's OK, because we already know how many segments there are (from
	 * the message header).  Convert this information into the DBT array
	 * that we will pass to the user's function.
	 *
	 * (See the definition of DB_MULTIPLE_INIT for a reminder of the format
	 * of a bulk buffer.)
	 */
	dbt = &msg->v.appmsg.buf;
	data = (uint8 *)dbt->data;
	dbt->size -= __REPMGR_MSG_METADATA_SIZE;
	ret = __repmgr_msg_metadata_unmarshal(env,
		&meta, &data[dbt->size], __REPMGR_MSG_METADATA_SIZE, NULL);
	DB_ASSERT(env, ret == 0);

	dbt->ulen = dbt->size;
	DB_MULTIPLE_INIT(ptr, dbt);
	for(i = 0; i < APP_MSG_SEGMENT_COUNT(msg->msg_hdr); i++) {
		segment = &msg->v.appmsg.segments[i];
		uiptr = (uint32 *)ptr;
		*uiptr = ntohl(*uiptr);
		uiptr[-1] = ntohl(uiptr[-1]);
		DB_MULTIPLE_NEXT(ptr, dbt, data, size);
		DB_ASSERT(env, data != NULL);
		DB_INIT_DBT(*segment, data, size);
	}

	flags = F_ISSET(&meta, REPMGR_REQUEST_MSG_TYPE) ?
	    DB_REPMGR_NEED_RESPONSE : 0;

	if(db_rep->msg_dispatch == NULL) {
		__db_errx(env, DB_STR("3670", "No message dispatch call-back function has been configured"));
		if(F_ISSET(channel.meta, REPMGR_REQUEST_MSG_TYPE))
			return (__repmgr_send_err_resp(env, &channel, DB_NOSERVER));
		else
			return 0;
	}
	(*db_rep->msg_dispatch)(env->dbenv, &db_channel, &msg->v.appmsg.segments[0], APP_MSG_SEGMENT_COUNT(msg->msg_hdr), flags);
	if(F_ISSET(channel.meta, REPMGR_REQUEST_MSG_TYPE) && !channel.responded) {
		__db_errx(env, DB_STR("3671", "Application failed to provide a response"));
		return (__repmgr_send_err_resp(env, &channel, DB_KEYEMPTY));
	}
	return 0;
}

/*
 * PUBLIC: int __repmgr_send_err_resp __P((ENV *, CHANNEL *, int));
 */
int __repmgr_send_err_resp(ENV *env, CHANNEL * channel, int err)
{
	DB_REP * db_rep;
	REPMGR_CONNECTION * conn;
	REPMGR_IOVECS iovecs;
	__repmgr_msg_hdr_args msg_hdr;
	uint8 msg_hdr_buf[__REPMGR_MSG_HDR_SIZE];
	int ret;

	db_rep = env->rep_handle;
	msg_hdr.type = REPMGR_RESP_ERROR;

	/* Make it non-negative, so we can send on wire without worry. */
	DB_ASSERT(env, err < 0);
	RESP_ERROR_CODE(msg_hdr) = (uint32)(-err);

	RESP_ERROR_TAG(msg_hdr) = channel->meta->tag;

	__repmgr_iovec_init(&iovecs);
	__repmgr_msg_hdr_marshal(env, &msg_hdr, msg_hdr_buf);
	__repmgr_add_buffer(&iovecs, msg_hdr_buf, __REPMGR_MSG_HDR_SIZE);

	conn = channel->c.conn;
	LOCK_MUTEX(db_rep->mutex);
	ret = __repmgr_send_many(env, conn, &iovecs, 0);
	UNLOCK_MUTEX(db_rep->mutex);

	return ret;
}

static int process_message(ENV *env, DBT * control, DBT * rec, int eid)
{
	DB_LSN lsn;
	DB_LSN ckp_lsn;
	DB_REP * db_rep;
	REP * rep;
	int dirty, ret, t_ret;
	uint32 generation;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ZERO_LSN(ckp_lsn);
	/*
	 * Save initial generation number, in case it changes in a close race
	 * with a NEWMASTER.
	 */
	generation = rep->gen;

	ret = 0;
	switch(t_ret =
	    __rep_process_message_int(env, control, rec, eid, &lsn, &ckp_lsn)) {
		case 0:
		    if(db_rep->takeover_pending)
			    ret = __repmgr_claim_victory(env);
		    break;

		case DB_REP_HOLDELECTION:
		    LOCK_MUTEX(db_rep->mutex);
		    ret = __repmgr_init_election(env,
			    ELECT_F_IMMED | ELECT_F_INVITEE);
		    UNLOCK_MUTEX(db_rep->mutex);
		    break;

		case DB_REP_DUPMASTER:
		    if(IS_PREFMAS_MODE(env) &&
			FLD_ISSET(rep->config, REP_C_PREFMAS_MASTER)) {
			    /*
			     * The preferred master site must restart as a master
			     * so that it sends out a NEWMASTER to help the client
			     * sync.  It must force a role change so that it
			     * advances its gen even though it is already master.
			     * This is needed if there was a temporary master at
			     * a higher gen that is now restarting as a client.
			     * A client won't process messages from a master at
			     * a lower gen than its own.
			     */
			    ret = __repmgr_repstart(env, DB_REP_MASTER,
				    REP_START_FORCE_ROLECHG);
		    }
		    else if(IS_PREFMAS_MODE(env) &&
			FLD_ISSET(rep->config, REP_C_PREFMAS_CLIENT) &&
			(ret = __repmgr_become_client(env)) == 0) {
			    /*
			     * The preferred master client site must restart as
			     * client without any elections to enable the preferred
			     * master site to preserve its own transactions.  It
			     * uses an election thread to repeatedly perform client
			     * startups so that it will perform its client sync
			     * when the preferred master's gen has caught up.
			     */
			    LOCK_MUTEX(db_rep->mutex);
			    ret = __repmgr_init_election(env,
				    ELECT_F_CLIENT_RESTART);
			    UNLOCK_MUTEX(db_rep->mutex);
		    }
		    else if((ret = __repmgr_become_client(env)) == 0 &&
			FLD_ISSET(rep->config, REP_C_LEASE | REP_C_ELECTIONS)
			== REP_C_ELECTIONS) {
			    /*
			     * Initiate an election if we're configured to be using
			     * elections, but only if we're *NOT* using leases.
			     * When using leases, there is never any uncertainty
			     * over which site is the rightful master, and only the
			     * loser gets the DUPMASTER return code.
			     */
			    LOCK_MUTEX(db_rep->mutex);
			    ret = __repmgr_init_election(env, ELECT_F_IMMED);
			    UNLOCK_MUTEX(db_rep->mutex);
		    }
		    DB_EVENT(env, DB_EVENT_REP_DUPMASTER, NULL);
		    break;

		case DB_REP_ISPERM:
#ifdef  CONFIG_TEST
		    if(env->test_abort == DB_TEST_REPMGR_PERM)
			    VPRINT(env, (env, DB_VERB_REPMGR_MISC,
				"ISPERM: Test hook.  Skip ACK for permlsn [%lu][%lu]",
				(u_long)lsn.file, (u_long)lsn.offset));
#endif
		    DB_TEST_SET(env->test_abort, DB_TEST_REPMGR_PERM);
		    ret = send_permlsn(env, generation, &lsn);
		    /*
		     * If we just applied a checkpoint, send a checkpoint message.
		     * A checkpoint message is a permlsn message whose offset is
		     * zero.
		     *
		     * The idea of checkpoint message is to keep log files that are
		     * needed to perform initial sync.  Suppose we become the new
		     * master, and a client wants to sync with us.  If the common
		     * sync point found with the client is the permlsn message sent
		     * above, the client needs to keep the log file indicated by
		     * ckp_lsn.file to be able to recover to the sync point.  The
		     * message below tells that client to keep that file.
		     *
		     * We need to send this message after the normal permlsn
		     * message above.  On the receiver site, if the above permlsn
		     * is lowest among all sites, we know every site must have
		     * processed the checkpoint so it is safe to use it as a lower
		     * bound.  If the above message is sent after the checkpoint
		     * message, this site might be mistakenly treated as having
		     * the lowest permlsn.  We might use the below message as a
		     * lower bound even if some other site might not have processed
		     * the checkpoint.
		     */
		    if(!IS_ZERO_LSN(ckp_lsn)) {
			    ckp_lsn.offset = 0;
			    if((ret = send_permlsn(env, generation,
				&ckp_lsn)) != 0)
				    return ret;
		    }
		    DB_TEST_RECOVERY_LABEL
		    break;

		case DB_LOCK_DEADLOCK:
		case DB_REP_IGNORE:
		case DB_REP_NEWSITE:
		case DB_REP_NOTPERM:
		    break;

		case DB_REP_JOIN_FAILURE:
		    RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			"repmgr fires join failure event"));
		    DB_EVENT(env, DB_EVENT_REP_JOIN_FAILURE, NULL);
		    break;

		case DB_REP_WOULDROLLBACK:
		    RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			"repmgr fires would-rollback event"));
		    DB_EVENT(env, DB_EVENT_REP_WOULD_ROLLBACK, &lsn);
		    break;

		default:
		    __db_err(env, t_ret, "DB_ENV->rep_process_message");
		    ret = t_ret;
	}

	if(ret != 0)
		goto err;
	LOCK_MUTEX(db_rep->mutex);
	dirty = db_rep->gmdb_dirty;
	db_rep->gmdb_dirty = FALSE;
	UNLOCK_MUTEX(db_rep->mutex);
	if(dirty) {
		if((ret = __op_rep_enter(env, FALSE, FALSE)) != 0)
			goto err;
		ret = __repmgr_reload_gmdb(env);
		t_ret = __op_rep_exit(env);
		if(ret == ENOENT)
			ret = 0;
		else if(ret == DB_DELETED && db_rep->demotion_pending)
			/*
			 * If a demotion is in progress, we want to keep
			 * the repmgr threads instead of bowing out because
			 * they are needed when we rejoin the replication group
			 * immediately as a view.
			 */
			ret = 0;
		else if(ret == DB_DELETED)
			ret = __repmgr_bow_out(env);
		if(t_ret != 0 && ret == 0)
			ret = t_ret;
	}
err:
	return ret;
}

/*
 * Handle replication-related events.  Returns only 0 or DB_EVENT_NOT_HANDLED;
 * no other error returns are tolerated.
 *
 * PUBLIC: int __repmgr_handle_event __P((ENV *, uint32, void *));
 */
int __repmgr_handle_event(ENV *env, uint32 event, void * info)
{
	DB_REP * db_rep = env->rep_handle;
	REP * rep = db_rep->region;
	if(db_rep->selector == NULL) {
		/* Repmgr is not in use, so all events go to application. */
		return (DB_EVENT_NOT_HANDLED);
	}
	switch(event) {
		case DB_EVENT_REP_ELECTED:
		    DB_ASSERT(env, info == NULL);
		    db_rep->takeover_pending = TRUE;

		    /*
		     * The application doesn't really need to see this, because the
		     * purpose of this event is to tell the winning site that it
		     * should call rep_start(MASTER), and in repmgr we do that
		     * automatically.  Still, they could conceivably be curious, and
		     * it doesn't hurt anything to let them know.
		     */
		    break;
		case DB_EVENT_REP_INIT_DONE:
		    /*
		     * An abbreviated internal init doesn't change the gmdb, so
		     * don't mark it dirty in this case.  A dirty gmdb will be
		     * reloaded, which causes problems in some mixed-version
		     * cases when the gmdb needs conversion.
		     */
		    if(db_rep->abbrev_init)
			    db_rep->abbrev_init = FALSE;
		    else
			    db_rep->gmdb_dirty = TRUE;
		    break;
		case DB_EVENT_REP_NEWMASTER:
		    DB_ASSERT(env, info != NULL);

		    /* Application still needs to see this. */
		    break;
		case DB_EVENT_REP_MASTER:
		case DB_EVENT_REP_STARTUPDONE:
		    /*
		     * Detect a rare case where a dupmaster or incomplete gmdb
		     * operation has left the site's gmdb inconsistent with
		     * a view callback definition.  The user would have correctly
		     * defined a view callback and called repmgr_start(), but the
		     * gmdb operation to update this site to a view would have been
		     * incomplete or rolled back.  The site cannot operate in this
		     * inconsistent state, so set an indicator to cause a message
		     * thread to panic and terminate.
		     *
		     * The one exception is during a demotion to view, when
		     * this inconsistency is expected for a short time.
		     */
		    if(IS_VALID_EID(db_rep->self_eid) && PARTICIPANT_TO_VIEW(db_rep, SITE_FROM_EID(db_rep->self_eid)) && !db_rep->demotion_pending)
			    db_rep->view_mismatch = TRUE;

		    /*
		     * In preferred master mode, when the preferred master site
		     * finishes synchronizing with the temporary master it must
		     * prepare to take over as master.  This is detected by the
		     * next heartbeat in a message thread, where the takeover is
		     * actually performed.
		     */
		    if(event == DB_EVENT_REP_STARTUPDONE && IS_PREFMAS_MODE(env) && FLD_ISSET(rep->config, REP_C_PREFMAS_MASTER)) {
			    RPRINT(env, (env, DB_VERB_REPMGR_MISC, "startupdone set preferred master switch"));
			    db_rep->prefmas_pending = prefmasMasterSwitch;
		    }
		    break;
		default:
		    break;
	}
	COMPQUIET(info, NULL);
	return (DB_EVENT_NOT_HANDLED);
}

static int send_permlsn(ENV *env, uint32 generation, DB_LSN * lsn)
{
	DB_REP * db_rep;
	REP * rep;
	REPMGR_CONNECTION * conn;
	REPMGR_SITE * site;
	int ack, bcast, eid, master, policy, ret;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ret = 0;
	master = rep->master_id;
	LOCK_MUTEX(db_rep->mutex);
	/*
	 * If the file number has changed, send it to everyone, regardless of
	 * anything else.  Otherwise, send it to the master if we know a master,
	 * and that master's ack policy requires it.
	 */
	bcast = FALSE;
	if(LOG_COMPARE(lsn, &db_rep->perm_lsn) > 0) {
		if(lsn->file > db_rep->perm_lsn.file) {
			bcast = TRUE;
			VPRINT(env, (env, DB_VERB_REPMGR_MISC,
			    "send_permlsn: broadcast [%lu][%lu]",
			    (u_long)lsn->file, (u_long)lsn->offset));
		}
		db_rep->perm_lsn = *lsn;
	}
	/*
	 * Checkpoint message should be sent to everyone.
	 */
	if(lsn->offset == 0)
		bcast = TRUE;
	if(IS_KNOWN_REMOTE_SITE(master)) {
		site = SITE_FROM_EID(master);
		/*
		 * Use master's ack policy if we know it; use our own if the
		 * master is too old (down-rev) to have told us its policy.
		 */
		policy = site->ack_policy > 0 ?
		    site->ack_policy : rep->perm_policy;
		if(IS_VIEW_SITE(env) || policy == DB_REPMGR_ACKS_NONE ||
		    (IS_PEER_POLICY(policy) && rep->priority == 0))
			ack = FALSE;
		else
			ack = TRUE;
	}
	else {
		site = NULL;
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "dropping ack with no known master"));
		ack = FALSE;
	}

	/*
	 * Send to master first, since we need to send to all its connections.
	 */
	if(site != NULL && (bcast || ack)) {
		if(site->state == SITE_CONNECTED) {
			if((conn = site->ref.conn.in) != NULL &&
			    conn->state == CONN_READY &&
			    (ret = send_permlsn_conn(env,
			    conn, generation, lsn)) != 0)
				goto unlock;
			if((conn = site->ref.conn.out) != NULL &&
			    conn->state == CONN_READY &&
			    (ret = send_permlsn_conn(env,
			    conn, generation, lsn)) != 0)
				goto unlock;
		}
		TAILQ_FOREACH(conn, &site->sub_conns, entries) {
			if((ret = send_permlsn_conn(env,
			    conn, generation, lsn)) != 0)
				goto unlock;
		}
	}
	if(bcast) {
		/*
		 * Send to everyone except the master (since we've already done
		 * that, above).
		 */
		FOR_EACH_REMOTE_SITE_INDEX(eid) {
			if(eid == master)
				continue;
			site = SITE_FROM_EID(eid);
			/*
			 * Send the ack out on primary connections only.
			 */
			if(site->state == SITE_CONNECTED) {
				if((conn = site->ref.conn.in) != NULL &&
				    conn->state == CONN_READY &&
				    (ret = send_permlsn_conn(env,
				    conn, generation, lsn)) != 0)
					goto unlock;
				if((conn = site->ref.conn.out) != NULL &&
				    conn->state == CONN_READY &&
				    (ret = send_permlsn_conn(env,
				    conn, generation, lsn)) != 0)
					goto unlock;
			}
		}
	}

unlock:
	UNLOCK_MUTEX(db_rep->mutex);
	return ret;
}
/*
 * Sends a perm LSN message on one connection, if it needs it.
 *
 * !!! Called with mutex held.
 */
static int send_permlsn_conn(ENV *env, REPMGR_CONNECTION * conn, uint32 generation, DB_LSN * lsn)
{
	DBT control2, rec2;
	__repmgr_permlsn_args permlsn;
	uint8 buf[__REPMGR_PERMLSN_SIZE];
	int ret = 0;
	if(conn->state == CONN_READY) {
		DB_ASSERT(env, conn->version > 0);
		permlsn.generation = generation;
		memcpy(&permlsn.lsn, lsn, sizeof(DB_LSN));
		if(conn->version == 1) {
			control2.data = &permlsn;
			control2.size = sizeof(permlsn);
		}
		else {
			__repmgr_permlsn_marshal(env, &permlsn, buf);
			control2.data = buf;
			control2.size = __REPMGR_PERMLSN_SIZE;
		}
		rec2.size = 0;
		/*
		 * It's hard to imagine anyone would care about a lost ack if
		 * the path to the master is so congested as to need blocking;
		 * so pass "maxblock" argument as 0.
		 */
		if((ret = __repmgr_send_one(env, conn, REPMGR_PERMLSN,
		    &control2, &rec2, 0)) == DB_REP_UNAVAIL)
			ret = __repmgr_bust_connection(env, conn);
	}
	return ret;
}

/*
 * Perform the steps on the preferred master site to take over again as
 * preferred master from a temporary master.  This routine should only be
 * called after the preferred master has restarted as a client and finished
 * a client sync with the temporary master.
 *
 * This routine makes a best effort to wait until all temporary master
 * transactions have been applied on this site before taking over.
 */
static int preferred_master_takeover(ENV *env)
{
	DB_LOG * dblp;
	DB_REP * db_rep;
	LOG * lp;
	REP * rep;
	DB_LSN last_ready_lsn, ready_lsn, sync_lsn;
	u_long usec;
	uint32 gen, max_tries, tries;
	int ret, synced;

	dblp = env->lg_handle;
	lp = (LOG *)dblp->reginfo.primary;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	gen = 0;
	ZERO_LSN(sync_lsn);
	ret = 0;

	if(!IS_PREFMAS_MODE(env))
		return ret;

	/*
	 * Start by making the temporary master a readonly master so that we
	 * can know when we have applied all of its transactions on this
	 * site before taking over.
	 */
	if((ret = __repmgr_make_site_readonly_master(env,
	    1, &gen, &sync_lsn)) != 0)
		return ret;
	DB_ASSERT(env, gen >= rep->gen);

	/*
	 * Make a best effort to wait until this site has all transactions
	 * from the temporary master.  We want to preserve temporary master
	 * transactions, but we can't wait forever.  If we exceed our wait,
	 * we restart this site as preferred master anyway.  This may
	 * sacrifice some temporary master transactions in order to preserve
	 * repgroup write availability.
	 *
	 * We restart the number of tries each time we make progress in
	 * transactions applied, until either we apply through sync_lsn or
	 * we exceed max_tries without progress.
	 */
	if((ret = __repmgr_prefmas_get_wait(env, &max_tries, &usec)) != 0)
		return ret;
	tries = 0;
	synced = 0;
	ZERO_LSN(ready_lsn);
	ZERO_LSN(last_ready_lsn);
	while(!synced && tries < max_tries) {
		__os_yield(env, 0, usec);
		tries++;
		/*
		 * lp->ready_lsn is the next LSN we expect to receive,
		 * which also indicates how much we've applied.  sync_lsn
		 * is the lp->lsn (indicating the next log record expected)
		 * from the other site.
		 */
		MUTEX_LOCK(env, rep->mtx_clientdb);
		ready_lsn = lp->ready_lsn;
		MUTEX_UNLOCK(env, rep->mtx_clientdb);
		if(gen == rep->gen && LOG_COMPARE(&ready_lsn, &sync_lsn) >= 0)
			synced = 1;
		else if(LOG_COMPARE(&ready_lsn, &last_ready_lsn) >= 0) {
			/* We are making progress, restart number of tries. */
			last_ready_lsn = ready_lsn;
			tries = 0;
		}
	}

	/* Restart the remote readonly temporary master as a client. */
	if((ret = __repmgr_restart_site_as_client(env, 1)) != 0)
		return ret;

	/* Restart this site as the preferred master, waiting for
	 * REP_LOCKOUT_MSG.  The NEWCLIENT message sent back from
	 * restarting the other site as client can briefly lock
	 * REP_LOCKOUT_MSG to do some cleanup.  We don't want this
	 * to cause the rep_start_int() call to restart this site
	 * as master to return 0 without doing anything.
	 */
	ret = __repmgr_become_master(env, REP_START_WAIT_LOCKMSG);
	return ret;
}

static int serve_repmgr_request(ENV *env, REPMGR_MESSAGE * msg)
{
	DB_REP * db_rep;
	DBT * dbt;
	DB_THREAD_INFO * ip;
	REPMGR_CONNECTION * conn;
	uint32 mtype;
	int ret, t_ret;

	db_rep = env->rep_handle;
	ENV_GET_THREAD_INFO(env, ip);
	conn = msg->v.gmdb_msg.conn;
	mtype = REPMGR_OWN_MSG_TYPE(msg->msg_hdr);
	switch(mtype) {
		case REPMGR_JOIN_REQUEST:
		    ret = serve_join_request(env, ip, msg);
		    break;
		case REPMGR_LSNHIST_REQUEST:
		    ret = serve_lsnhist_request(env, ip, msg);
		    break;
		case REPMGR_READONLY_MASTER:
		    ret = serve_readonly_master_request(env, msg);
		    break;
		case REPMGR_REJOIN:
		    RPRINT(env, (env, DB_VERB_REPMGR_MISC, "One try at rejoining group automatically"));
		    if((ret = __repmgr_join_group(env)) == DB_REP_UNAVAIL)
			    ret = __repmgr_bow_out(env);
		    else if(ret == 0 && IS_PREFMAS_MODE(env)) {
			    /*
			     * For preferred master mode, we need to get
			     * a "regular" connection to the other site without
			     * calling an election prematurely here.
			     */
			    RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Establishing connections after rejoin"));
			    ret = rejoin_connections(env);
		    }
		    else if(ret == 0 && db_rep->rejoin_pending) {
			    RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Calling deferred election after rejoin"));
			    ret = rejoin_deferred_election(env);
		    }
		    db_rep->rejoin_pending = FALSE;
		    break;
		case REPMGR_REMOVE_REQUEST:
		    ret = serve_remove_request(env, ip, msg);
		    break;
		case REPMGR_RESOLVE_LIMBO:
		    ret = resolve_limbo_wrapper(env, ip);
		    break;
		case REPMGR_RESTART_CLIENT:
		    ret = serve_restart_client_request(env, msg);
		    break;
		case REPMGR_SHARING:
		    dbt = &msg->v.gmdb_msg.request;
		    ret = __repmgr_refresh_membership(env, (uint8 *)dbt->data, dbt->size, (conn == NULL ? DB_REPMGR_VERSION : conn->version));
		    break;
		default:
		    ret = __db_unknown_path(env, "serve_repmgr_request");
		    break;
	}
	if(conn != NULL) {
		/*
		 * A site that removed itself may have already closed its
		 * connections.  Do not return an error and panic if we
		 * can't close the one-shot GMDB connection for a remove
		 * request here.
		 */
		if((t_ret = __repmgr_close_connection(env, conn)) != 0 && ret == 0 && mtype != REPMGR_REMOVE_REQUEST)
			ret = t_ret;
		if((t_ret = __repmgr_decr_conn_ref(env, conn)) != 0 && ret == 0)
			ret = t_ret;
	}
	return ret;
}

/*
 * Attempts to fulfill a remote site's request to join the replication group.
 * Only the master can grant this request, so if we've received this request
 * when we're not the master, we'll send an appropriate failure message instead.
 */
static int serve_join_request(ENV *env, DB_THREAD_INFO * ip, REPMGR_MESSAGE * msg)
{
	DB_REP * db_rep;
	REPMGR_CONNECTION * conn;
	REPMGR_SITE * site;
	DBT * dbt;
	__repmgr_site_info_args site_info;
	__repmgr_v4site_info_args v4site_info;
	uint8 * buf;
	char * host;
	size_t len;
	uint32 membership, status;
	int eid, ret, t_ret;

	db_rep = env->rep_handle;
	COMPQUIET(status, 0);

	conn = msg->v.gmdb_msg.conn;
	DB_ASSERT(env, conn->version > 0 && conn->version <= DB_REPMGR_VERSION);
	dbt = &msg->v.gmdb_msg.request;
	if(conn->version < 5) {
		ret = __repmgr_v4site_info_unmarshal(env, &v4site_info, (uint8 *)dbt->data, dbt->size, NULL);
		site_info.host = v4site_info.host;
		site_info.port = v4site_info.port;
		site_info.status = v4site_info.flags;
		site_info.flags = 0;
	}
	else
		ret = __repmgr_site_info_unmarshal(env, &site_info, (uint8 *)dbt->data, dbt->size, NULL);

	host = (char *)site_info.host.data;
	host[site_info.host.size - 1] = '\0';
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "Request to join group from %s:%u", host, (u_int)site_info.port));

	/*
	 * If the requesting site is already known, get its group membership
	 * status for the 2-site edge case check in hold_master_role().
	 */
	membership = 0;
	if((site = __repmgr_lookup_site(env, host, site_info.port)) != NULL)
		membership = site->membership;
	if((ret =
	    __repmgr_hold_master_role(env, conn, membership)) == DB_REP_UNAVAIL)
		return 0;
	if(ret != 0)
		return ret;

	LOCK_MUTEX(db_rep->mutex);
	if((ret = __repmgr_find_site(env, host, site_info.port, &eid)) == 0) {
		DB_ASSERT(env, eid != db_rep->self_eid);
		site = SITE_FROM_EID(eid);
		status = site->membership;
		/*
		 * Remote site electability is usually exchanged when
		 * a connection is established, but when a new site
		 * joins the repgroup there is a brief gap between the
		 * join and the connection.  Record electability for
		 * the joining site so that we are not overly conservative
		 * about the number of acks we require for a PERM
		 * transaction if the joining site is unelectable.
		 */
		if(FLD_ISSET(site_info.flags, SITE_JOIN_ELECTABLE)) {
			F_SET(site, SITE_ELECTABLE);
			FLD_CLR(site_info.flags, SITE_JOIN_ELECTABLE);
		}
		else
			F_CLR(site, SITE_ELECTABLE);
		F_SET(site, SITE_HAS_PRIO);
	}
	UNLOCK_MUTEX(db_rep->mutex);
	if(ret != 0)
		goto err;

	switch(status) {
		case 0:
		case SITE_ADDING:
		    ret = __repmgr_update_membership(env, ip, eid, SITE_ADDING,
			    site_info.flags);
		    break;
		case SITE_PRESENT:
		    /* Already in desired state. */
		    break;
		case SITE_DELETING:
		    ret = rescind_pending(env,
			    ip, eid, SITE_DELETING, SITE_PRESENT);
		    break;
		default:
		    ret = __db_unknown_path(env, "serve_join_request");
		    break;
	}
	if(ret != 0)
		goto err;

	LOCK_MUTEX(db_rep->mutex);
	ret = __repmgr_marshal_member_list(env, conn->version, &buf, &len);
	UNLOCK_MUTEX(db_rep->mutex);
	if(ret != 0)
		goto err;
	ret = __repmgr_send_sync_msg(env, conn, REPMGR_JOIN_SUCCESS, buf, (uint32)len);
	__os_free(env, buf);

err:
	if((t_ret = __repmgr_rlse_master_role(env)) != 0 && ret == 0)
		ret = t_ret;
	if(ret == DB_REP_UNAVAIL)
		ret = __repmgr_send_sync_msg(env, conn, REPMGR_GM_FAILURE, NULL, 0);
	return ret;
}

static int serve_remove_request(ENV *env, DB_THREAD_INFO * ip, REPMGR_MESSAGE * msg)
{
	DB_REP * db_rep;
	REPMGR_CONNECTION * conn;
	REPMGR_SITE * site;
	DBT * dbt;
	__repmgr_site_info_args site_info;
	__repmgr_v4site_info_args v4site_info;
	char * host;
	uint32 status, type;
	int eid, ret, t_ret;
	COMPQUIET(status, 0);
	db_rep = env->rep_handle;

	conn = msg->v.gmdb_msg.conn;
	DB_ASSERT(env, conn->version > 0 && conn->version <= DB_REPMGR_VERSION);
	dbt = &msg->v.gmdb_msg.request;
	if(conn->version < 5) {
		ret = __repmgr_v4site_info_unmarshal(env, &v4site_info, (uint8 *)dbt->data, dbt->size, NULL);
		site_info.host = v4site_info.host;
		site_info.port = v4site_info.port;
		site_info.status = v4site_info.flags;
		site_info.flags = 0;
	}
	else
		ret = __repmgr_site_info_unmarshal(env, &site_info, (uint8 *)dbt->data, dbt->size, NULL);
	host = (char *)site_info.host.data;
	host[site_info.host.size - 1] = '\0';
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Request to remove %s:%u from group", host, (u_int)site_info.port));
	if((ret = __repmgr_hold_master_role(env, conn, 0)) == DB_REP_UNAVAIL)
		return 0;
	if(ret != 0)
		return ret;
	LOCK_MUTEX(db_rep->mutex);
	if((site = __repmgr_lookup_site(env, host, site_info.port)) == NULL)
		eid = DB_EID_INVALID;
	else {
		eid = EID_FROM_SITE(site);
		status = site->membership;
	}
	UNLOCK_MUTEX(db_rep->mutex);
	if(eid == DB_EID_INVALID) {
		/* Doesn't exist: already been removed. */
		ret = 0;
		goto err;
	}
	else if(eid == db_rep->self_eid) {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "Reject request to remove current master"));
		ret = DB_REP_UNAVAIL;
		goto err;
	}

	switch(status) {
		case 0:
		    /* Already in desired state. */
		    break;
		case SITE_ADDING:
		    ret = rescind_pending(env, ip, eid, SITE_ADDING, 0);
		    break;
		case SITE_PRESENT:
		case SITE_DELETING:
		    ret = __repmgr_update_membership(env, ip, eid, SITE_DELETING,
			    site_info.flags);
		    break;
		default:
		    ret = __db_unknown_path(env, "serve_remove_request");
		    break;
	}
err:
	if((t_ret = __repmgr_rlse_master_role(env)) != 0 && ret == 0)
		ret = t_ret;
	switch(ret) {
		case 0:
		    type = REPMGR_REMOVE_SUCCESS;
		    break;
		case DB_REP_UNAVAIL:
		    type = REPMGR_GM_FAILURE;
		    break;
		default:
		    return ret;
	}
	/*
	 * It is possible when a site removes itself that by now it has
	 * already acted on the first GMDB update and closed its connections.
	 * Do not return an error and panic if we can't send the final
	 * status of the remove operation.
	 */
	if((ret = __repmgr_send_sync_msg(env, conn, type, NULL, 0)) != 0)
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "Problem sending remove site status message %d", ret));
	return 0;
}

/*
 * Serve the REPMGR_RESTART_CLIENT message by restarting this site as a
 * client if it is not already a client.  Always sends back a
 * REPMGR_PREFMAS_SUCCESS message with an empty payload.
 */
static int serve_restart_client_request(ENV *env, REPMGR_MESSAGE * msg)
{
	DB_REP * db_rep;
	REP * rep;
	REPMGR_CONNECTION * conn;
	int ret, t_ret;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ret = 0;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Serving restart_client request"));
	conn = msg->v.gmdb_msg.conn;
	DB_ASSERT(env, conn->version > 0 && conn->version <= DB_REPMGR_VERSION);
	/* No need to read payload - it is just a dummy byte. */
	if(IS_PREFMAS_MODE(env) && !F_ISSET(rep, REP_F_CLIENT))
		ret = __repmgr_become_client(env);
	if((t_ret = __repmgr_send_sync_msg(env, conn,
	    REPMGR_PREFMAS_SUCCESS, NULL, 0)) != 0)
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Problem sending restart client success message %d", ret));
	if(ret == 0 && t_ret != 0)
		ret = t_ret;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Request for restart_client returning %d", ret));
	return ret;
}

/*
 * Serve the REPMGR_READONLY_MASTER message by turning this site into a
 * readonly master.  Always sends back a REPMGR_READONLY_RESPONSE message with
 * a payload containing this site's gen and next LSN expected.  If there are
 * any errors, the gen is 0 and the next LSN is [0,0].
 */
static int serve_readonly_master_request(ENV *env, REPMGR_MESSAGE * msg)
{
	REPMGR_CONNECTION * conn;
	__repmgr_permlsn_args permlsn;
	uint8 buf[__REPMGR_PERMLSN_SIZE];
	int t_ret;
	int ret = 0;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Serving readonly_master request"));
	conn = msg->v.gmdb_msg.conn;
	DB_ASSERT(env, conn->version > 0 && conn->version <= DB_REPMGR_VERSION);
	/* No need to read payload - it is just a dummy byte. */
	if(IS_PREFMAS_MODE(env))
		ret = __rep_become_readonly_master(env, &permlsn.generation, &permlsn.lsn);
	__repmgr_permlsn_marshal(env, &permlsn, buf);
	if((t_ret = __repmgr_send_sync_msg(env, conn,
	    REPMGR_READONLY_RESPONSE, buf, __REPMGR_PERMLSN_SIZE)) != 0)
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Problem sending readonly response message %d", ret));
	if(ret == 0 && t_ret != 0)
		ret = t_ret;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Request for readonly_master returning %d", ret));
	return ret;
}
/*
 * Serve the REPMGR_LSNHIST_REQUEST message by retrieving information from
 * this site's LSN history database for the requested gen.  If the requested
 * gen exists at this site, sends back a REPMGR_LSNHIST_RESPONSE message
 * containing the LSN and timestamp at the requested gen and the LSN for the
 * next gen if that gen exists (next gen LSN is [0,0] if next gen doesn't
 * yet exist at this site.)  Sends back a PREFMAS_FAILURE message if the
 * requested gen does not yet exist at this site or if there are any errors.
 */
static int serve_lsnhist_request(ENV *env, DB_THREAD_INFO * ip, REPMGR_MESSAGE * msg)
{
	REPMGR_CONNECTION * conn;
	DBT * dbt;
	__repmgr_lsnhist_match_args lsnhist_match;
	__rep_lsn_hist_data_args lsnhist_data, next_lsnhist_data;
	__rep_lsn_hist_key_args key;
	uint8 match_buf[__REPMGR_LSNHIST_MATCH_SIZE];
	DB_LSN next_gen_lsn;
	int ret, t_ret;

	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Serving lsnhist request"));
	conn = msg->v.gmdb_msg.conn;
	DB_ASSERT(env, conn->version > 0 && conn->version <= DB_REPMGR_VERSION);
	/* Read lsn_hist_key incoming payload to get gen being requested. */
	dbt = &msg->v.gmdb_msg.request;
	if((ret = __rep_lsn_hist_key_unmarshal(env, &key, (uint8 *)dbt->data, dbt->size, NULL)) != 0)
		return ret;
	if(key.version != REP_LSN_HISTORY_FMT_VERSION) {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "serve_lsnhist_request version mismatch"));
		return 0;
	}

	/*
	 * There's no need to retry if we don't find an lsnhist record for
	 * requested gen.  This site is either a temporary master or a client,
	 * which means that if it doesn't already have an lsnhist record at
	 * this gen, it is highly unlikely to get one in the near future.
	 */
	if((ret = __rep_get_lsnhist_data(env,
	    ip, key.gen, &lsnhist_data)) == 0) {
		if((t_ret = __rep_get_lsnhist_data(env,
		    ip, key.gen + 1, &next_lsnhist_data)) == 0)
			next_gen_lsn = next_lsnhist_data.lsn;
		else
			ZERO_LSN(next_gen_lsn);

		lsnhist_match.lsn = lsnhist_data.lsn;
		lsnhist_match.hist_sec = lsnhist_data.hist_sec;
		lsnhist_match.hist_nsec = lsnhist_data.hist_nsec;
		lsnhist_match.next_gen_lsn = next_gen_lsn;
		__repmgr_lsnhist_match_marshal(env, &lsnhist_match, match_buf);
		if((t_ret = __repmgr_send_sync_msg(env, conn,
		    REPMGR_LSNHIST_RESPONSE, match_buf,
		    __REPMGR_LSNHIST_MATCH_SIZE)) != 0)
			RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			    "Problem sending lsnhist response message %d",
			    ret));
	}
	else if((t_ret = __repmgr_send_sync_msg(env, conn,
	    REPMGR_PREFMAS_FAILURE, NULL, 0)) != 0)
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "Problem sending prefmas failure message %d", ret));

	/* Do not return an error if LSN history record not found. */
	if(ret == DB_NOTFOUND)
		ret = 0;
	if(ret == 0 && t_ret != 0)
		ret = t_ret;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Request for lsnhist returning %d", ret));
	return ret;
}

/*
 * Runs a limbo resolution on a message processing thread, upon request from the
 * send() function when it notices that a user transaction has gotten a perm
 * success.  (It wouldn't work for the user thread to do it in-line.)
 */
static int resolve_limbo_wrapper(ENV *env, DB_THREAD_INFO * ip)
{
	int do_close, ret, t_ret;
	if((ret = __repmgr_hold_master_role(env, NULL, 0)) == DB_REP_UNAVAIL)
		return 0;
	if(ret != 0)
		return ret;
retry:
	if((ret = __repmgr_setup_gmdb_op(env, ip, NULL, 0)) != 0)
		goto rlse;

	/*
	 * A limbo resolution request is merely a "best effort" attempt to
	 * shorten the duration of a pending change.  So if it fails for lack of
	 * acks again, no one really cares.
	 */
	if((ret = resolve_limbo_int(env, ip)) == DB_REP_UNAVAIL) {
		do_close = FALSE;
		ret = 0;
	}
	else
		do_close = TRUE;

	if((t_ret = __repmgr_cleanup_gmdb_op(env, do_close)) != 0 &&
	    ret == 0)
		ret = t_ret;
	if(ret == DB_LOCK_DEADLOCK || ret == DB_LOCK_NOTGRANTED)
		goto retry;
rlse:
	if((t_ret = __repmgr_rlse_master_role(env)) != 0 && ret == 0)
		ret = t_ret;
	return ret;
}
/*
 * Checks for the need to resolve limbo (failure of a previous GMDB update to
 * get enough acks), and does it if nec.  No-op if none needed.
 *
 * Must be called within setup_gmdb_op/cleanup_gmdb_op context.
 */
static int resolve_limbo_int(ENV *env, DB_THREAD_INFO * ip)
{
	DB_REP * db_rep;
	DB_TXN * txn;
	REPMGR_SITE * site;
	DB_LSN orig_lsn;
	DBT key_dbt, data_dbt;
	__repmgr_member_args logrec;
	repmgr_netaddr_t addr;
	uint32 orig_status, status;
	int eid, locked, ret, t_ret;
	uint8 data_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];
	uint8 key_buf[MAX_MSG_BUF];

	db_rep = env->rep_handle;
	ret = 0;

	LOCK_MUTEX(db_rep->mutex);
	locked = TRUE;

	/*
	 * Is there a previous GMDB update failure currently pending?  If not,
	 * there's nothing for us to do.
	 */
	eid = db_rep->limbo_victim;
	if(!IS_VALID_EID(eid))
		goto out;
	site = SITE_FROM_EID(eid);
	addr = site->net_addr;
	marshal_site_key(env, &addr, key_buf, &key_dbt, &logrec);
	orig_status = site->membership;
	if(orig_status == SITE_PRESENT || orig_status == 0)
		goto out;

	/*
	 * It is possible after an autotakeover on a master to have no
	 * limbo_failure LSN but to have a limbo_victim that was found
	 * in the gmdb that still needs to be resolved.
	 */
	if(IS_ZERO_LSN(db_rep->limbo_failure) &&
	    !db_rep->limbo_resolution_needed)
		goto out;

	/*
	 * There are potentially two parts: the self-update of the existing
	 * limbo record, and then the finishing-off if the first is successful.
	 * We might only have to do the finishing-off, if some arbitrary random
	 * txn triggered a limbo resolution request on a msg processing thread.
	 */
	if(LOG_COMPARE(&db_rep->durable_lsn, &db_rep->limbo_failure) > 0) {
		/*
		 * Nice!  Limbo has been resolved by an arbitrary other txn
		 * succeeding subsequently.  So we don't have to do the
		 * "self-update" part.
		 */
	}
	else {
		/*
		 * Do a self-update, to try to trigger a "durable".  Since
		 * nothing in the database is changing, we need neither an ASL
		 * hint nor a bump in the version sequence.
		 */
		orig_lsn = db_rep->limbo_failure;
		db_rep->active_gmdb_update = gmdb_primary;
		UNLOCK_MUTEX(db_rep->mutex);
		locked = FALSE;

		if((ret = __txn_begin(env,
		    ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
			goto out;

		marshal_site_data(env,
		    orig_status, site->gmdb_flags, data_buf, &data_dbt);

		ret = __db_put(db_rep->gmdb, ip, txn, &key_dbt, &data_dbt, 0);
		if((t_ret = __db_txn_auto_resolve(env, txn, 0, ret)) != 0 &&
		    ret == 0)
			ret = t_ret;
		if(ret != 0)
			goto out;

		/*
		 * Check to see whether we got another PERM failure.  This is
		 * quite possible in the case where a GMDB request is being
		 * retried by a requestor, but unlikely if we had a resolution
		 * via an "arbitrary" txn.
		 */
		LOCK_MUTEX(db_rep->mutex);
		locked = TRUE;
		if(LOG_COMPARE(&db_rep->limbo_failure, &orig_lsn) > 0) {
			db_rep->limbo_resolution_needed = TRUE;
			ret = DB_REP_UNAVAIL;
			goto out;
		}
	}
	DB_ASSERT(env, locked);

	/*
	 * Here, either we didn't need to do the self-update, or we did it and
	 * it succeeded.  So now we're ready to do the second phase update.
	 */
	db_rep->limbo_victim = DB_EID_INVALID;
	UNLOCK_MUTEX(db_rep->mutex);
	locked = FALSE;
	status = NEXT_STATUS(orig_status);
	if((ret = finish_gmdb_update(env, ip,
	    &key_dbt, orig_status, status, site->gmdb_flags, &logrec)) != 0)
		goto out;

	/* Track modified membership status in our in-memory sites array. */
	LOCK_MUTEX(db_rep->mutex);
	locked = TRUE;
	if((ret = __repmgr_set_membership(env,
	    addr.host, addr.port, status, site->gmdb_flags)) != 0)
		goto out;
	__repmgr_set_sites(env);

out:
	if(locked)
		UNLOCK_MUTEX(db_rep->mutex);
	return ret;
}
/*
 * Update a specific record in the Group Membership database.  The record to be
 * updated is implied by "eid"; "pstatus" is the provisional status (ADDING or
 * DELETING) to be used in the first phase of the update.  The ultimate goal
 * status is inferred (ADDING -> PRESENT, or DELETING -> 0).
 *
 * PUBLIC: int __repmgr_update_membership __P((ENV *, DB_THREAD_INFO *, int, uint32, uint32));
 */
int __repmgr_update_membership(ENV *env, DB_THREAD_INFO * ip, int eid, uint32 pstatus/* Provisional status. */, uint32 site_flags)
{
	DB_REP * db_rep;
	REPMGR_SITE * site;
	DB_TXN * txn;
	DB_LSN lsn, orig_lsn;
	DBT key_dbt, data_dbt;
	__repmgr_member_args logrec;
	repmgr_netaddr_t addr;
	uint32 orig_status, ult_status;
	int do_close, locked, ret, t_ret;
	uint8 key_buf[MAX_MSG_BUF];
	uint8 status_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];

	DB_ASSERT(env, pstatus == SITE_ADDING || pstatus == SITE_DELETING);

	db_rep = env->rep_handle;
	COMPQUIET(orig_status, 0);
	COMPQUIET(addr.host, NULL);
	COMPQUIET(addr.port, 0);

retry:
	txn = NULL;
	locked = FALSE;
	DB_ASSERT(env, db_rep->gmdb_busy);
	if((ret = __repmgr_setup_gmdb_op(env, ip, NULL, 0)) != 0)
		return ret;

	/*
	 * Usually we'll keep the GMDB closed, to conserve resources, since
	 * changes should be rare.  However, if a PERM FAIL puts us in limbo, we
	 * expect to clean that up as soon as we can; so leave it open for now
	 * in that case.
	 */
	do_close = TRUE;

	/*
	 * Before attempting any fresh updates, resolve any lingering incomplete
	 * updates from the past (i.e., those that resulted in PERM_FAIL).  If
	 * we can't, then we mustn't proceed with any more updates.  Getting an
	 * additional perm failure would increase the dissonance between the
	 * effective group size and the number of sites from which we can safely
	 * accept acks.  Besides, if we can't clear the previous failure,
	 * there's practically no hope that a new update would fare any better.
	 */
	if((ret = resolve_limbo_int(env, ip)) != 0) {
		if(ret == DB_REP_UNAVAIL)
			do_close = FALSE;
		goto err;
	}

	/*
	 * If there was a successful limbo resolution, it could have either been
	 * for some unrelated change, or it could have been the same change our
	 * caller is now (re-)trying to perform.  In the latter case, we have
	 * nothing more to do -- resolve_limbo() has done it all for us!  To
	 * find out, compare the site's current status with the ultimate goal
	 * status associated with the provisional status that was passed to us
	 * as input.
	 */
	LOCK_MUTEX(db_rep->mutex);
	locked = TRUE;
	DB_ASSERT(env, IS_KNOWN_REMOTE_SITE(eid));
	site = SITE_FROM_EID(eid);
	if((orig_status = site->membership) == NEXT_STATUS(pstatus))
		goto err;
	addr = site->net_addr;

	/*
	 * Anticipate modified membership status in our in-memory sites array.
	 * This forces us into an awkward rescission, below, if our transaction
	 * suffers a hard failure and must be aborted.  But it's necessary
	 * because of the requirement that, on additions, the quorum computation
	 * must be based on the incremented nsites value.  An alternative might
	 * possibly be to increment nsites separately from adding the new site
	 * to the array, or even having a special epicycle at the point where
	 * send() counts acks (we'd have to make active_gmdb_update richer), but
	 * those seem even more confusing.
	 */
	if((ret = __repmgr_set_membership(env, addr.host, addr.port, pstatus, site_flags)) != 0)
		goto err;
	__repmgr_set_sites(env);
	/*
	 * Hint to our send() function that we want to know the result of ack counting.
	 */
	orig_lsn = db_rep->limbo_failure;
	db_rep->active_gmdb_update = gmdb_primary;
	UNLOCK_MUTEX(db_rep->mutex);
	locked = FALSE;
	if((ret = __txn_begin(env, ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
		goto err;
	marshal_site_key(env, &addr, key_buf, &key_dbt, &logrec);
	marshal_site_data(env, pstatus, site_flags, status_buf, &data_dbt);
	if((ret = __db_put(db_rep->gmdb, ip, txn, &key_dbt, &data_dbt, 0)) != 0)
		goto err;
	if((ret = incr_gm_version(env, ip, txn)) != 0)
		goto err;

	/*
	 * Add some information to the log for this txn.  This is an annotation,
	 * for the sole purpose of enabling the client to notice whenever a
	 * change has occurred in this database.  It has nothing to do with
	 * local recovery.
	 */
	ZERO_LSN(lsn);
	if((ret = __repmgr_member_log(env, txn, &lsn, 0, db_rep->membership_version, orig_status, pstatus, &logrec.host, logrec.port)) != 0)
		goto err;
	ret = __txn_commit(txn, 0);
	txn = NULL;
	if(ret != 0)
		goto err;

	LOCK_MUTEX(db_rep->mutex);
	locked = TRUE;

	if(LOG_COMPARE(&db_rep->limbo_failure, &orig_lsn) > 0) {
		/*
		 * Failure LSN advanced, meaning this update wasn't acked by
		 * enough clients.
		 */
		db_rep->limbo_resolution_needed = TRUE;
		db_rep->limbo_victim = eid;
		ret = DB_REP_UNAVAIL;
		do_close = FALSE;
		goto err;
	}

	/* Now we'll complete the status change. */
	ult_status = NEXT_STATUS(pstatus);
	UNLOCK_MUTEX(db_rep->mutex);
	locked = FALSE;

	if((ret = finish_gmdb_update(env, ip,
	    &key_dbt, pstatus, ult_status, site_flags, &logrec)) != 0)
		goto err;

	/* Track modified membership status in our in-memory sites array. */
	LOCK_MUTEX(db_rep->mutex);
	locked = TRUE;
	ret = __repmgr_set_membership(env, addr.host, addr.port,
		ult_status, site_flags);
	__repmgr_set_sites(env);

err:
	if(locked)
		UNLOCK_MUTEX(db_rep->mutex);
	if(txn != NULL) {
		DB_ASSERT(env, ret != 0);
		(void)__txn_abort(txn);
		/*
		 * We've just aborted the txn which moved the site info from
		 * orig_status to something else, so restore that value now so
		 * that we keep in sync.
		 */
		(void)__repmgr_set_membership(env,
		    addr.host, addr.port, orig_status, site_flags);
	}
	if((t_ret = __repmgr_cleanup_gmdb_op(env, do_close)) != 0 && ret == 0)
		ret = t_ret;
	if(ret == DB_LOCK_DEADLOCK || ret == DB_LOCK_NOTGRANTED)
		goto retry;
	return ret;
}

/*
 * Rescind a partially completed membership DB change, setting the new status to
 * the value given.
 */
static int rescind_pending(ENV *env, DB_THREAD_INFO * ip, int eid, uint32 cur_status, uint32 new_status)
{
	DB_REP * db_rep;
	REPMGR_SITE * site;
	DBT key_dbt;
	__repmgr_member_args logrec;
	repmgr_netaddr_t addr;
	uint8 key_buf[MAX_MSG_BUF];
	int ret, t_ret;
	db_rep = env->rep_handle;
retry:
	if((ret = __repmgr_setup_gmdb_op(env, ip, NULL, 0)) != 0)
		return ret;
	LOCK_MUTEX(db_rep->mutex);
	DB_ASSERT(env, IS_KNOWN_REMOTE_SITE(eid));
	site = SITE_FROM_EID(eid);
	addr = site->net_addr;
	UNLOCK_MUTEX(db_rep->mutex);

	marshal_site_key(env, &addr, key_buf, &key_dbt, &logrec);
	if((ret = finish_gmdb_update(env, ip,
	    &key_dbt, cur_status, new_status, site->gmdb_flags, &logrec)) != 0)
		goto err;

	/* Track modified membership status in our in-memory sites array. */
	LOCK_MUTEX(db_rep->mutex);
	ret = __repmgr_set_membership(env, addr.host, addr.port,
		new_status, site->gmdb_flags);
	__repmgr_set_sites(env);
	UNLOCK_MUTEX(db_rep->mutex);

err:
	if((t_ret = __repmgr_cleanup_gmdb_op(env, TRUE)) != 0 &&
	    ret == 0)
		ret = t_ret;
	if(ret == DB_LOCK_DEADLOCK || ret == DB_LOCK_NOTGRANTED)
		goto retry;
	return ret;
}

/*
 * Caller must have already taken care of serializing this operation
 * (hold_master_role(), setup_gmdb_op()).
 */
static int incr_gm_version(ENV *env, DB_THREAD_INFO * ip, DB_TXN * txn)
{
	int ret;
	DB_REP * db_rep = env->rep_handle;
	uint32 version = db_rep->membership_version + 1;
	if((ret = __repmgr_set_gm_version(env, ip, txn, version)) == 0)
		db_rep->membership_version = version;
	return ret;
}
/*
 * PUBLIC: int __repmgr_set_gm_version __P((ENV *, DB_THREAD_INFO *, DB_TXN *, uint32));
 */
int __repmgr_set_gm_version(ENV *env, DB_THREAD_INFO * ip, DB_TXN * txn, uint32 version)
{
	DBT key_dbt, data_dbt;
	__repmgr_membership_key_args key;
	__repmgr_member_metadata_args metadata;
	uint8 key_buf[__REPMGR_MEMBERSHIP_KEY_SIZE + 1];
	uint8 metadata_buf[__REPMGR_MEMBER_METADATA_SIZE];
	size_t len;
	int ret;
	DB_REP * db_rep = env->rep_handle;
	metadata.format = REPMGR_GMDB_FMT_VERSION;
	metadata.version = version;
	__repmgr_member_metadata_marshal(env, &metadata, metadata_buf);
	DB_INIT_DBT(data_dbt, metadata_buf, __REPMGR_MEMBER_METADATA_SIZE);
	DB_INIT_DBT(key.host, NULL, 0);
	key.port = 0;
	ret = __repmgr_membership_key_marshal(env, &key, key_buf, sizeof(key_buf), &len);
	DB_ASSERT(env, ret == 0);
	DB_INIT_DBT(key_dbt, key_buf, len);
	if((ret = __db_put(db_rep->gmdb, ip, txn, &key_dbt, &data_dbt, 0)) != 0)
		return ret;
	return 0;
}

/*
 * Performs the second phase of a 2-phase membership DB operation: an "adding"
 * site becomes fully "present" in the group; a "deleting" site is finally
 * really deleted.
 */
static int finish_gmdb_update(ENV *env, DB_THREAD_INFO * ip, DBT * key_dbt, uint32 prev_status, uint32 status, uint32 flags, __repmgr_member_args * logrec)
{
	DB_REP * db_rep;
	DB_LSN lsn;
	DB_TXN * txn;
	DBT data_dbt;
	uint8 data_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];
	int ret, t_ret;
	db_rep = env->rep_handle;
	db_rep->active_gmdb_update = gmdb_secondary;
	if((ret = __txn_begin(env, ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
		return ret;
	if(status == 0)
		ret = __db_del(db_rep->gmdb, ip, txn, key_dbt, 0);
	else {
		marshal_site_data(env, status, flags, data_buf, &data_dbt);
		ret = __db_put(db_rep->gmdb, ip, txn, key_dbt, &data_dbt, 0);
	}
	if(ret != 0)
		goto err;

	if((ret = incr_gm_version(env, ip, txn)) != 0)
		goto err;

	ZERO_LSN(lsn);
	if((ret = __repmgr_member_log(env,
	    txn, &lsn, 0, db_rep->membership_version,
	    prev_status, status, &logrec->host, logrec->port)) != 0)
		goto err;

err:
	if((t_ret = __db_txn_auto_resolve(env, txn, 0, ret)) != 0 && ret == 0)
		ret = t_ret;
	return ret;
}
/*
 * Set up everything we need to update the Group Membership database.  This may
 * or may not include providing a transaction in which to do the updates
 * (depending on whether the caller wants the creation of the database to be in
 * the same transaction as the updates).
 *
 * PUBLIC: int __repmgr_setup_gmdb_op __P((ENV *, DB_THREAD_INFO *, DB_TXN **, uint32));
 */
int __repmgr_setup_gmdb_op(ENV *env, DB_THREAD_INFO * ip, DB_TXN ** txnp, uint32 flags)
{
	int ret, was_open;
	DB_REP * db_rep = env->rep_handle;
	DB * dbp = NULL;
	DB_TXN * txn = NULL;
	/*
	 * If the caller provided a place to return a txn handle, create it and
	 * perform any open operation as part of that txn.  The caller is
	 * responsible for disposing of the txn.  Otherwise, only begin a txn if
	 * we need to do the open and in that case commit it right after the
	 * open.
	 */
	DB_ASSERT(env, db_rep->gmdb_busy);
	was_open = db_rep->gmdb != NULL;
	if((txnp != NULL || !was_open) && (ret = __txn_begin(env, ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
		goto err;

	if(!was_open) {
		DB_ASSERT(env, txn != NULL);
		/*
		 * Opening the membership database is like a secondary GMDB
		 * operation, in the sense that we don't care how many clients
		 * ack it, yet we don't want the application to see any perm
		 * failure events.
		 */
		DB_ASSERT(env, db_rep->active_gmdb_update == gmdb_none);
		db_rep->active_gmdb_update = gmdb_secondary;
		ret = __rep_open_sysdb(env, ip, txn, REPMEMBERSHIP, flags, &dbp);
		if(ret == 0 && txnp == NULL) {
			/* The txn was just for the open operation. */
			ret = __txn_commit(txn, 0);
			txn = NULL;
		}
		db_rep->active_gmdb_update = gmdb_none;
		if(ret != 0)
			goto err;
	}

	/*
	 * Lock out normal API operations.  Again because we need to know that
	 * if a PERM_FAIL occurs, it was associated with our txn.  Also, so that
	 * we avoid confusing the application with a PERM_FAIL event for our own
	 * txn.
	 */
	if((ret = __rep_take_apilockout(env)) != 0)
		goto err;

	/*
	 * Here, all steps have succeeded.  Stash and/or pass back the fruits of
	 * our labor.
	 */
	if(!was_open) {
		DB_ASSERT(env, dbp != NULL);
		db_rep->gmdb = dbp;
	}
	if(txnp != NULL) {
		DB_ASSERT(env, txn != NULL);
		*txnp = txn;
	}
	/*
	 * In the successful case, a later call to cleanup_gmdb_op will
	 * ENV_LEAVE.
	 */
	return 0;

err:
	DB_ASSERT(env, ret != 0);
	if(dbp != NULL)
		(void)__db_close(dbp, txn, DB_NOSYNC);
	if(txn != NULL)
		(void)__txn_abort(txn);
	return ret;
}
/*
 * PUBLIC: int __repmgr_cleanup_gmdb_op __P((ENV *, int));
 */
int __repmgr_cleanup_gmdb_op(ENV *env, int do_close)
{
	int ret, t_ret;
	DB_REP * db_rep = env->rep_handle;
	db_rep->active_gmdb_update = gmdb_none;
	ret = __rep_clear_apilockout(env);
	if(do_close && db_rep->gmdb != NULL) {
		if((t_ret = __db_close(db_rep->gmdb, NULL, DB_NOSYNC) != 0) && ret == 0)
			ret = t_ret;
		db_rep->gmdb = NULL;
	}
	return ret;
}
/*
 * Check whether we're currently master, and if so hold that role so that we can
 * perform a Group Membership database operation.  After a successful call, the
 * caller must call rlse_master_role to release the hold.
 *
 * If we can't guarantee that we can remain master, send an appropriate failure
 * message on the given connection (unless NULL).
 *
 * We also ensure that only one GMDB operation will take place at time, for a
 * couple of reasons: if we get a PERM_FAIL it means the fate of the change is
 * indeterminate, so we have to assume the worst.  We have to assume the higher
 * value of nsites, yet we can't accept ack from the questionable site.  If we
 * allowed concurrent operations, this could lead to more than one questionable
 * site, which would be even worse.  Also, when we get a PERM_FAIL we want to
 * know which txn failed, and that would be messy if there could be several.
 *
 * Of course we can't simply take the mutex for the duration, because
 * the mutex needs to be available in order to send out the log
 * records.
 *
 * PUBLIC: int __repmgr_hold_master_role __P((ENV *, REPMGR_CONNECTION *, uint32));
 */
int __repmgr_hold_master_role(ENV *env, REPMGR_CONNECTION * conn, uint32 membership)
{
	int ret, t_ret;
	DB_REP * db_rep = env->rep_handle;
	REP * rep = db_rep->region;
	LOCK_MUTEX(db_rep->mutex);
	if((ret = __repmgr_await_gmdbop(env)) == 0) {
		/*
		 * If we're currently master, but client_intent is set, it means
		 * that another thread is on the way to becoming master, so we
		 * can't promise to hold the master role for the caller: we've
		 * lost a close race.
		 */
		if(rep->master_id != db_rep->self_eid ||
		    db_rep->client_intent)
			ret = DB_REP_UNAVAIL;
		else
			db_rep->gmdb_busy = TRUE;
	}
	UNLOCK_MUTEX(db_rep->mutex);

	/*
	 * Check for an edge case that is possible with a newly-joining site
	 * in a 2SITE_STRICT group.  If both sites crash before the joining
	 * site receives this site's gmdb contents with the GM_SUCCESS message,
	 * the group is left is a state where this site can't elect itself
	 * master and the joining site has no master to retry joining the
	 * group.  Be a little extra lenient if the joining site retries its
	 * join when there is no master: enable caller to return GM_SUCCESS
	 * and this site's gmdb contents again if the joining site is already
	 * in the ADDING or PRESENT state in this site's gmdb.
	 */
	if(conn != NULL && ret == DB_REP_UNAVAIL && rep->config_nsites == 2 &&
	    FLD_ISSET(rep->config, REP_C_2SITE_STRICT) &&
	    (membership == SITE_ADDING || membership == SITE_PRESENT))
		ret = 0;

	if(conn != NULL && ret == DB_REP_UNAVAIL && (t_ret = reject_fwd(env, conn)) != 0)
		ret = t_ret;
	return ret;
}

/*
 * Releases the "master role" lock once we're finished performing a membership
 * DB operation.
 *
 * PUBLIC: int __repmgr_rlse_master_role(ENV *);
 */
int __repmgr_rlse_master_role(ENV *env)
{
	int ret;
	DB_REP * db_rep = env->rep_handle;
	LOCK_MUTEX(db_rep->mutex);
	db_rep->gmdb_busy = FALSE;
	ret = __repmgr_signal(&db_rep->gmdb_idle);
	UNLOCK_MUTEX(db_rep->mutex);
	return ret;
}
/*
 * Responds to a membership change request in the case we're not currently
 * master.  If we know the master, responds with a "forward" message, to tell
 * the requestor who is master.  Otherwise rejects it outright.
 */
static int reject_fwd(ENV *env, REPMGR_CONNECTION * conn)
{
	DB_REP * db_rep;
	REP * rep;
	SITE_STRING_BUFFER site_string;
	__repmgr_gm_fwd_args fwd;
	repmgr_netaddr_t addr;
	uint8 buf[MAX_MSG_BUF];
	uint32 msg_type;
	size_t len;
	int ret;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	if(IS_KNOWN_REMOTE_SITE(rep->master_id)) {
		msg_type = REPMGR_GM_FORWARD;
		LOCK_MUTEX(db_rep->mutex);
		addr = SITE_FROM_EID(rep->master_id)->net_addr;
		UNLOCK_MUTEX(db_rep->mutex);
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Forwarding request to master %s", __repmgr_format_addr_loc(&addr, site_string)));
		fwd.host.data = addr.host;
		fwd.host.size = (uint32)strlen((const char *)fwd.host.data) + 1;
		fwd.port = addr.port;
		fwd.gen = rep->mgen;
		ret = __repmgr_gm_fwd_marshal(env, &fwd, buf, sizeof(buf), &len);
		DB_ASSERT(env, ret == 0);
	}
	else {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Rejecting membership request with no known master"));
		msg_type = REPMGR_GM_FAILURE;
		len = 0;
	}
	return (__repmgr_send_sync_msg(env, conn, msg_type, buf, (uint32)len));
}

/*
 * The length of "buf" must be at least MAX_GMDB_KEY.
 */
static void marshal_site_key(ENV *env, repmgr_netaddr_t * addr, uint8 * buf, DBT * dbt, __repmgr_member_args * logrec)
{
	__repmgr_membership_key_args key;
	size_t len;
	int ret;
	DB_INIT_DBT(key.host, addr->host, strlen(addr->host) + 1);
	logrec->host = key.host;
	key.port = addr->port;
	logrec->port = key.port;
	ret = __repmgr_membership_key_marshal(env,
		&key, buf, MAX_MSG_BUF, &len);
	DB_ASSERT(env, ret == 0);
	DB_INIT_DBT(*dbt, buf, len);
}

static void marshal_site_data(ENV *env, uint32 status, uint32 flags, uint8 * buf, DBT * dbt)
{
	__repmgr_membership_data_args member_data;
	member_data.status = status;
	member_data.flags = flags;
	__repmgr_membership_data_marshal(env, &member_data, buf);
	DB_INIT_DBT(*dbt, buf, __REPMGR_MEMBERSHIP_DATA_SIZE);
}
/*
 * PUBLIC: void __repmgr_set_sites(ENV *);
 *
 * Caller must hold mutex.
 */
void __repmgr_set_sites(ENV *env)
{
	REP * rep;
	int ret;
	uint32 n;
	u_int i;
	DB_REP * db_rep = env->rep_handle;
	rep = db_rep->region;
	for(i = 0, n = 0; i < db_rep->site_cnt; i++) {
		/*
		 * Views do not count towards nsites because they cannot
		 * vote in elections, become master or contribute to
		 * durability.
		 */
		if(db_rep->sites[i].membership > 0 && !FLD_ISSET(db_rep->sites[i].gmdb_flags, SITE_VIEW))
			n++;
	}
	ret = __rep_set_nsites_int(env, n);
	DB_ASSERT(env, ret == 0);
	if(FLD_ISSET(rep->config, REP_C_PREFMAS_MASTER | REP_C_PREFMAS_CLIENT) && rep->config_nsites > 2)
		__db_errx(env, DB_STR("3701", "More than two sites in preferred master replication group"));
}

/*
 * If a site is rejoining a 2-site repgroup with 2SITE_STRICT off
 * and has a rejection because it needs to catch up with the latest
 * group membership database, it cannot call an election right away
 * because it would win with only its own vote and ignore an existing
 * master in the repgroup.  Instead, this routine is used to call the
 * deferred election after the site has rejoined the repgroup successfully.
 */
static int rejoin_deferred_election(ENV *env)
{
	uint32 flags;
	int eid, ret;
	DB_REP * db_rep = env->rep_handle;
	LOCK_MUTEX(db_rep->mutex);
	/*
	 * First, retry all connections so that the election can communicate
	 * with the other sites.  Normally there should only be one other
	 * site in the repgroup, but it is safest to retry all remote sites
	 * found in case the group membership changed while we were gone.
	 */
	FOR_EACH_REMOTE_SITE_INDEX(eid) {
		if((ret = __repmgr_schedule_connection_attempt(env, eid, TRUE)) != 0)
			break;
	}

	/*
	 * Call an immediate, but not a fast, election because a fast
	 * election reduces the number of votes needed by 1.
	 */
	flags = ELECT_F_EVENT_NOTIFY;
	if(FLD_ISSET(db_rep->region->config, REP_C_ELECTIONS))
		LF_SET(ELECT_F_IMMED);
	else
		RPRINT(env, (env, DB_VERB_REPMGR_MISC, "Deferred rejoin election, but no elections"));
	ret = __repmgr_init_election(env, flags);
	UNLOCK_MUTEX(db_rep->mutex);
	return ret;
}
/*
 * If a site is rejoining a preferred master replication group and has a
 * rejection because it needs to catch up with the latest group membership
 * database, it needs to establish its "regular" connection to the other site
 * so that it can proceed through the preferred master startup sequence.
 */
static int rejoin_connections(ENV *env)
{
	int eid;
	DB_REP * db_rep = env->rep_handle;
	int ret = 0;
	LOCK_MUTEX(db_rep->mutex);
	/*
	 * Retry all connections.   Normally there should only be one other
	 * site in the repgroup, but it is safest to retry all remote sites
	 * found in case the group membership changed while we were gone.
	 */
	FOR_EACH_REMOTE_SITE_INDEX(eid) {
		if((ret = __repmgr_schedule_connection_attempt(env, eid, TRUE)) != 0)
			break;
	}
	UNLOCK_MUTEX(db_rep->mutex);
	return ret;
}
