static const char rcsid[] = "$Id: pgid.c,v 1.4 2003/02/19 14:20:17 siefca Exp $";

/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#include "pgid.h"

inline pid_t my_getpgid(pid_t p)
{
    return getpgid(p);
}
