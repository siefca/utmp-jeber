/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#ifndef PIDLIST_H
#define PIDLIST_H

#include "includes.h"

/* pidlist struct */

struct pidlist
{
    pid_t pid;	/* PID	*/
    pid_t ppid;	/* PPID	*/
    uid_t uid;	/* UID	*/
    uid_t euid;	/* EUID	*/
    pid_t pgid; /* PGID */
    unsigned char checked; /* used by tree-search to mark BAD paths */
    struct pidlist *parent; /* self explanatoru */
    struct pidlist *next; /* as above */
};

extern struct pidlist plist;
extern struct pidlist *pmem[];

unsigned long int fetch_procs();
unsigned long int create_parentship();
void reset_tree_pathinfo();

void current_reset();
void destroy_list();
void add_list_item(pid_t pid, pid_t ppid, uid_t uid, uid_t euid, pid_t pgid);

struct pidlist * jump_to_parent(struct pidlist *cur);
struct pidlist * search_byuids(struct pidlist **p, uid_t uid);
struct pidlist * search_bypid(struct pidlist *c, pid_t pid);

int get_proc_uid(const char * statusf, uid_t *ruid, uid_t *euid);

inline void reset_path_memory();

#endif
