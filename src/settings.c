/* utmp-jeber: remove broken entries from UTMP
 *
 * GNU/Linux program, 
 * Copyright (C) 2003 Pawel Wilk <siefca@gnu.org>,
 *
 * This is free software; see the GNU General Public License version 2
 * or later for copying conditions.  There is NO warranty.
 *
 */

#include "settings.h"

sts ST;

/****************************************/

void init_settings()
{
    memset((void *) &ST, 0, sizeof(ST));
    ST.justprint = 1;
    ST.verb = 1;
}

/****************************************/

void postconf_settings()
{
    if (!ST.test_ex && !ST.test_user && !ST.test_term &&
	!ST.test_inherit && !ST.test_inh_term &&
	!ST.test_pgid && !ST.test_pgid_term)
	ST.test_all = 1;

    if (ST.batched)
	ST.verb = 0;

    if (ST.test_all)
    {
	ST.test_ex = 1;
	ST.test_user = 1;
	ST.test_inherit = 1;
	ST.test_inh_term = 1;
    }
    
    if (ST.test_inherit)
	ST.test_ex = 1;

    if (ST.test_pgid_term)
    {
	ST.test_term = 0;
	ST.test_pgid = 0;
    }

    if (ST.test_inh_term)
    {
	ST.test_ex = 1;
	ST.test_user = 1;
	ST.test_term = 0;
	ST.test_inherit = 0;
    }
}

/****************************************/
