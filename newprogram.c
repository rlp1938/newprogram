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
	char *dir;		// directory name.
	char *exe;		// program name.
	char *src;		// source code name.
	char *thr;		// three letter abbreviation.
	char *man;		// man page name.
	char *author;	// program author name.
	char *email;	// author email address,
} progid;

typedef struct oplist_t {	/* var to use when generating options */
	char	*shoptname;		// short options name.
	char	*longoptname;	// long options name.
	char	*dataname;		// name of the opts variable.
	int		optarg;			// 0,1 or 2.
} oplist_t;

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
static void gensrcfiles(progid *, char *, int);
static void genoptions(progid *, char *);
static oplist_t **words2ol(char *words);
static void updatemainfile(const char *, oplist_t **);
static void updategoptHfile(const char *, oplist_t **);
static void updategoptCfile(const char * ,oplist_t **);
static void sotarget(oplist_t *);
static void lotarget(oplist_t *);
static void optarget(oplist_t *);
static void hltarget(oplist_t *);
static void sytarget(void);
static void addautotools(progid *);

char *gprname;	// set early in the piece and used near the end.

int main(int argc, char **argv)
{	/* newprogram - write the initial files for a new C program. */
	if (!checkfirstrun("newprogram", "am.mak", "prdata.cfg",
					"goptC", "goptH", "mainC", "manpage.md", NULL)) {
		firstrun("newprogram", "am.mak", "prdata.cfg",
						"goptC", "goptH", "mainC", "manpage.md", NULL);
		fprintf(stderr,
					"Please edit prdata.cfg in %s.config/newprogram"
					" to meet your needs.\n",
					getenv("HOME"));
		exit(EXIT_SUCCESS);
	}
	options_t opt = process_options(argc, argv);
	if (!argv[optind]) {
		fputs("No project name provided.\n", stderr);
		exit(EXIT_FAILURE);
	}
	progid *pi = makeprogname(argv[optind]);
	printf("%s %s %s %s %s\n",pi->dir, pi->exe, pi->src, pi->man,
			pi->thr);
	gprname = argv[optind];	// used toward the end of the program.
	// get location of boilerplate code (if any), and source library
	char *prog = cfgpath("newprogram", "prdata.cfg", "progdir");
	char *stub = cfgpath("newprogram", "prdata.cfg", "stubdir");
	char *comp = cfgpath("newprogram", "prdata.cfg", "compdir");
	char *tmp = pi->dir;	// preserve it to free() it.
	pi->dir = makefullpath(prog, pi->dir);
	free(tmp);
	char *compdir = makefullpath(prog, comp);
	char *stubdir = makefullpath(prog, stub);
	vfree(comp, stub, prog, NULL);
	printf("%s\n%s\n%s\n", pi->dir, compdir, stubdir);
	// create the Makefile.am for the new program
	newdir(pi->dir, 1);
	xchdir(pi->dir);
	writemakefile_am(pi, "am.mak");	// from $HOME/.config/...
	char *extras = swdepends(opt.software_deps);
	updmakefile_am(pi->exe, extras);
	// copy in boilerplate and link library source
	linkorcopy(stubdir, compdir, extras);
	if (opt.hasopts) {	// add gopt.h and gopt.c to Makefile.am
		updmakefile_am(pi->exe, " gopt.c gopt.h");
	}
	// add extra-dist to Makefile.am if optioned.
	if (opt.extra_data) {
		extramakefile_am(opt.extra_data);
		free(opt.extra_data);
	}
	// generate C program regardless
	gensrcfiles(pi, opt.options_list, opt.hasopts);
	// Generate the autotools
	addautotools(pi);
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
	// to be filled in later
	prid->author = NULL;
	prid->email = NULL;

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
	vfree(pi->exe, pi->src, pi->man, pi->thr, pi->dir, pi->author,
	pi->email, pi, NULL);
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

void
gensrcfiles(progid *pi, char *oplist, int hasopts)
{/* Generate the C program pi->src and if hasopts has been set generate
  * the gopt.c and gopt.h files. If oplist is NULL quit, else
  * call genoptions().
*/
	char path[PATH_MAX];
	const int max = PATH_MAX;
	path[0] = 0;
	sprintf(path, "%s/.config/newprogram/", getenv("HOME"));
	char *tocfg = NULL;
	tocfg = xstrdup(path);	// need this for gopt maybe.
	strjoin(path, '/', "mainC", max);
	copyfile(path, pi->src);
	if (!hasopts) goto quit;
	// else make gopt.[h|c]
	strcpy(path, tocfg);
	strjoin(path, '/', "goptC", max);
	copyfile(path, "gopt.c");
	strcpy(path, tocfg);
	strjoin(path, '/', "goptH", max);
	copyfile(path, "gopt.h");
	if (oplist) {
		genoptions(pi, oplist);
	}
	/* If the gopt source files got made they will contain many comments
	 * with the word 'target' in them. Likely useful if writing in
	 * options processing by hand. If genoptions() is run those comments
	 * will have been obliterated.
	*/
quit:
	if (tocfg) free(tocfg);
} // gensrcfiles()

void
genoptions(progid *pi, char *optionslst)
{	/* Using comments with the word 'target' in them, insert options
	 * processing code into the files, gopt.[c|h] and pi->src.
	*/
	oplist_t **ol = words2ol(optionslst);
	// Deal with 3 files that have been already saved in current dir.
	updatemainfile(pi->src, ol);
	updategoptHfile("gopt.h", ol);
	updategoptCfile("gopt.c", ol);
} // genoptions()

oplist_t **words2ol(char *listofopts)
{/* From a list of words (coded as options data) generate a list of
  * oplist_t structs.
*/
	char **wordlist = list2array(listofopts, ' ');
	size_t n = 0;
	while (wordlist[n]) n++;
	oplist_t **ol = xmalloc((n+1) * sizeof(oplist_t *));
	memset(ol, 0, (n+1) * sizeof(oplist_t *));
	size_t index = 0;
	while (wordlist[index]) {
		char *cp;
		oplist_t *tmp = xmalloc(sizeof(oplist_t));
		char *w = wordlist[index];
		if ((cp = strstr(w, "::"))) {
			*cp = 0;
			tmp->optarg = 2;
		} else if ((cp = strchr(w, ':'))) {
			*cp = 0;
			tmp->optarg = 1;
		} else {
			tmp->optarg = 0;
		}
		tmp->longoptname = xstrdup(w+1);
		char buf[16] = {0};
		buf[0] = w[0];
		strncpy(buf + 1, "::", tmp->optarg);
		tmp->shoptname = xstrdup(buf);
		sprintf(buf, "opts.o_%c", tmp->shoptname[0]);
		tmp->dataname = xstrdup(buf);
		ol[index] = tmp;
		index++;
	} // while()
	destroystrarray(wordlist);	// no longer needed;
	return ol;
} // words2ol()

void
updatemainfile(const char *fn, oplist_t **ol)
{/* The job here is to generate a set of print statements to show
  * what options have been chosen. And it creates a place holder to go
  * to write the code really required when choosing those options.
 */
	mdata *md = readfile(fn, 1, 1024);
	char joinbuf[PATH_MAX];
	joinbuf[0] = 0;
	size_t idx;
	for (idx = 0; ol[idx]; idx++) {
		char line[NAME_MAX];
		oplist_t *tmp = ol[idx];
		char *fmt;
		fmt = (tmp->optarg == 0) ? "%d" : "%s";
		sprintf(line, "\tif (%s) printf(\"%s%c%c\", %s);\t",
				tmp->dataname, fmt, '\\', 'n', tmp->dataname);
		strjoin(joinbuf, 0, line, PATH_MAX);
		sprintf(line, "// -%c, --%s\n", tmp->shoptname[0],
					tmp->longoptname);
		strjoin(joinbuf, 0, line, PATH_MAX);
	}
	memreplace(md, "/* dummy opts target */", joinbuf, 1024);
	writefile(fn, md->fro, md->to, "w");
	free(md->fro);
} // updatemainfile()

void
updategoptHfile(const char *fn, oplist_t **ol)
{/* define the variables used in the options_t struct. */
	mdata *md = readfile(fn, 1, 128);
	char joinbuf[PATH_MAX];
	memset(joinbuf, 0, PATH_MAX);	// easier to read in gdb.
	size_t idx;
	for (idx = 0; ol[idx]; idx++) {
		char line[NAME_MAX];
		memset(line, 0, NAME_MAX);
		oplist_t *tmp = ol[idx];
		char *vartype = (tmp->optarg == 0) ? "int\t " : "char\t*";
		sprintf(line, "\t%so_%c;\t// -%c, --%s\n", vartype,
				tmp->shoptname[0], tmp->shoptname[0], tmp->longoptname);
		strjoin(joinbuf, 0, line, PATH_MAX);
	}
	memreplace(md, "/* header target */", joinbuf, 256);
	writefile(fn, md->fro, md->to, "w");
	free(md->fro);
} // updategoptHfile()

char bufso[NAME_MAX];
char buflo[PATH_MAX];
char bufop[PATH_MAX];
char bufhl[PATH_MAX];
char bufsy[PATH_MAX];

void
updategoptCfile(const char *fn, oplist_t **ol)
{
	mdata *md = readfile(fn, 1, 1024);
	char joinbuf[PATH_MAX];
	memset(joinbuf, 0, PATH_MAX);	// easier to read in gdb.
	size_t idx;
	for (idx = 0; ol[idx]; idx++) {
		char line[NAME_MAX];
		memset(line, 0, NAME_MAX);
		oplist_t *tmp = ol[idx];
		sotarget(tmp);
		lotarget(tmp);
		optarget(tmp);
		hltarget(tmp);
	} // for()
	sytarget();	// uses the global gprname (name of generated program)
	memreplace(md, "/* short options target */", bufso, 1024);
	memreplace(md, "/* long options target */\n", buflo, 1024);
	memreplace(md, "/* option proc target */\n", bufop, 1024);
	memreplace(md, "/* help target */\n", bufhl, 1024);
	memreplace(md, "/* syn target */\n", bufsy, 1024);
	writefile(fn, md->fro, md->to, "w");
	free(md->fro);
} // updategoptCfile()

void
sotarget(oplist_t *os)
{ /* Short option */
	strcat(bufso, os->shoptname);
} // sotarget()

void
lotarget(oplist_t *os)
{ /* Long option */
	char line[NAME_MAX];
	sprintf(line, "\t\t{\"%s\",\t%d,\t0,\t'%c'},\n",
			os->longoptname, os->optarg, os->shoptname[0]);
	strjoin(buflo, 0, line, PATH_MAX);
} // lotarget()

void
optarget(oplist_t *os)
{ /* Option processing */
	char *vfmt[] = { "%s = 1", "%s = xstrdup(optarg)",
					"if (optarg) %s = xstrdup(optarg)" };
	char action[64];
	sprintf(action, vfmt[os->optarg], os->dataname);
	char line[NAME_MAX];
	sprintf(line, "\t\tcase '%c':\n\t\t\t%s;\n\t\t\tbreak;\n",
				os->shoptname[0], action);
	strjoin(bufop, 0, line, PATH_MAX);
} // optarget()

void
hltarget(oplist_t *os)
{ /* help processing */
	char *vchar[] = { "", "options_argument",
						"(optional) options_argument"};
	char line[NAME_MAX];
	sprintf(line, "  \"\\t-%c, --%s %s\\n\"\n", os->shoptname[0],
				os->longoptname, vchar[os->optarg]);
	strjoin(bufhl, 0, line, PATH_MAX);
	char *vfmt[] = {"Sets %s to 1, the default is 0.\\n\\n",
					"Copies optarg to %s, default is NULL.\\n\\n",
					"If optarg is provided, copies it to %s,"
					" default value is NULL.\\n\\n"};
	char usefmt[80];
	strcpy(usefmt, "  \"\\t");
	strcat(usefmt, vfmt[os->optarg]);
	strcat(usefmt, "\"\n");
	sprintf(line, usefmt, os->dataname);
	strjoin(bufhl, 0, line, PATH_MAX);
} // hltarget()

void
sytarget(void)
{ /* synopsis and description */
	char line[NAME_MAX];
	sprintf(line, "  \"\\t\\t%s [option] program_name\\n\\n\"\n",
				gprname);
	strjoin(bufsy, 0, line, PATH_MAX);
	strcpy(line, "  \"\\tDESCRIPTION\\n\"\n"
	"  \"\\tMake necessary explanation of the"
	" purpose and features of the program.\"\n"
		);
	strjoin(bufsy, 0, line, PATH_MAX);
} // sytarget()

void
addautotools(progid *pi)
{/* adds files needed by autotools, runs programs and amends files
  * as required. NB `automake --add-missing --copy` no longer makes
  * copies of some files required by a GNU standard build so I create
  * them here.
  */
	// Create files required.
	char joinbuf[NAME_MAX];
	char *author = cfgpath("newprogram", "prdata.cfg", "author");
	char *email = cfgpath("newprogram", "prdata.cfg", "email");
	sprintf(joinbuf, "README for %s", pi->exe);
	str2file("README", joinbuf);
	sprintf(joinbuf, "NOTES for %s", pi->exe);
	str2file("NOTES", joinbuf);
	sprintf(joinbuf, "ChangeLog for %s", pi->exe);
	str2file("ChangeLog", joinbuf);
	sprintf(joinbuf, "NEWS for %s", pi->exe);
	str2file("NEWS", joinbuf);
	sprintf(joinbuf, "Author for %s", pi->exe);
	strjoin(joinbuf, '\n', author, NAME_MAX);
	strjoin(joinbuf, ' ', email, NAME_MAX);
	str2file("AUTHORS", joinbuf);
	// run the autotools stuff
	xsystem("autoscan", 1);
	mdata *cfd = readfile("configure.scan", 1, 128);
	memreplace(cfd, "FULL-PACKAGE-NAME", pi->exe, 128);
	memreplace(cfd, "VERSION", "1.0", 128);	// Hard wired? OK I think.
	memreplace(cfd, "BUG-REPORT-ADDRESS", email, 128);
	// Lazy way to stop infinite loop.
	memreplace(cfd, "AC_CONFIG_SRCDIR",
						"AM_INIT_AUTOMAKE\nac_config_srcdir", 128);
	// Original value of search target restored.
	memreplace(cfd, "ac_config_srcdir", "AC_CONFIG_SRCDIR", 128);
	writefile("configure.ac", cfd->fro, cfd->to, "w");
	xsystem("autoheader", 1);
	xsystem("aclocal", 1);
	xsystem("automake --add-missing --copy", 1);
	xsystem("autoconf", 1);
	touch(pi->man);
	vfree(email, author, NULL);
} // addautotools()
