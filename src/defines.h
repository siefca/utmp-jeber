/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */


#ifndef DEFINES_H
#define DEFINES_H

#ifdef	HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef	PACKAGE_VERSION
# define PACKAGE_VERSION "(unknown)"
#endif

#define sayg	if (ST.debug) fprintf
#define	say	if (ST.verb) printf
#define	log	if (ST.batched) printf

/* procfs mount root */

#ifndef PROC_MOUNT
# define PROC_MOUNT "/proc"
#endif

/* procfs file descriptors subdirectory */

#ifndef P_SUF
# define P_SUF "fd"
#endif

/* where is the UTMP file? */

#ifndef	UTMP_NAME
# define UTMP_NAME "/var/run/utmpx"
#endif

#if HAVE_PATHS_H
# ifdef _PATH_UTMP
#  define UTMP_ORG_NAME _PATH_UTMP
# endif
#endif

#ifndef UTMP_ORG_NAME
# define UTMP_ORG_NAME "/var/run/utmp"
#endif

/* do we need superuser to run? */

#ifdef NEED_ROOT
# define NEED_SUPERUSER
#endif

/* consider init proccess as possible parent for user session? */

#ifndef CONSIDER_INIT
# define INIT_NOT_PROVIDE
#endif

/* max search depth while jumping from parent to parent */

#ifndef SRCH_DEEP
# define SRCH_DEEP 256
#endif

/* minimum number of hits when checking access to fds' */

#ifndef MIN_FD_CHECKS
# define MIN_FD_CHECKS 8
#endif

/* processes list pointer */

#define list_begin()	&plist

#endif
