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

#include "dirs.h"

DIR
*dopendir(const char *name)
{	/* open a dir with error handling */
	DIR *dir = opendir(name);
	if (!dir) {
		perror(name);
		exit(EXIT_FAILURE);
	}
	return dir;
} // dopendir()

int
recursedir(char *dirname, mdata *ddat)
{	/* Returns count of records recorded.
	* Caller must initialise fsoselect and meminc
	* before calling this function.
	*/
	size_t meminc = 1048576;	// 1 meg seem reasonable.

	DIR *dp = dopendir(dirname);
	static int recs = 0;
	struct dirent *de;
	while ((de = readdir(dp))) {
		if (strcmp(de->d_name, ".") == 0 ) continue;
		if (strcmp(de->d_name, "..") == 0) continue;
		if (de->d_type == DT_DIR) {
			if(inlist(de->d_name, except)) continue;
		}
		//fprintf(stderr, "(b4)dirname %s\n", dirname);
		if (!(inarray(de->d_type, fsoselect))) continue;
		//fprintf(stderr, "(inarray)dirname %s\n", dirname);
		char *fsoname = join(dirname, de->d_name);
		//fprintf(stderr, "(after join)dirname %s\n", dirname);
		meminsert(fsoname, ddat, meminc);
		/*fprintf(stderr, "dirname %s\nd_name %s d_type %d\n"
						"fsoname %s\n\n",
						dirname, de->d_name, de->d_type, fsoname);
		int ans = getc(stdin); */
		recs++;
		if (de->d_type == DT_DIR) {
			recursedir(fsoname, ddat);
			free(fsoname);
		} else free(fsoname);
	} // while()
	doclosedir(dp);
	return recs;
} // recursedir()

/*
 * for fstyp below use DT_BLK, DT_CHR, DT_DIR, DT_FIFO, DT_LNK, DT_REG,
 * DT_SOCK, DT_UNKNOWN as required. Most needs will be met by DT_DIR,
 * DT_LNK, and DT_REG. DT_DIR will always be needed otherwise the
 * recursion can never happen.
 * excludes may be NULL and if it is there will be no dirs excluded from
 * the output.
 * */
void
init_recursedir(mdata *dd, char **excludes, .../* last varg MUST be
												0xff, terminator */)
{	/* prepare to use recursedir() */
	initmdat(dd);
	meminc = 1024 * 1024;	// 1 meg seems reasonable
	except = excludes;
	initfsoslect(fsoselect);
	va_list ap;
	va_start(ap, excludes);
	int i = 0;
	while (1) {
		unsigned char res = va_arg(ap, int);
		if (res == 0xff) break;
		fsoselect[i] = res;
		i++;
	}
	va_end(ap);
} // init_recursedir()

int
inlist(const char *name, char **list)
{	/* return 1 if name is found in list, 0 otherwise. */
	if (!list) return 0;	// list may not have been made.

	int i = 0;
	while (list[i]) {
		if(strcmp(name, list[i]) == 0) return 1;
		i++;
	}
	return 0;
} // inlist()

char
*join(const char *dirname, const char *newname)
{	/* Concatenate dirname and newname inserting '/' between. */
	char buf[PATH_MAX];
	size_t len = strlen(dirname);
	strcpy(buf, dirname);
	char *eol = buf + len;
	if(*eol != '/') {
		*eol = '/';
		eol++;
	}
	strcpy(eol, newname);
	return strdup(buf);
} // join()

int
inarray(const unsigned char c, unsigned char *buf)
{	/* Return 1 if c is in buf, 0 otherwise */
	unsigned char *cp = buf;
	while (*cp != 0xff) {
		if(*cp == c) return 1;
		cp++;
	}
	return 0;
} // inarray()

void
initmdat(mdata *dd)
{	/* 0/NULL elements of dd */
	dd->fro = dd->to = dd->limit = (char *)NULL;
} // initmdat()

void
initfsoslect(unsigned char fsoselect[])
{	/* set all element of fsoselect to 0xff */
	int i;
	for (i = 0; i < 9; i++) {
		fsoselect[i] = 0xff;
	}
} // initfsoslect()

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
newdir(const char *p)
{ /* mkdir() with error handling. Also have hard wired mode. */
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

