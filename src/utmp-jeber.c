static const char rcsid[] = "$Id: utmp-jeber.c,v 1.5 2003/02/12 13:07:38 siefca Exp $";
/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#include "includes.h"
#include "defines.h"

#include "proc.h"
#include "pidlist.h"
#include "settings.h"

/****************************************/

void usage(char *n);
void show_version();
void show_legend();
void show_header();
void say_used_checks(const char *prefix, const char separator);
void try_permissions(const char *utmp_org_file,
		     const char *utmp_file, const char *procroot);

/****************************************/

int main (int argc, char *argv[])
{
    unsigned long int n_proc, n_orp;
    int c=0;
    char x;
    const char *r, *t;
    struct utmp *utp;
    struct utmp utw;

    init_settings();
    plist.next = NULL;
    
    if (argc > 1)
    while ((x=getopt(argc, argv, "Vhsqdbaijulmntx")) != EOF)
	{
		switch (x)
		    {
		    case 'V':
			    show_version();
			    exit(0);
			    break;
		    case 'h':
			    usage(argv[0]);
			    exit(0);
			    break;
		    case 's':
			    show_legend();
			    exit(0);
			    break;
		    case 'd':
			    ST.debug = 1;
			    fprintf(stderr, "[debug mode enabled!]\n");
			    break;
		    case 'q':
			    ST.verb = 0;
			    break;
		    case 'b':
			    ST.batched = 1;
			    break;
		    case 'x':
			    ST.justprint = 0;
			    break;
		    case 'u':
			    ST.test_user = 1;
			    break;
		    case 'l':
			    ST.test_ex = 1;
			    break;
		    case 't':
			    ST.test_term = 1;
			    break;
		    case 'i':
			    ST.test_inherit = 1;
			    break;
		    case 'j':
			    ST.test_inh_term = 1;
			    break;
		    case 'm':
			    ST.test_pgid = 1;
			    break;
		    case 'n':
			    ST.test_pgid_term = 1;
			    break;
		    case 'a':
			    ST.test_all = 1;
			    break;
		    default :
			    usage(argv[0]);
			    exit(0);
			    break;
		}
	}

    postconf_settings();
    show_header();
    try_permissions(UTMP_ORG_NAME, UTMP_NAME, PROC_MOUNT);
    utmpname(ST.utmp_file);

    say("-( scanning processes table: ");
    n_proc = fetch_procs();
    n_orp = n_proc - create_parentship();
    if (n_orp < 0) n_orp = 0;

    say("%ld processes (%ld with init as parent)\n", n_proc, n_orp);
    say("-( methods: ");
    say_used_checks("", ' ');
    say("\n");
    say("-( scanning utmp for bad entries\n");
    
    setutent();
    while ((utp=getutent()))
    {
	if (utp->ut_type != USER_PROCESS) continue;
	t = "none";
	sayg(stderr, "\n[checking %s on %s]\n", utp->ut_user, utp->ut_line);
	if ((r=badproc(utp)))
	{
	    if (!c) say("\nbad entries:\n");
	    c++;
	    say(" %d. %s on %s with PID=%d (%s)",
		c,
		utp->ut_user,
		utp->ut_line,
		(unsigned int) utp->ut_pid, r);
	    if (ST.justprint)
	    {
		say("\n");
	    }
	    else
	    {
		/* da killin' code here */
		(void) memcpy(&utw, utp, sizeof(struct utmp));
		utw.ut_type = DEAD_PROCESS;
		if (pututline(&utw)) t = "removed";
		else t = "unable to remove";
		say(" [%s]\n", t);
	    }
	    log("user=%s tty=/dev/%s pid=%d action=%s reason=%s\n",
		utp->ut_user,
		utp->ut_line,
		utp->ut_pid,
		t, r);
	}
    }
    if (c) say("\n");
    endutent();

    say("-( scanning completed\n");
    destroy_list();
    exit(0);
}

/****************************************/

void usage(char *n)
{
    show_header();
    fprintf(stderr, "usage: %s [options]\n\n"
	    "* printing options:\n"
	    "\t-b  produce batched output\n"
	    "\t-d  enable debugging\n"
	    "\t-h  show this screen\n"
	    "\t-s  show short legend of available checks\n"
	    "\t-q  enable quiet mode\n"
	    "\t-V  show version and copying information\n\n"
	    "* workmode options:\n"
	    "\t-a  perform all safe checks (default)\n"
	    "\t-i  processes inheritance check\n"
	    "\t-j  processes inheritance check + line test\n"
    	    "\t-l  login process existance check\n"
	    "\t-m  processes PGID check\n"
	    "\t-n  processes PGID check + line test\n"
	    "\t-t  terminal line check\n"
	    "\t-u  user processes existance check\n"
	    "\t-x  remove bad entries from utmp\n\n"
	    "* default checks are:\n", n);
    ST.test_all = 1;
    postconf_settings();
    say_used_checks("\t- ", '\n');
}

/****************************************/

void say_used_checks(const char *prefix, const char separator)
{
    if (ST.test_ex) { say("%sl-seek%c", prefix, separator); }
    if (ST.test_user) { say("%su-seek%c", prefix, separator); }
    if (ST.test_pgid) { say("%spgid-check%c", prefix, separator); }
    if (ST.test_pgid_term) { say("%spgid+line-check%c", prefix, separator); }
    if (ST.test_inherit) { say("%sinherit-check%c", prefix, separator); }
    if (ST.test_inh_term) { say("%sinherit+line-check%c", prefix, separator); }
    if (ST.test_term) { say("%sline-check%c", prefix, separator); }
}

/****************************************/

void show_legend()
{
    show_header();
    printf("Used checks and their meanings:\n\n"
	   "l-seek (s)             - checks whether login process for an entry exists\n"
	   "u-seek (s)             - checks whether any process for a user exists\n"
	   "pgid-check (u)         - checks whether there is any proccess with valid PGID\n"
	   "pgid+line-check (v)    - works as above but also needs a valid terminal line\n"
	   "inherit-check (s)      - check whether there is any process with valid process inh. path\n"
	   "inherit+line-check (s) - works as above but also needs a valid terminal line\n"
	   "line-check (n)         - checks whether there is any process with valid terminal line\n"
	   "\n"
	   "(s) safe        - no mistakes should be generated\n"
	   "(u) unsafe      - mistakes may occur sometimes\n"
	   "(v) very unsafe - so restrictive that mistakes may occur often\n"
	   "(n) neutral     - no mistakes, but may not show all bad entries\n\n"
	    "Default checks are:\n");
	    ST.test_all = 1;
	    postconf_settings();
	    say_used_checks("\t- ", '\n');
}

/****************************************/

void show_version()
{
    printf("This is UTMP Jeber, version " PACKAGE_VERSION "\n"
	   "This program is Copyright (c) 2003 by Pawel Wilk <siefca@gnu.org>\n\n"
	    "This program is free software; you can redistribute it and/or modify\n"
	    "it under the terms of the GNU General Public License as published by\n"
	    "the Free Software Foundation; either version 2 of the License, or\n"
	    "(at your option) any later version.\n\n"
	    "This program is distributed in the hope that it will be useful,\n"
	    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	    "GNU General Public License for more details.\n\n"
	    "You should have received a copy of the GNU General Public License\n"
	    "along with this program; if not, write to the\n"
	    "Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n"
	    "Boston, MA  02111-1307  USA\n");

}

/****************************************/

void show_header()
{
    say("UTMP Jeber v" PACKAGE_VERSION " by Pawel Wilk <siefca@gnu.org> [GNU GPL Licensed]\n");
}

/****************************************/

void try_permissions(const char *utmp_org_file,
		     const char *utmp_file, const char *procroot)
{
    int have_u_write = 1;

#ifdef NEED_SUPERUSER
    if (geteuid())
    {
	show_header();
	fprintf (stderr, "error: must be root in order to run me!\n");
	exit(1);
    }
#endif
    if (access(utmp_file, R_OK))
    {
	if (access(utmp_org_file, R_OK))
	{
	    fprintf (stderr, "error: cannot read UTMP file (%s)\n", utmp_file);
	    fprintf (stderr, "error: cannot read UTMP original file (%s)\n", utmp_org_file);
    	    exit(1);
	}
	else
	{
	    ST.utmp_file = utmp_org_file;
	}
    }
    else
    {
	ST.utmp_file = utmp_file;
    }
    
    if (access(ST.utmp_file, W_OK))
    {
        if (!ST.justprint)
	{
	    say("-( warning: cannot write UTMP file - fixes disabled\n");
	    ST.justprint = 1;
	    have_u_write = 0;
	}
    }
    
    if (access(procroot, R_OK))
    {
	fprintf (stderr, "error: cannot read procfs (%s)\n", procroot);
	exit(1);
    }

    sayg(stderr, "[ UTMP file: %s (have write=%d)]\n", ST.utmp_file, have_u_write);
    sayg(stderr, "[ procfs root: %s]\n", procroot);
}

/****************************************/
