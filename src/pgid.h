/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#ifndef	PGID_H
#define	PGID_H

#include "defines.h"

#define	_XOPEN_SOURCE
#define	_XOPEN_SOURCE_EXTENDED

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

extern inline pid_t my_getpgid(pid_t p);

#endif
