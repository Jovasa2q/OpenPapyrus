/* Binary mode I/O.
   Copyright (C) 2001, 2003, 2005, 2008-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _BINARY_H
#define _BINARY_H

/* SET_BINARY (fd);
   changes the file descriptor fd to perform binary I/O.  */
#if O_BINARY
#if defined __EMX__ || defined __DJGPP__ || defined __CYGWIN__
	#include <io.h> /* declares setmode() */
#else
	#define setmode _setmode
	#undef fileno
	#define fileno _fileno
#endif
#ifdef __DJGPP__
	#include <unistd.h> /* declares isatty() */
   /* Avoid putting stdin/stdout in binary mode if it is connected to
      the console, because that would make it impossible for the user
      to interrupt the program through Ctrl-C or Ctrl-Break.  */
	#define SET_BINARY(fd) ((void) (!isatty (fd) ? (setmode (fd, O_BINARY), 0) : 0))
#else
	#define SET_BINARY(fd) ((void)setmode(fd, O_BINARY))
#endif
#else
	#define SET_BINARY(fd) /* do nothing */ ((void)0) /* On reasonable systems, binary I/O is the default.  */
#endif

#endif /* _BINARY_H */
