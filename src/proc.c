static const char rcsid[] = "$Id: proc.c,v 1.3 2003/02/11 13:24:28 siefca Exp $";
/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#include "proc.h"
#include "pgid.h"
#include "pidlist.h"
#include "settings.h"

unsigned long int fetch_procs()
{
    unsigned long int cnt=0;
    char buf[1024];
    static char pathbuf[MAXPATHLEN];
    FILE *f;
    DIR *d;
    struct dirent *dr;
    pid_t pid, ppid;
    uid_t uid, euid;
    unsigned int x;

    d = opendir(PROC_MOUNT);
    if (!d) return 0;

    current_reset();
    while ((dr=readdir(d)))
    {
	if (!dr->d_name) continue;
	snprintf(pathbuf, MAXPATHLEN-2, "%s/%s/status", PROC_MOUNT, dr->d_name);
	pathbuf[MAXPATHLEN-1] = '\0';
	f = fopen(pathbuf, "r");
	if (!f) continue;
	x=0;
	while (!feof(f) && !ferror(f))
	{
	    if (x == 7) break;
	    fgets(buf, 1023, f);
	    buf[1023] = '\0';
	    if (sscanf(buf, "Pid:\t%d\n", &pid) == 1) { x|=1; continue; }
	    if (sscanf(buf, "PPid:\t%d\n", &ppid) == 1) { x|=2; continue; }
	    if (sscanf(buf, "Uid:\t%d\t%d\t", &uid, &euid) == 2) { x|=4; continue; }
	}
	if (x != 7) { fclose(f); continue; }
	fclose(f);
	if (pid)
	    add_list_item(pid, ppid, uid, euid, my_getpgid(pid));
	cnt++;
    }
    closedir(d);
    if (cnt <= 2)
    {
	fprintf(stderr, "utmp-jeber: something went wrong with procfs! aborting!\n");
	destroy_list();
	exit(3);
    }
    return cnt;
}


/****************************************/

int check_proc_pgid(char *username, pid_t startproc, int level, const char *device)
{
    pid_t nextproc;
    struct pidlist *p, *o;
    struct passwd *pwd;
    
    pwd = getpwnam(username);
    if (!pwd) return 0;
    
    nextproc = obtain_unbearable_child(startproc, pwd->pw_uid);

    reset_tree_pathinfo();
    p = list_begin();
    while ((o=search_byuids(&p, pwd->pw_uid)))
    {
#ifdef INIT_NOT_PROVIDE
	if (o->pid == 1) continue;
#endif
	if (o->pid == startproc || o->pgid == startproc || o->pgid == nextproc)
	{
	    sayg(stderr, "*pgid*");
	    if (!device)
		 return 1;
	    else /* extended usage - check term */
	    {
		sayg(stderr, " [call termfind(%d, %s)]", o->pid, device);
		if (termfind(o->pid, device))
		    return 1;
	    }
	}
    }
    return 0;
}

/****************************************/

int check_proc(char *username, pid_t startproc, int level, const char *device)
{
    struct pidlist *p, *o, *q, *w;
    struct passwd *pwd;
    int x=1;
    
    pwd = getpwnam(username);
    if (!pwd) return 0;
    
    p = list_begin();
    p = search_bypid(p, startproc);
    if (!p || !p->pid) return 0;

    reset_tree_pathinfo();
    p = list_begin();
    while ((o=search_byuids(&p, pwd->pw_uid)))
    {
#ifdef INIT_NOT_PROVIDE
	if (o->pid == 1) continue;
#endif
	sayg(stderr, "[ pid=%d uid=%d ]", o->pid, o->uid);

	/* for each of it we must check the roots 		*/
	/* bingo when pid is same as startproc    		*/
	/* it means that our login process from utmp		*/
	/* is alive and keeps living children with our UID	*/

	if (o->pid == startproc) /* do we have good luck? */
	{
	    sayg(stderr, "*");
	    if (!device)
		 return 1;
	    else /* extended usage - check term */
	    {
		sayg(stderr, " [call termfind(%d, %s)]", startproc, device);
		if (termfind(startproc, device))
		    return 1;
	    }
	}

	x = level;
	q = o;			/* keep origin, we may need it	*/
	while (x)		/* less luck.. seek parents	*/
	{
	    w = o;
	    o = jump_to_parent(o);
	    if (!o) break;		/* nothing left in path  */
#ifdef INIT_NOT_PROVIDE
	    if (o->pid == 1) break;
#endif
	    sayg(stderr, "-->[ pid=%d uid=%d ]", o->pid, o->uid);
	    if (o->pid == startproc)	/* got login process */
		{
		    sayg(stderr, "*");
		    if (!device)
			return 1;
		    else /* extended usage - check term */
		    {
		    	sayg(stderr, " [call(2) termfind(%d, %s)]", q->pid, device);
			if (termfind(q->pid, device))
			    return 1;
		    }
		}
	    x--;
	}
	sayg(stderr, "\n");
    }

    return 0;
}

/****************************************/

const char * badproc(struct utmp *ut)
{
    static char pathbuf[MAXPATHLEN], devstring[MAXPATHLEN];
    static const char *reasons[5] = {"login process not found",
				     "no valid terminal line",
				     "invalid inheritance path",
				     "separated process group",
				     "no user processes present"};

    if (!ut || !ut->ut_pid || !ut->ut_line ||
	!ut->ut_user) return NULL;

    snprintf(devstring, MAXPATHLEN-2, "/dev/%s", ut->ut_line);
    devstring[MAXPATHLEN-1] = '\0';

    /* condition: login process doesn't exists */

    if (ST.test_ex)
    {
	snprintf(pathbuf, MAXPATHLEN-2, "%s/%d", PROC_MOUNT, ut->ut_pid);
	pathbuf[MAXPATHLEN-1] = '\0';
	if (access(pathbuf, F_OK)) return reasons[0];
    }

    /* condition: the user (uid) doesn't exists */

    if (ST.test_user)
    {
	if (!check_user_procs(ut->ut_user))
	    return reasons[4];
    }

    /* condition: process's PGID doesn't match login's PID */

    if (ST.test_pgid)
    {
	if (!check_proc_pgid(ut->ut_user, ut->ut_pid, SRCH_DEEP, NULL))
	    return reasons[3];
    }

    /* condition: process's PGID doesn't match login's PID
		  or the process doesn't have a line	  */

    if (ST.test_pgid_term)
    {
	if (!check_proc_pgid(ut->ut_user, ut->ut_pid, SRCH_DEEP, devstring))
	    return reasons[3];
    }

    /* condition: process owner doesn't match	     */
    /*		  or login proces is not an ancestor */

    if (ST.test_inherit)
    {
	if (!check_proc(ut->ut_user, ut->ut_pid, SRCH_DEEP, NULL))
	    return reasons[2];
    }

    /* condition: process owner doesn't match	     */
    /*		  or login proces is not an ancestor */
    /*		  or the process doesn't have a line */

    if (ST.test_inh_term)
    {
	if (!check_proc(ut->ut_user, ut->ut_pid, SRCH_DEEP, devstring))
	    return reasons[2];
    }

    /* condition: terminal line belongs to user */
    
    if (ST.test_term)
    {
	if (!check_terminal_owner(ut->ut_user, devstring, ut->ut_pid))
	    return reasons[1];
    }

    return NULL;
}

/****************************************/

int check_user_procs(const char *username)
{
    struct pidlist *p;
    struct passwd *pwd;
    
    if (!username || *username == '\0') return 0;
    pwd = getpwnam(username);
    if (!pwd) return 0;
    p = list_begin();
    if (search_byuids(&p, pwd->pw_uid))
	return 1;
    return 0;
}

/****************************************/

int check_terminal_owner(const char *username, const char *device, pid_t startproc)
{
    struct pidlist *p, *o;
    struct passwd *pwd;
    
    if (!username || !device)
	return 1;
    
    pwd = getpwnam(username);
    if (!pwd) return 0;

    reset_tree_pathinfo();
    p = list_begin();
    while ((o=search_byuids(&p, pwd->pw_uid)))
	if (o->pid && termfind(o->pid, device)) return 1;

    return 0;
}

/****************************************/

int termfind (pid_t pid, const char *device)
{
    int f;
    DIR *d;
    char *c;
    struct dirent *dr;
    struct stat statbuf;
    static char pathbuf[MAXPATHLEN];

    snprintf(pathbuf, MAXPATHLEN-2, "%s/%d/%s", PROC_MOUNT, pid, P_SUF);
    pathbuf[MAXPATHLEN-1] = '\0';
    if (access(pathbuf, F_OK))
    {
	sayg(stderr, " (no such proc at termfind) ");
	return 1; /* no such proccess */
    }
    d = opendir(pathbuf);
    if (!d)
    {
    	sayg(stderr, " (opendir failed at termfind) ");
	return 0; /* failure -- better leave */
    }
    while ((dr=readdir(d)))
    {
        if (!dr->d_name) continue;
	snprintf(pathbuf, MAXPATHLEN-2, "%s/%d/%s/%s", PROC_MOUNT, pid, P_SUF, dr->d_name);
	pathbuf[MAXPATHLEN-1] = '\0';
	if (stat(pathbuf, &statbuf) == -1) continue;
	if (S_ISCHR(statbuf.st_mode))
	{
	    f = open(pathbuf, O_NOCTTY);
	    if (f == -1) continue;
	    c = ttyname(f);
	    close(f);
	    if (c) /* have some terminal */
	    {
	        if (!strcmp(c,device)) /* have THAT terminal! */
	        {
		    sayg(stderr, " (pid %d has %s) ", pid, device);
		    closedir(d);
		    return 1;
		}
	    }
	}
    }
    closedir(d);
    return 0;
}

/****************************************/

pid_t obtain_unbearable_child(pid_t startproc, uid_t uid)
{
    struct pidlist *p, *o;
    
    
    p = list_begin();
    while ((o=search_byuids(&p, uid)))
    {
#ifdef INIT_NOT_PROVIDE
	    if (o->pid == 1 || !(o->parent)) continue;
#endif
	if (o->pgid != startproc && o->parent->pgid == startproc)
	    return o->pid;
    }
    return startproc;
}

/****************************************/
