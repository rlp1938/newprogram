/*     newprogram.c
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdarg.h>
#include <getopt.h>
#include <ctype.h>
#include <limits.h>
#include <linux/limits.h>
#include <libgen.h>
#include <errno.h>

typedef struct progid { /* vars to use in Makefile.am etc */
	char *dir;	// directory name.
	char *exe;	// program name.
	char *src;	// source code name.
	char *thr;	// three letter abbreviation.
	char *man;	// man page name.
} progid;

#include "dirs.h"
#include "files.h"
#include "gopt.h"
#include "firstrun.h"

static progid *makeprogname(const char *);
static void ulstr(int, char *);
static void destroyprogid(progid *);
static void writemakefile_am(progid *, char *);
static void updmakefile_am(char *, char *);
static char *swdepends(char *optslist);
static char *cfgpath(char *, char *, char *);
static char *makefullpath(char *, char *);
static void linkorcopy(const char *, const char *, char *);
static void extramakefile_am(char *);

int main(int argc, char **argv)
{	/* newprogram - write the initial files for a new C program. */
	if (!checkfirstrun("newprogram", "am.mak", "paths.cfg", NULL)) {
		firstrun("newprogram", "am.mak", "paths.cfg", NULL);
		fprintf(stderr,
				"Please edit paths.cfg in %s to meet your needs.\n",
				getenv("HOME"));
		exit(EXIT_SUCCESS);
	}

	options_t opt = process_options(argc, argv);
	progid *pi = makeprogname(argv[optind]);
	printf("%s %s %s %s %s\n",pi->dir, pi->exe, pi->src, pi->man,
			pi->thr);
	char *prog = cfgpath("newprogram", "paths.cfg", "progdir");
	char *stub = cfgpath("newprogram", "paths.cfg", "stubdir");
	char *comp = cfgpath("newprogram", "paths.cfg", "compdir");
	char *tmp = pi->dir;	// preserve it to free() it.
	pi->dir = makefullpath(prog, pi->dir);
	free(tmp);
	char *compdir = makefullpath(prog, comp);
	char *stubdir = makefullpath(prog, stub);
	vfree(comp, stub, prog, NULL);
	printf("%s\n%s\n%s\n", pi->dir, compdir, stubdir);
	newdir(pi->dir);
	xchdir(pi->dir);
	writemakefile_am(pi, "am.mak");	// from $HOME/.config/...
	char *extras = swdepends(opt.software_dependencies);
	updmakefile_am(pi->exe, extras);
	linkorcopy(stubdir, compdir, extras);
	if (opt.extra_data) {
		extramakefile_am(opt.extra_data);
		free(opt.extra_data);
	}
	vfree(compdir, stubdir, NULL);
	free(extras);
	destroyprogid(pi);
	return 0;
}

progid
*makeprogname(const char *pname)
{	/* Create and fill in the progid struct with the values needed in
	 * Makefile.am, ->exe = name, ->src = name.c, ->man = name.1
	 * and ->thr = nam .
	*/
	char name[NAME_MAX], lcname[NAME_MAX];
	progid *prid = malloc(sizeof(progid));
	if (!prid) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	strcpy(name, pname);
	ulstr('l', name);
	strcpy(lcname, name);	// keep pristine lower case copy
	prid->exe = xstrdup(name);
	strcat(name, ".c");
	prid->src = xstrdup(name);
	strcpy(name, lcname);
	strcat(name, ".1");
	prid->man = xstrdup(name);
	name[3] = 0;
	prid->thr = xstrdup(name);	// 3 letter abbreviation
	strcpy(name, lcname);
	name[0] = toupper(name[0]);
	prid->dir = xstrdup(name);
	return prid;
} // makeprogname()

void
ulstr(int ul, char *b)
{	/* convert b to upper or lower case depending on value of ul */
	size_t i;
	switch (ul)
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
		case 'u':
			for (i = 0; i < strlen(b); i++) {
				b[i] = toupper(b[i]);
			}
			break;
		case 'l':
			for (i = 0; i < strlen(b); i++) {
				b[i] = tolower(b[i]);
			}
			break;
#pragma GCC diagnostic pop
		default:
			b[0] = 0;	// trash the input string if ul is rubbish.
			break;
	}
} // ulstr()

void
destroyprogid(progid *pi)
{
	vfree(pi->exe, pi->src, pi->man, pi->thr, pi->dir, pi, NULL);
} // destroyprogid()

void
writemakefile_am(progid *pi, char *amstub)
{	/* Reads the Makefile.am stub , amstub and fills in the names
	 * recorded at pi->, then writes Makefile.am
	*/
	const off_t meminc = 512;	// plenty for this job
	mdata  *amsdat = getconfigfile("newprogram", amstub);
	memreplace(amsdat, "exe%s", pi->exe, meminc);
	memreplace(amsdat, "src%s", pi->src, meminc);
	memreplace(amsdat, "man%s", pi->man, meminc);
	memreplace(amsdat, "thr%s", pi->thr, meminc);
	writefile("Makefile.am", amsdat->fro, amsdat->to, "w");
	free(amsdat->fro);
	free(amsdat);
} // writemakefile_am()

void
updmakefile_am(char *pname, char *swdeplist)
{	/* Find ???_SOURCES= in Makefile.am and insert swdeplist before
	 * the line end in that line
	*/
	if (!swdeplist) return;
	char srchfor[NAME_MAX];
	// Relies on exact formatting of _SOURCES
	sprintf(srchfor, "%s_SOURCES=%s.c", pname, pname);
	size_t slen = strlen(srchfor);
	size_t ilen = strlen(swdeplist);
	// read the file in, with enough extra space for the insert (ilen).
	mdata  *amdat = readfile("Makefile.am", 1, ilen);
	char *ip = memmem(amdat->fro, amdat->to - amdat->fro, srchfor,
						slen);
	if (!ip) {
		fputs("Could not find ???_SOURCES in Makefile.am.\n", stderr);
		exit(EXIT_FAILURE);
	}
	ip = memchr(ip, '\n', amdat->to - ip);
	char *mvto = ip + ilen;
	memmove(mvto, ip, amdat->to - ip);
	memcpy(ip, swdeplist, ilen);
	amdat->to += ilen;
	writefile("Makefile.am", amdat->fro, amdat->to, "w");
	free(amdat->fro);
	free(amdat);
} //updmakefile_am()

char
*swdepends(char *optslist)
{/* optslist may have software names in the form of xyz.h+c.
  * Any such names will be expanded to xyz.h and xyz.c
*/
	if (!optslist) return NULL;

	size_t olen = strlen(optslist);
	if(!olen) return NULL;
	size_t blen = 2*olen;
	char *buf = xmalloc(blen);	// enough for every item xyz.h+c
	buf[0] = 0;
	char name[NAME_MAX];
	char *wp, *we, *oe;
	oe = optslist + olen;
	wp = optslist;
	while (wp < oe) {
		while (*wp == ' ' || *wp == 0) wp++;
		we = wp;
		while (*we != ' ' && we < oe) we++;
		*we = 0;
		strcpy(name, wp);
		char *splitp = strchr(name, '+');
		if (splitp) {
			*splitp = 0;
			strjoin(buf, ' ', name, blen);
			*(splitp-1) = *(splitp + 1);
			*(splitp + 1) = 0;
			strjoin(buf, ' ', name, blen);
		} else strjoin(buf, ' ', name, blen);
		wp = we;
	}
	free(optslist);	// this was strdup()'d
	char *ret = xstrdup(buf);
	return ret;
} // swdepends()

char
*cfgpath(char *cfgdir, char *cfgfname, char *cfgid)
{ /* Gets the parameter identified by cfgid from the config file
   * identified by cfgfname located in the $HOME/.config subdir
   * identified by cfgdir.
*/
	mdata *md = getconfigfile(cfgdir, cfgfname);
	char *cp = getcfgdata(md, cfgid);
	char *ret = xstrdup(cp);
	freemdata(md);
	return ret;
} // cfgpath()

char
*makefullpath(char *left, char *right)
{/* Join them with '/' between and strdup() the result. */
	char joinbuf[PATH_MAX];
	const int max = PATH_MAX;
	strcpy(joinbuf, getenv("HOME"));
	strjoin(joinbuf, '/', left, max);
	strjoin(joinbuf, '/', right, max);
	return xstrdup(joinbuf);
} // makefullpath()

void
linkorcopy(const char *stubdir, const char *compdir, char *swdeplist)
{ /* Checks the items in swdeplist to see if they exist in stubdir or
   * compdir. Any files that exist in stubdir will be copied into the
   * current dir, those that exist in compdir will be hard linked into
   * the current dir. Any filenames that exist in neither place will be
   * warned about (stderr, non fatal).
  */
	if (!swdeplist) return;
	char **depwords = list2array(swdeplist, ' ');
	size_t index = 0;
	size_t max = PATH_MAX;
	while (depwords[index]) {
		char joinbuf[PATH_MAX];
		strcpy(joinbuf, stubdir);
		strjoin(joinbuf, '/', depwords[index], max);
		if (exists_file(joinbuf)) { // I should be in the target dir
			copyfile(joinbuf, depwords[index]);
		} else { // not in stubdir
			strcpy(joinbuf, compdir);
			strjoin(joinbuf, '/', depwords[index], max);
			if (exists_file(joinbuf)) { // I should be in the target dir
				dolink(joinbuf, depwords[index]);
			} else { // not in compdir either
				fprintf(stderr, "Software file unknown: %s\n",
							depwords[index]);
			}
		}
		index++;
	} // while()
} // linkorcopy()

void
extramakefile_am(char *extraslist)
{/* writes the extras list into the proper place in Makefile.am */
	size_t xlen = strlen(extraslist);
	mdata *md = readfile("Makefile.am", 1, 2 * xlen);	// to insert 2x
	char *ip = memmem(md->fro, md->to - md->fro, "_DATA=",
						strlen("_DATA="));
	if (!ip) {
		fputs("Corrupted Makefile.am, no _DATA=\n", stderr);
		exit(EXIT_FAILURE);
	}
	ip = memchr(ip, '\n', md->to - ip);
	char *moveto = ip + xlen;
	memmove(moveto, ip, md->to - ip);
	memcpy(ip, extraslist, xlen);
	md->to += xlen;

	ip = memmem(md->fro, md->to - md->fro, "EXTRA_DIST=",
						strlen("EXTRA_DIST="));
	if (!ip) {
		fputs("Corrupted Makefile.am, no EXTRA_DIST=\n", stderr);
		exit(EXIT_FAILURE);
	}
	ip = memchr(ip, '\n', md->to - ip);
	moveto = ip + xlen;
	memmove(moveto, ip, md->to - ip);
	memcpy(ip, extraslist, xlen);
	md->to += xlen;
	writefile("Makefile.am", md->fro, md->to, "w");
} // extramakefile_am()
