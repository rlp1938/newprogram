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

int checkfirstrun(char *progname, char **names)
{
	// construct the user's path to .config
	char upath[PATH_MAX];
	sprintf(upath, "%s/.config/%s/", getenv("HOME"), progname);
	if (!exists_dir(upath)) return 0;
	int i;
	for (i = 0; names[i]; i++) {
		char buf[PATH_MAX];
		strcpy(buf, upath);
		strjoin(buf, '/', names[i], PATH_MAX);
		if(!exists_file(buf)) return 0;
	}
	return 1;
} // checkfirstrun()

void firstrun(char *progname, char **names)
{
	/* The purpose of this function is to copy a caller specified set
	 * of files from /usr/local/share/<progname>/ to
	 * $HOME/.config/<progname>/
	 * The assumption is that the user who built the program has not
	 * meddled with --prefix at configure time. Anyone who has the
	 * confidence to do this will no doubt have the ability to make
	 * the necessary copies by hand instead of relying on this feature.
	*/
	// To
	char pathto[PATH_MAX];
	sprintf(pathto, "%s/.config/%s/", getenv("HOME"), progname);
	if (!exists_dir(pathto)) {
		newdir(pathto, 0);
	} else {
		rmconfigs(pathto);
	}
	size_t tolen = strlen(pathto);
	// From
	char pathfr[PATH_MAX];
	sprintf(pathfr, "/usr/local/share/%s/", progname);
	size_t frlen = strlen(pathfr);
	// File copy
	int i;
	for (i = 0; names[i]; i++) {
		strcpy(pathfr + frlen, names[i]);
		strcpy(pathto + tolen, names[i]);
		copyfile(pathfr, pathto);
	}
} // firstrun()

void rmconfigs(char *cfgdir)
{/* This is run to ensure that there are no unused config files left. */
	mdata *md = init_mdata();
	rd_data *rd = init_recursedir(NULL, 2*PATH_MAX, DT_REG, 0);
	int res = recursedir(cfgdir, md, rd);
	if (res) {
		char *cp = md->fro;
		while (cp < md->to) { // redundant testing
			if (exists_file(cp)) dounlink(cp);
			cp += strlen(cp) + 1;
		}
	}
	free_recursedir(rd, md);
} // rmconfigs()
