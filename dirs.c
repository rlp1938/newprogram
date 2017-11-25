/*    dirs.c
 *
 * Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
*/

/* The purpose of dirs.[h|c] is to provide a set of utility functions
 * for manipulating directories. For other file system objects see
 * files.[h|c].
 * */

#include "dirs.h"

DIR
*dopendir(const char *name)
{ /* open a dir with error handling */
	DIR *dir = opendir(name);
	if (!dir) {
		perror(name);
		exit(EXIT_FAILURE);
	}
	return dir;
} // dopendir()

int
recursedir(char *dirname, mdata *ddat, rd_data *rd)
{ /* Returns count of records recorded.
	* Caller must init_recursedir() before calling this.
	*/
	DIR *dp = dopendir(dirname);
	static int recs = 0;
	struct dirent *de;
	while ((de = readdir(dp))) {
		if (strcmp(de->d_name, ".") == 0 ) continue;
		if (strcmp(de->d_name, "..") == 0) continue;
		// Process only file system objects named in rd->fsobj[]
		if (!in_uch_array(de->d_type, rd->fsobj)) continue;
		char joinbuf[PATH_MAX];
		strcpy(joinbuf, dirname);
		strjoin(joinbuf, '/', de->d_name, PATH_MAX);
		/* If there is list of paths to reject check that any dirs
		 * found are not in rd->rejectlist[] */
		if ((rd->rejectlist) && de->d_type == DT_DIR) {
			if(inlist(joinbuf, rd->rejectlist)) continue;
		}
		meminsert(joinbuf, ddat, rd->meminc);
		recs++;
		if (de->d_type == DT_DIR) {
			recursedir(joinbuf, ddat, rd);
		}
	} // while()
	doclosedir(dp);
	return recs;
} // recursedir()

/*
 * For fsobj below use DT_BLK, DT_CHR, DT_DIR, DT_FIFO, DT_LNK, DT_REG,
 * DT_SOCK, DT_UNKNOWN as required.
 * Most needs will be met by DT_DIR and DT_REG.
 * DT_DIR will always be needed else the recursion can never happen.
 * Excludes may be NULL and if it is there will be no dirs excluded from
 * the output.
 * */

rd_data
*init_recursedir(char **excludes, size_t meminc, ...)
/* vargs are list of fsobj, must terminate with 0 */
{ /* prepare to use recursedir() */
	rd_data *rd = xmalloc(sizeof(rd_data));
	memset(rd, 0, sizeof(rd_data));
	rd->meminc = meminc;
	if (excludes) {	// excludes may be NULL
		size_t i, n;
		for (i = 0; excludes[i] ; i++);
		n = i + 1;
		rd->rejectlist = xmalloc(n * sizeof(char *));
		memset(rd->rejectlist, 0, n * sizeof(char *));
		for (i = 0; i < n; i++)
			rd->rejectlist[i] = realpath(excludes[i], NULL);
	} // if()
	int i = 0;
	va_list ap;
	va_start(ap, meminc);
	unsigned char ch;
	while (1) {
		ch = va_arg(ap, unsigned);
		rd->fsobj[i] = ch;
		i++;
		if (ch == 0) break;
	}
	va_end(ap);
	return rd;
} // init_recursedir()

void
free_recursedir(rd_data *rd, mdata *md)
{ /* free resources allocated by init_recursedir() */
	if (rd->rejectlist) {
		int i;
		for (i = 0; rd->rejectlist[i]; i++) free(rd->rejectlist[i]);
		free(rd->rejectlist);
	}
	free(rd);
	free(md->fro);
	free(md);
} // free_recursedir()

void
doclosedir(DIR *dp)
{	/* closedir() with error handling */
	int res = closedir(dp);
	if (res == -1) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}
} // doclosedir

void
newdir(const char *p, int mayexist)
{ /* mkdir() with error handling. Also have hard wired mode. */
	if (mayexist) {
		if(exists_dir(p)) return;
	}
	const int crmode = 0775;	// stat yielded this value.
	if (mkdir(p, crmode) == -1) {
		perror(p);
		exit(EXIT_FAILURE);
	}
} // newdir()

void
xchdir(const char *path)
{/* Just chdir() with error handling. */
	if (chdir(path) == -1) {
		perror(path);
		exit(EXIT_FAILURE);
	}
} // xchdir()

int
exists_dir(const char *path)
{ /* return 1 if the dir exists, 0 otherwise */
	struct stat sb;
	int res = stat(path, &sb);
	if (res == -1) {
		res = 0;
	} else {
		res = 1;
	}
	if (res && S_ISDIR(sb.st_mode)) {
		res = 1;
	} else {
		res = 0;
	}
	return res;
} // exists_dir()
