/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#ifndef INCLUDES_H
#define INCLUDES_H

#include "defines.h"

#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <utmp.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#endif
