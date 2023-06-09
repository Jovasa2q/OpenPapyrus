/* Do not edit: automatically built by gen_msg.awk. */

#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * PUBLIC: int __rep_bulk_marshal __P((ENV *, __rep_bulk_args *,
 * PUBLIC:	 uint8 *, size_t, size_t *));
 */
int __rep_bulk_marshal(ENV * env, __rep_bulk_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_BULK_SIZE+(size_t)argp->bulkdata.size)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->len);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.Offset_);
	DB_HTONL_COPYOUT(env, bp, argp->bulkdata.size);
	if(argp->bulkdata.size > 0) {
		memcpy(bp, argp->bulkdata.data, argp->bulkdata.size);
		bp += argp->bulkdata.size;
	}
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_bulk_unmarshal __P((ENV *, __rep_bulk_args *,
 * PUBLIC:	 uint8 *, size_t, uint8 **));
 */
int __rep_bulk_unmarshal(ENV * env, __rep_bulk_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	size_t needed = __REP_BULK_SIZE;
	if(max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->len, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.Offset_, bp);
	DB_NTOHL_COPYIN(env, argp->bulkdata.size, bp);
	argp->bulkdata.data = bp;
	needed += (size_t)argp->bulkdata.size;
	if(max < needed)
		goto too_few;
	bp += argp->bulkdata.size;
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_bulk message"));
	return EINVAL;
}

/*
 * PUBLIC: int __rep_control_marshal __P((ENV *, __rep_control_args *,
 * PUBLIC:	 uint8 *, size_t, size_t *));
 */
int __rep_control_marshal(ENV * env, __rep_control_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_CONTROL_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->rep_version);
	DB_HTONL_COPYOUT(env, bp, argp->log_version);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.Offset_);
	DB_HTONL_COPYOUT(env, bp, argp->rectype);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
	DB_HTONL_COPYOUT(env, bp, argp->msg_sec);
	DB_HTONL_COPYOUT(env, bp, argp->msg_nsec);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_control_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_control_args *, uint8 *, size_t, uint8 **));
 */
int __rep_control_unmarshal(ENV * env, __rep_control_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_CONTROL_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->rep_version, bp);
	DB_NTOHL_COPYIN(env, argp->log_version, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.Offset_, bp);
	DB_NTOHL_COPYIN(env, argp->rectype, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	DB_NTOHL_COPYIN(env, argp->msg_sec, bp);
	DB_NTOHL_COPYIN(env, argp->msg_nsec, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_control message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_egen_marshal __P((ENV *, __rep_egen_args *,
 * PUBLIC:	 uint8 *, size_t, size_t *));
 */
int __rep_egen_marshal(ENV * env, __rep_egen_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_EGEN_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->egen);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_egen_unmarshal __P((ENV *, __rep_egen_args *,
 * PUBLIC:	 uint8 *, size_t, uint8 **));
 */
int __rep_egen_unmarshal(ENV * env, __rep_egen_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_EGEN_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_egen message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_fileinfo_marshal __P((ENV *, uint32,
 * PUBLIC:	 __rep_fileinfo_args *, uint8 *, size_t, size_t *));
 */
int __rep_fileinfo_marshal(ENV * env, uint32 version, __rep_fileinfo_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	int copy_only;
	uint8 * start;
	if(max < __REP_FILEINFO_SIZE+(size_t)argp->uid.size+(size_t)argp->info.size)
		return ENOMEM;
	start = bp;
	copy_only = 0;
	if(version < DB_REPVERSION_47)
		copy_only = 1;
	if(copy_only) {
		memcpy(bp, &argp->pgsize, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->pgsize);
	if(copy_only) {
		memcpy(bp, &argp->pgno, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->pgno);
	if(copy_only) {
		memcpy(bp, &argp->max_pgno, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->max_pgno);
	if(copy_only) {
		memcpy(bp, &argp->filenum, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->filenum);
	if(copy_only) {
		memcpy(bp, &argp->finfo_flags, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->finfo_flags);
	if(copy_only) {
		memcpy(bp, &argp->type, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->type);
	if(copy_only) {
		memcpy(bp, &argp->db_flags, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->db_flags);
	if(copy_only) {
		memcpy(bp, &argp->uid.size, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->uid.size);
	if(argp->uid.size > 0) {
		memcpy(bp, argp->uid.data, argp->uid.size);
		bp += argp->uid.size;
	}
	if(copy_only) {
		memcpy(bp, &argp->info.size, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->info.size);
	if(argp->info.size > 0) {
		memcpy(bp, argp->info.data, argp->info.size);
		bp += argp->info.size;
	}
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_fileinfo_unmarshal __P((ENV *, uint32,
 * PUBLIC:	 __rep_fileinfo_args **, uint8 *, size_t, uint8 **));
 */
int __rep_fileinfo_unmarshal(ENV * env, uint32 version, __rep_fileinfo_args ** argpp, uint8 * bp, size_t max, uint8 ** nextp)
{
	__rep_fileinfo_args * argp;
	int ret;
	int copy_only;
	size_t needed = __REP_FILEINFO_SIZE;
	if(max < needed)
		goto too_few;
	if((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return ret;
	copy_only = 0;
	if(version < DB_REPVERSION_47)
		copy_only = 1;
	if(copy_only) {
		memcpy(&argp->pgsize, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->pgsize, bp);
	if(copy_only) {
		memcpy(&argp->pgno, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->pgno, bp);
	if(copy_only) {
		memcpy(&argp->max_pgno, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->max_pgno, bp);
	if(copy_only) {
		memcpy(&argp->filenum, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->filenum, bp);
	if(copy_only) {
		memcpy(&argp->finfo_flags, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->finfo_flags, bp);
	if(copy_only) {
		memcpy(&argp->type, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->type, bp);
	if(copy_only) {
		memcpy(&argp->db_flags, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->db_flags, bp);
	if(copy_only) {
		memcpy(&argp->uid.size, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->uid.size, bp);
	argp->uid.data = bp;
	needed += (size_t)argp->uid.size;
	if(max < needed)
		goto too_few;
	bp += argp->uid.size;
	if(copy_only) {
		memcpy(&argp->info.size, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->info.size, bp);
	argp->info.data = bp;
	needed += (size_t)argp->info.size;
	if(max < needed)
		goto too_few;
	bp += argp->info.size;
	if(nextp)
		*nextp = bp;
	*argpp = argp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_fileinfo message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_grant_info_marshal __P((ENV *,
 * PUBLIC:	 __rep_grant_info_args *, uint8 *, size_t, size_t *));
 */
int __rep_grant_info_marshal(ENV * env, __rep_grant_info_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_GRANT_INFO_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->msg_sec);
	DB_HTONL_COPYOUT(env, bp, argp->msg_nsec);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_grant_info_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_grant_info_args *, uint8 *, size_t, uint8 **));
 */
int __rep_grant_info_unmarshal(ENV * env, __rep_grant_info_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_GRANT_INFO_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->msg_sec, bp);
	DB_NTOHL_COPYIN(env, argp->msg_nsec, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_grant_info message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_logreq_marshal __P((ENV *, __rep_logreq_args *,
 * PUBLIC:	 uint8 *, size_t, size_t *));
 */
int __rep_logreq_marshal(ENV * env, __rep_logreq_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_LOGREQ_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->endlsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->endlsn.Offset_);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_logreq_unmarshal __P((ENV *, __rep_logreq_args *,
 * PUBLIC:	 uint8 *, size_t, uint8 **));
 */
int __rep_logreq_unmarshal(ENV * env, __rep_logreq_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_LOGREQ_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->endlsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->endlsn.Offset_, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_logreq message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_newfile_marshal __P((ENV *, __rep_newfile_args *,
 * PUBLIC:	 uint8 *, size_t, size_t *));
 */
int __rep_newfile_marshal(ENV * env, __rep_newfile_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_NEWFILE_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->version);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_newfile_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_newfile_args *, uint8 *, size_t, uint8 **));
 */
int __rep_newfile_unmarshal(ENV * env, __rep_newfile_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_NEWFILE_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	ASSIGN_PTR(nextp, bp);
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_newfile message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_update_marshal __P((ENV *, uint32,
 * PUBLIC:	 __rep_update_args *, uint8 *, size_t, size_t *));
 */
int __rep_update_marshal(ENV * env, uint32 version, __rep_update_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	int copy_only;
	uint8 * start;
	if(max < __REP_UPDATE_SIZE)
		return ENOMEM;
	start = bp;
	copy_only = 0;
	if(version < DB_REPVERSION_47)
		copy_only = 1;
	if(copy_only) {
		memcpy(bp, &argp->first_lsn.file, sizeof(uint32));
		bp += sizeof(uint32);
		memcpy(bp, &argp->first_lsn.Offset_, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else {
		DB_HTONL_COPYOUT(env, bp, argp->first_lsn.file);
		DB_HTONL_COPYOUT(env, bp, argp->first_lsn.Offset_);
	}
	if(copy_only) {
		memcpy(bp, &argp->first_vers, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->first_vers);
	if(copy_only) {
		memcpy(bp, &argp->num_files, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_HTONL_COPYOUT(env, bp, argp->num_files);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_update_unmarshal __P((ENV *, uint32,
 * PUBLIC:	 __rep_update_args **, uint8 *, size_t, uint8 **));
 */
int __rep_update_unmarshal(ENV * env, uint32 version, __rep_update_args ** argpp, uint8 * bp, size_t max, uint8 ** nextp)
{
	__rep_update_args * argp;
	int ret;
	int copy_only;
	if(max < __REP_UPDATE_SIZE)
		goto too_few;
	if((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return ret;
	copy_only = 0;
	if(version < DB_REPVERSION_47)
		copy_only = 1;
	if(copy_only) {
		memcpy(&argp->first_lsn.file, bp, sizeof(uint32));
		bp += sizeof(uint32);
		memcpy(&argp->first_lsn.Offset_, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else {
		DB_NTOHL_COPYIN(env, argp->first_lsn.file, bp);
		DB_NTOHL_COPYIN(env, argp->first_lsn.Offset_, bp);
	}
	if(copy_only) {
		memcpy(&argp->first_vers, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->first_vers, bp);
	if(copy_only) {
		memcpy(&argp->num_files, bp, sizeof(uint32));
		bp += sizeof(uint32);
	}
	else
		DB_NTOHL_COPYIN(env, argp->num_files, bp);
	ASSIGN_PTR(nextp, bp);
	*argpp = argp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_update message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_vote_info_marshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_args *, uint8 *, size_t, size_t *));
 */
int __rep_vote_info_marshal(ENV * env, __rep_vote_info_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_VOTE_INFO_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->egen);
	DB_HTONL_COPYOUT(env, bp, argp->nsites);
	DB_HTONL_COPYOUT(env, bp, argp->nvotes);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
	DB_HTONL_COPYOUT(env, bp, argp->spare_pri);
	DB_HTONL_COPYOUT(env, bp, argp->tiebreaker);
	DB_HTONL_COPYOUT(env, bp, argp->data_gen);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_vote_info_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_args *, uint8 *, size_t, uint8 **));
 */
int __rep_vote_info_unmarshal(ENV * env, __rep_vote_info_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_VOTE_INFO_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);
	DB_NTOHL_COPYIN(env, argp->nsites, bp);
	DB_NTOHL_COPYIN(env, argp->nvotes, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	DB_NTOHL_COPYIN(env, argp->spare_pri, bp);
	DB_NTOHL_COPYIN(env, argp->tiebreaker, bp);
	DB_NTOHL_COPYIN(env, argp->data_gen, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_vote_info message"));
	return EINVAL;
}
/*
 * PUBLIC: int __rep_vote_info_v5_marshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_v5_args *, uint8 *, size_t, size_t *));
 */
int __rep_vote_info_v5_marshal(ENV * env, __rep_vote_info_v5_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REP_VOTE_INFO_V5_SIZE)
		return ENOMEM;
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->egen);
	DB_HTONL_COPYOUT(env, bp, argp->nsites);
	DB_HTONL_COPYOUT(env, bp, argp->nvotes);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
	DB_HTONL_COPYOUT(env, bp, argp->tiebreaker);
	*lenp = (size_t)(bp-start);
	return 0;
}
/*
 * PUBLIC: int __rep_vote_info_v5_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_v5_args *, uint8 *, size_t, uint8 **));
 */
int __rep_vote_info_v5_unmarshal(ENV * env, __rep_vote_info_v5_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_VOTE_INFO_V5_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);
	DB_NTOHL_COPYIN(env, argp->nsites, bp);
	DB_NTOHL_COPYIN(env, argp->nvotes, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	DB_NTOHL_COPYIN(env, argp->tiebreaker, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_vote_info_v5 message"));
	return EINVAL;
}
/*
 * PUBLIC: void __rep_lsn_hist_key_marshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_key_args *, uint8 *));
 */
void __rep_lsn_hist_key_marshal(ENV * env, __rep_lsn_hist_key_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
}
/*
 * PUBLIC: int __rep_lsn_hist_key_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_key_args *, uint8 *, size_t, uint8 **));
 */
int __rep_lsn_hist_key_unmarshal(ENV * env, __rep_lsn_hist_key_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_LSN_HIST_KEY_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	if(nextp)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_lsn_hist_key message"));
	return EINVAL;
}
/*
 * PUBLIC: void __rep_lsn_hist_data_marshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_data_args *, uint8 *));
 */
void __rep_lsn_hist_data_marshal(ENV * env, __rep_lsn_hist_data_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->envid);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.Offset_);
	DB_HTONL_COPYOUT(env, bp, argp->hist_sec);
	DB_HTONL_COPYOUT(env, bp, argp->hist_nsec);
}
/*
 * PUBLIC: int __rep_lsn_hist_data_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_data_args *, uint8 *, size_t, uint8 **));
 */
int __rep_lsn_hist_data_unmarshal(ENV * env, __rep_lsn_hist_data_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REP_LSN_HIST_DATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->envid, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.Offset_, bp);
	DB_NTOHL_COPYIN(env, argp->hist_sec, bp);
	DB_NTOHL_COPYIN(env, argp->hist_nsec, bp);
	ASSIGN_PTR(nextp, bp);
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __rep_lsn_hist_data message"));
	return EINVAL;
}

