/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */


#ifndef PROC_H
#define PROC_H

#include "includes.h"

#define TERMFIND_FAILED -1 /* cannot access fd or fd doesn't match */
#define TERMFIND_OK 0 	   /* have good terminal line found */
#define TERMFIND_NO_CHAR 1 /* no character devices for this process */
#define TERMFIND_NO_PROC 2 /* process not found */

int termfind (pid_t pid, const char *device);

int check_perm_foreign_process(uid_t my_uid, unsigned int min_hits);

int check_proc(char *username, pid_t startproc, int level, const char *device);
int check_proc_pgid(char *username, pid_t startproc, int level, const char *device);
int check_terminal_owner(const char *username, const char *device, pid_t startproc);
int check_user_procs(const char *username);

pid_t obtain_unbearable_child(pid_t startproc, uid_t uid);

const char * badproc(struct utmp *ut);

#endif
