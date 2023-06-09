/** @file
 *  @brief Socket handling utilities.
 */
/* Copyright (C) 2006,2007,2008,2015 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */
#ifndef XAPIAN_INCLUDED_SOCKET_UTILS_H
#define XAPIAN_INCLUDED_SOCKET_UTILS_H

#include "safenetdb.h"
#include "safeunistd.h"

#ifdef __WIN32__

#include "safewinsock2.h"

/// Convert an fd (which might be a socket) to a WIN32 HANDLE.
extern HANDLE fd_to_handle(int fd);
/// Close an fd, which might be a socket.
extern void close_fd_or_socket(int fd);

#else

// For INET_ADDRSTRLEN and INET6_ADDRSTRLEN.
#include <arpa/inet.h>

// There's no distinction between sockets and other fds on UNIX.
inline void close_fd_or_socket(int fd) { close(fd); }

#endif

/** Attempt to set socket-level timeouts.
 *
 *  These aren't supported by all platforms, and some platforms allow them to
 *  set but ignore them, so we can't easily report failure.
 *
 *  Also sets SO_KEEPALIVE (if supported), which should ensure a stuck
 *  connection will eventually time out, though it may take up to ~2 hours.
 */
void set_socket_timeouts(int fd, double timeout);

constexpr size_t PRETTY_IP6_LEN = (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

int pretty_ip6(const void* p, char * buf);

#endif // XAPIAN_INCLUDED_SOCKET_UTILS_H
