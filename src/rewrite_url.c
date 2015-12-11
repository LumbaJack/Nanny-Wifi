/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 1999 George Talusan <gstalusan@uwaterloo.ca>
 * Copyright (C) 2002 James E. Flemer <jflemer@acm.jhu.edu>
 * Copyright (C) 2002 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* A substring of the domain to be filtered goes into the file
 * pointed at by DEFAULT_FILTER.
 */

#include "main.h"
#include "heap.h"
#include "hashmap.h"
#include "log.h"
#include "reqs.h"
#include "conf.h"
#include "rewrite_url.h"

static int err;

struct lst_tg {
	const char *reg;
	const char *sad;
	regex_t *cpat;
};

struct lst_tg lst[] = {
        		{"(search\\.live\\..*\\/.*q=.*)" , "&adlt=strict", 0 },
        		{"(search\\.msn\\..*\\/.*q=.*)" , "&adlt=strict", 0 },
        		{"(\\.bing\\..*\\/.*q=.*)" , "&adlt=strict", 0 },
        };


static int already_init = 0;

/*
 * Initializes a linked list of strings containing hosts/urls to be filtered
 */
void rewrite_init (void)
{
        int cflags, i;

        if (already_init) {
                return;
        }
        cflags = REG_NEWLINE | REG_NOSUB | REG_ICASE | REG_EXTENDED;

        for(i=0; i<3; i++) {
        	lst[i].cpat = (regex_t *) safemalloc (sizeof (regex_t));
        	err = regcomp (lst[i].cpat, lst[i].reg, cflags);
        	if (err != 0) {
        		fprintf (stderr,
        	             "Bad regex %s\n", lst[i].reg);
        	} 
        }
        already_init = 1;
}

/* returns 0 to allow, non-zero to block */
char *rewrite_url (const char *url)
{

	int i,result;
	char *newurl;
        if (!already_init)
                goto COMMON_EXIT;

        for (i = 0; i < 3; i++) {
                result =
                    regexec (lst[i].cpat , url, (size_t) 0, (regmatch_t *) 0, 0);
                if (result == 0) {
			newurl = (char *) safemalloc
                                        (strlen (url) +
                                         strlen (lst[i].sad) +
                                         1);

			strcpy(newurl, url);
			strncat(newurl, lst[i].sad, strlen(lst[i].sad));
                        return newurl;
		}
        }

COMMON_EXIT:
        return NULL;
}

static int
add_header (hashmap_t hashofheaders, char *header, size_t len)
{
        char *sep;
        log_message (LOG_INFO, "header %s ", header);

        /* Get rid of the new line and return at the end */
        len -= chomp (header, len);

        sep = strchr (header, ':');
        if (!sep)
                return -1;

        /* Blank out colons, spaces, and tabs. */
        while (*sep == ':' || *sep == ' ' || *sep == '\t')
                *sep++ = '\0';

        /* Calculate the new length of just the data */
        len -= sep - header - 1;

        log_message (LOG_INFO, "header %s %s", header, sep);

        return hashmap_insert (hashofheaders, header, sep, len);
}

