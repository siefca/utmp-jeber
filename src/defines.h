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
#include "config.h"
#endif

#ifndef	VERSION
#define	VERSION	"1.0.6"
#endif

#define	INIT_NOT_PROVIDE

#define sayg	if (ST.debug) fprintf
#define	say	if (ST.verb) printf
#define	log	if (ST.batched) printf
#define	P_PRE	"/proc"
#define	P_SUF	"fd"

/* max search depth while jumping from parent to parent */

#define	SRCH_DEEP	256

/* processes list pointer */

#define list_begin()	&plist

#endif
