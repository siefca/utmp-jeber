/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct settings
	{
	const char *utmp_file;		/* detected name of UTMP file		*/
	
	unsigned verb:1;		/* display some info			*/
	unsigned batched:1;		/* batched output			*/
	unsigned debug:1;		/* debug mode				*/
	unsigned justprint:1;		/* do not remove, just inform		*/

	unsigned test_ex:1;		/* login process existance test		*/
	unsigned test_user:1;		/* user process existance test		*/
	unsigned test_term:1;		/* terminal ownership test		*/
	unsigned test_inherit:1;	/* processes inheritance test		*/
	unsigned test_inh_term:1;	/* processes inheritance + terminal 	*/
	unsigned test_pgid:1;		/* processes PGID test			*/
	unsigned test_pgid_term:1;	/* processes PGID test + terminal 	*/
	unsigned test_all:1;		/* all possible tests		 	*/
	} sts;

extern sts ST;

void init_settings();
void postconf_settings();
void disable_line_checks();

#endif
