static const char rcsid[] = "$Id: proc.c,v 1.7 2003/02/25 18:09:18 siefca Exp $";

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

int check_perm_foreign_process(uid_t my_uid, unsigned int min_hits)
{
  int x=0;
  unsigned int hit_counter=0;
  pid_t pid;
  uid_t uid, euid;
  FILE *f;
  DIR *d;
  struct dirent *dr;
  char buf[1024];
  char pathbuf[MAXPATHLEN];
  
  sayg(stderr, "[looking for a foreign process]\n");
  d = opendir(PROC_MOUNT);
  if (!d)
    {
      sayg(stderr, "[opendir "PROC_MOUNT" succeeded]\n");
      return 0;
    }
  while ((dr=readdir(d)))
    {
      if (!dr->d_name) continue;
      snprintf(pathbuf, MAXPATHLEN-2, "%s/%s/status",
	       PROC_MOUNT,
	       dr->d_name);
      pathbuf[MAXPATHLEN-1] = '\0';
      
      f = fopen(pathbuf, "r");
      if (!f) continue;
      sayg(stderr, "[fopen for proc-entry %s succeeded] ", pathbuf);
      x = 0;
      pid = 0;
      while (!feof(f) && !ferror(f))
        { 
          if (x == 3) break;
          fgets(buf, 1023, f);
          buf[1023] = '\0';
          if (sscanf(buf, "Pid:\t%d\n", &pid) == 1) { x|=1; continue; }
	  if (sscanf(buf, "Uid:\t%d\t%d\t", &uid, &euid) == 2) { x|=2; continue; }
        }
      if (x != 3) { fclose(f); continue; }
      fclose(f);
      if (!pid) continue;
      sayg(stderr, "[have pid, uid, euid]\n");
      if (my_uid != uid && my_uid != euid)
	{
	  sayg(stderr,
	       "[foreign process was found (uid=%d euid=%d, pid=%d)]\n"
	       "[calling termfind(%d, NULL)]\n",
	       (unsigned int) uid,
    	       (unsigned int) euid,
	       (unsigned int) pid,
	       (unsigned int) pid);
	  x = termfind(pid, NULL);
	  if (x == TERMFIND_NO_CHAR || x == TERMFIND_NO_PROC)
	    continue;
	  if (x == TERMFIND_OK && hit_counter++ >= min_hits)
	    {
	      sayg(stderr, "[termfind(): can check foreign descriptors]\n");
	      closedir(d);
	      return 1; /* can check foreign descriptors */
	    }
	 if (x == TERMFIND_FAILED)
	    {
	      sayg(stderr, "[termfind(): cannot check foreign descriptors]\n");
	      closedir(d);
	      return 0;
	    }
	}
    }

  closedir(d);
  return 0; /* processes visited but no occasion */
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
	      if (termfind(o->pid, device) == TERMFIND_OK)
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
	      if (termfind(startproc, device) == TERMFIND_OK)
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
		  if (termfind(q->pid, device) == TERMFIND_OK)
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
    if (o->pid && termfind(o->pid, device) == TERMFIND_OK)
	return 1;
  
  return 0;
}

/****************************************/

int termfind (pid_t pid, const char *device)
{
  int f;
  DIR *d;
  char was_chardevice;
  const char *c;
  struct dirent *dr;
  struct stat statbuf;
  static char pathbuf[MAXPATHLEN];
  static char linkbuf[MAXPATHLEN];
  
  snprintf(pathbuf, MAXPATHLEN-2, "%s/%d/%s", PROC_MOUNT, pid, P_SUF);
  pathbuf[MAXPATHLEN-1] = '\0';
  if (access(pathbuf, F_OK))
    {
      sayg(stderr, " (no such proc at termfind) ");
      return TERMFIND_NO_PROC; /* no such proccess */
    }
  d = opendir(pathbuf);
  if (!d)
    {
      sayg(stderr, " (opendir failed at termfind) (dir=%s)\n", pathbuf);
      return TERMFIND_FAILED; /* failure -- better leave */
    }
  was_chardevice = 0;
  while ((dr=readdir(d)))
    {
      if (!dr->d_name) continue;
      snprintf(pathbuf, MAXPATHLEN-2, "%s/%d/%s/%s", PROC_MOUNT, pid, P_SUF, dr->d_name);
      pathbuf[MAXPATHLEN-1] = '\0';
      if (stat(pathbuf, &statbuf) == -1) continue;
      if (S_ISCHR(statbuf.st_mode))
	{
	  was_chardevice = 1;
	  c = NULL;
	  f = open(pathbuf, O_NOCTTY);
	  if (f == -1)
	    {
	      /* cannot open - it may happend */
	      /* try to read symlink */
	      f = readlink(pathbuf, linkbuf, MAXPATHLEN-2);
	      linkbuf[MAXPATHLEN-1] = '\0';
	      if (f == -1) continue;
	      c = linkbuf; /* got name */
	      if (c)
		{
		  sayg(stderr, " (pid %d passed readlink at termfind()) ", pid);
		  closedir(d);
		  return TERMFIND_OK;
		}
	    }
	  else
	    {
	      if (!device)
	        {
		  /* working in checkmode                  */
		  /* need only readable character device   */
		  /* so it's not necessary to do ttyname() */
		  /* it's even not recommended..           */
		  sayg(stderr, " (pid %d passed half-ttyname at termfind()) ", pid);
		  closedir(d);
		  return TERMFIND_OK;
	        }
	      c = ttyname(f); /* got name */
	      close(f);
	      sayg(stderr, " (pid %d passed ttyname at termfind()) ", pid);
	    }

	  if (c) /* have some terminal */
	    {
	      /* have THAT terminal */
	      if (!strcmp(c,device))
	        {
		  sayg(stderr, " (pid %d has %s) ", pid, device);
		  closedir(d);
		  return TERMFIND_OK;
		}
	    }
	}
    }
  closedir(d);
  if (!was_chardevice)
    return TERMFIND_NO_CHAR;

  return TERMFIND_FAILED;
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
