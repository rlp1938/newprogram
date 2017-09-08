/* firstrun.c
 * Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
*/

#include "firstrun.h"

int checkfirstrun(char *progname, ...)
{
	va_list ap;
	// construct the user's path to .config
	char upath[PATH_MAX];
	sprintf(upath, "%s/.config/%s/", getenv("HOME"), progname);
	if (!exists_dir(upath)) return 0;
	size_t len = strlen(upath);
	va_start(ap, progname);
	char *cp;
	while (1) {
		cp = va_arg(ap, char *);
		if(!cp) break;	// last va must be NULL
		strcpy(upath + len, cp);
		if(!exists_file(upath)) return 0;
	}
	va_end(ap);
	return 1;
} // checkfirstrun()

void firstrun(char *progname, ...)
{
	/* The purpose of this function is to copy a caller specified set
	 * of files from /usr/local/share/<progname>/ to
	 * $HOME/.config/<progname>/
	 * The assumption is that the user who built the program has not
	 * meddled with --prefix at configure time. Anyone who has the
	 * confidence to do this will no doubt have the ability to make
	 * the necessary copies by hand instead of relying on this feature.
	*/
	va_list ap;
	// To
	char pathto[PATH_MAX];
	sprintf(pathto, "%s/.config/%s/", getenv("HOME"), progname);
	if(!exists_dir(pathto)) newdir(pathto);
	size_t tolen = strlen(pathto);
	// From
	char pathfr[PATH_MAX];
	sprintf(pathfr, "/usr/local/share/%s/", progname);
	size_t frlen = strlen(pathfr);

	va_start(ap, progname);	// allow this to work with 0 named files.
	char *cp;
	while (1) {
		cp = va_arg(ap, char *);
		if (!cp) break;	// last va must be NULL.
		strcpy(pathto + tolen, cp);	// To
		strcpy(pathfr + frlen, cp);	// From
		copyfile(pathfr, pathto);
	}
	va_end(ap);
} // firstrun()

void dosystem(const char *cmd)
{
	const int status = system(cmd);

    if (status == -1) {
        fprintf(stderr, "system failed to execute: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        fprintf(stderr, "%s failed with non-zero exit\n", cmd);
        exit(EXIT_FAILURE);
    }

    return;
} // dosystem()
