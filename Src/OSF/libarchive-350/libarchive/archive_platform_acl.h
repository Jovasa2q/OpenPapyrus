/*-
 * Copyright (c) 2017 Martin Matuska
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * $FreeBSD$
 */
/* !!ONLY FOR USE INTERNALLY TO LIBARCHIVE!! */

#ifndef ARCHIVE_PLATFORM_ACL_H_INCLUDED
#define ARCHIVE_PLATFORM_ACL_H_INCLUDED

#ifndef __LIBARCHIVE_BUILD
#ifndef __LIBARCHIVE_TEST_COMMON
#error This header is only to be used internally to libarchive.
#endif
#endif

/*
 * Determine what ACL types are supported
 */
#if ARCHIVE_ACL_FREEBSD || ARCHIVE_ACL_SUNOS || ARCHIVE_ACL_LIBACL
#define ARCHIVE_ACL_POSIX1E     1
#endif

#if ARCHIVE_ACL_FREEBSD_NFS4 || ARCHIVE_ACL_SUNOS_NFS4 || \
    ARCHIVE_ACL_DARWIN || ARCHIVE_ACL_LIBRICHACL
#define ARCHIVE_ACL_NFS4        1
#endif

#if ARCHIVE_ACL_POSIX1E || ARCHIVE_ACL_NFS4
#define ARCHIVE_ACL_SUPPORT     1
#endif

#endif	/* ARCHIVE_PLATFORM_ACL_H_INCLUDED */
