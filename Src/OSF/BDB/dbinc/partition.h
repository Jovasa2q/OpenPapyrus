/*-
 * See the file LICENSE for redistribution information.
 * Copyright (c) 1996, 2011 Oracle and/or its affiliates.  All rights reserved.
 */
/*
 * $Id$
 */
#ifndef	_DB_PART_H_
#define	_DB_PART_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct __db_partition {
	uint32	nparts;		/* number of partitions. */
	DBT		*keys;		/* array of range keys. */
	void		*data;		/* the partion info. */
	const char	**dirs;		/* locations for partitions. */
	DB		**handles;	/* array of partition handles. */
	uint32	(*callback) (DB *, DBT *);
#define	PART_CALLBACK	0x01
#define	PART_RANGE	0x02
	uint32	flags;
} DB_PARTITION;
// 
// Internal part of a partitoned cursor.
// 
typedef struct __part_internal {
	__DBC_INTERNAL
	uint32	part_id;
	DBC		*sub_cursor;
} PART_CURSOR;

#ifdef HAVE_PARTITION
	#define	PART_NAME	"__dbp.%s.%03d"
	#define	PART_LEN	(sstrlen("__dbp..")+3)
	#define	PART_PREFIX	"__dbp."

	#define	DB_IS_PARTITIONED(dbp) (dbp->p_internal != NULL && ((DB_PARTITION *)dbp->p_internal)->handles != NULL)
	#define	DBC_PART_REFRESH(dbc)	(F_SET(dbc, DBC_PARTITIONED))
#else
	#define	DBC_PART_REFRESH(dbc)
	#define	DB_IS_PARTITIONED(dbp)	(0)
#endif

#if defined(__cplusplus)
}
#endif
#endif
