static const char rcsid[] = "$Id: pidlist.c,v 1.2 2003/02/11 13:24:28 siefca Exp $";
/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#include "pidlist.h"
#include "settings.h"

struct pidlist plist;
struct pidlist *pmem[SRCH_DEEP+2];
static struct pidlist *current = &plist;

/****************************************/

void current_reset()
{
    current = &plist;
}

/****************************************/

void destroy_list()
{
    struct pidlist *p, *o;

    p = list_begin();
    if (!(p->next)) return;
    p = p->next;

    while (p)
    {
	o = p->next;
	free(p);
	p = o;
    }

    current_reset();
}

/****************************************/

void add_list_item(pid_t pid, pid_t ppid, uid_t uid, uid_t euid, pid_t pgid)
{
    struct pidlist *p = (struct pidlist *) malloc(sizeof(struct pidlist));

    current->pid = pid;
    current->ppid = ppid;
    current->uid = uid;
    current->euid = euid;
    current->pgid = pgid;
    current->checked = 0;
    current->parent = NULL;
    if (!p)
    { 
	fprintf(stderr, "utmpchk: malloc error!\n");
	destroy_list();
	exit(2);
    }
    current->next = p;
    p->next = NULL;
    p->parent = NULL;
    p->pid = 0;
    p->checked = 0;
    current = p;
}

/****************************************/

struct pidlist * search_bypid(struct pidlist *c, pid_t pid)
{
    struct pidlist *p;
    
    if (!c || !(c->next) || !(c->pid)) return NULL;
    for (;;)
    {
	if (c->pid == pid)
	    {
		p = c;
		if (c->next) c = c->next;
		return p;
	    }
	if (c->next) c = c->next;
	else return NULL;
    }
}

/****************************************/

struct pidlist *search_byuids(struct pidlist **p, uid_t uid)
{
    struct pidlist *c=NULL, *r=NULL;
    
    if (!p) return NULL;

    c = *p;
    if (!c || !(c->next) || !(c->pid)) return NULL;
    
    for (; c->next ; c = c->next)
    {
	if (c->uid == uid || c->euid == uid)
	{
	    r = c;
	    c = c->next;
	    break;
	}
    }
    
    *p = c;
    return r;
}

/****************************************/

unsigned long int create_parentship()
{
    unsigned long int cnt=0;
    struct pidlist *q, *t, *c;
    
    q = &plist;
    for (;;)
    {
	c = &plist;
	t = search_bypid(c, q->ppid);
	q->parent = t;
	if (t && t->pid > 1) cnt++;
	if (q->next && q->next->pid) q = q->next;
	else break;
    }
    
    return cnt;
}

/****************************************/

void reset_tree_pathinfo()
{
    struct pidlist *p;

    for (p = &plist ; p && p->pid && p->next ; p = p->next)
	p->checked = 0;
}

/****************************************/

struct pidlist * jump_to_parent(struct pidlist *cur)
{
    if (!cur || !(cur->next) || !(cur->pid)) return NULL;
//    if (cur->checked)
//    {
//	sayg(stderr, "X");
//	return NULL; /* path marked as bad */
//    }
    if (!(cur->parent))
	return NULL;

    cur = cur->parent;
    return cur;
}

/****************************************/
