/*    str.c
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

/* The purpose of str.[h|c] is to provide some utility functions for
 * manipulating memory. Consequently many of the functions will simply
 * be wrappers around the C library functions str*() and mem*().
 * */

#include "str.h"
char
*getcfgfn(const char *prg, const char *fn)
{/* assemble a config file name from the given parameters */
	char buf[PATH_MAX];
	sprintf(buf, "%s/.config/%s/%s", getenv("HOME"), prg, fn);
	return xstrdup(buf);
} // getcfgfn()

char
*get_home(void)
{
	char buf[PATH_MAX];
	sprintf(buf, "%s", getenv("HOME"));
	return xstrdup(buf);
} // get_home()

size_t
home_len(void)
{
	char buf[PATH_MAX] = {0};
	sprintf(buf, "%s", getenv("HOME"));
	return strlen(buf);
} // home_len()

char
**memblocktoarray(mdata *md, int islines)
{/* Memory data, C strings or lines with '\n'.
  * Make a char * array of them.
  * To be NULL terminated.
*/
	size_t num;
	if (islines) num = memlinestostr(md);	// '\n' terminated.
	else num = countmemstr(md);				// '\0' terminated.
	size_t rqd = (num + 1) * sizeof(char *);
	char **result = xmalloc(rqd);
	memset(result, 0, rqd);
	char *cp = md->fro;
	size_t i = 0;
	while (cp < md->to) {
		result[i++] = xstrdup(cp);
		cp += strlen(cp) + 1;
	}
	return result;
} // memblocktoarray()

size_t
lenrequired(size_t nominal_len)
{ /* Ensure that memory operations always have 8 bytes to spare. */
	const size_t fudgefence = 8;
	return nominal_len + fudgefence;
} // lenrequired()

size_t
countmemstr(mdata *md)
{ /* In memory block specified by md, count the number of C strings. */
	char *cp = md->fro;
	size_t c = 0;
	while (cp < md->to) {
		if (*cp == 0) c++;
		cp++;
	}
	return c;
} // countmemlines()

char
*mktmpfn(char *prname, char *extrafn, char *thename)
{/* Make a temporary file name.
  * The name will be the concatenation of: "/tmp/", prname, $USER, pid
  * and extrafn. Extrafn may be NULL if the application needs only one
  * filename. If thename is NULL a buffer of PATH_MAX will be created
  * and the returned result must be freed after use, otherwise thename
  * should be a buffer of PATH_MAX size allocated by the caller.
*/
	char *buf;
	if (thename) {
		buf = thename;
	} else {
		buf = xmalloc(PATH_MAX);
	}
	sprintf(buf, "/tmp/%s%s%d%s.lst", prname, getenv("USER"), getpid(),
				extrafn);
	return buf;
} // mktmpfn()

mdata
*init_mdata(void)
{
	mdata *md = xmalloc(sizeof(mdata));
	md->fro = md->to = md->limit = (char *)NULL;
	return md;
} // init_mdata()

void
meminsert(const char *line, mdata *dd, size_t meminc)
{	/* insert line into the data block described by dd, taking care of
	 * necessary memory reallocation as needed.
	*/
	size_t len = strlen(line);
	size_t safelen = lenrequired(len);
	if (safelen > (unsigned)(dd->limit - dd->to)) { // >= 0 always
		/* Ensure that line always has room to fit. */
		size_t needed = (meminc > safelen) ? meminc : safelen;
		memresize(dd, needed);
	}
	strcpy(dd->to, line);
	dd->to += len+1;
} // meminsert()

/* There's a bug in memreplace(). It manifests as dofopen() segfaulting
 * when run immediately after a run of memreplace(). dofopen() should
 * never segfault, it can and should abort when there is some problem
 * with the file it's trying to open. So I am clobbering some memory, I
 * suppose with some kind of write beyond my malloc'd space.
 *
 * The problem happens when available space is == space needed or
 * available space is 1 more than space needed.
 * If available space is 1 less than needed, a resize is forced and all
 * is well. If available space is 2 more than needed it works as well.
 *
 * If I am off by 1 somewhere I can't see where. Is memmove() doing
 * something it shouldn't oughta?
 *
 * Workaround - I'll put an 8 byte fudge fence into the thing.
 * */

int
printstrlist(char **list)
{
	int i;
	for (i = 0; list[i]; i++) {
		printf("%s\n", list[i]);
	}
	return i;
} // printstrlist()

void
memreplace(mdata *md, char *find, char *repl, off_t meminc)
{/* Replace find with repl for every occurrence in the data block md. */
	size_t flen = strlen(find);
	size_t rlen = strlen(repl);
	off_t ldiff = rlen - flen;
	const off_t fudge_fence = 8;	// see bug commentary above.
	char *fp = md->fro;
	fp = memmem(fp, md->to - fp, find, flen);
	while (fp) {
		off_t avail = md->limit - md->to;
		if (ldiff + fudge_fence > avail) {
			off_t fpoffset = fp - md->fro;	// Handle relocation.
			// Ensure that the resized memory has space to fit new data.
			off_t actualinc = (meminc > ldiff)
								? meminc : ldiff + fudge_fence;
			/* Ensure that we never have available space within 0 or 1
			 * bytes of the needed space. */
			memresize(md, actualinc);
			fp = md->fro + fpoffset;
		}
		char *movefro = fp + flen;
		char *moveto = fp + rlen;
		size_t mlen = md->to - fp;
		memmove(moveto, movefro, mlen);
		memcpy(fp, repl, rlen);
		md->to += ldiff;
		fp = memmem(fp, md->to - fp, find, flen);
	} // while(fp)
} // memreplace()

void
memresize(mdata *dd, off_t change)
{	/* Alter the the size of a malloc'd memory block.
	 * Takes care of any relocation of the original pointer.
	 * Writes 0 over the new space when increasing size.
	 * Handles memory allocation failure.
	*/
	size_t now = dd->limit - dd->fro;
	size_t dlen = dd->to - dd->fro;
	size_t newsize = now + change;	// change can be negative
	dd->fro = realloc(dd->fro, newsize);
	if (!dd->fro) {
		fputs("Out of memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	dd->limit = dd->fro + newsize;
	dd->to = dd->fro + dlen;
	if (dd->limit > dd->to) {	// will be the case after an increase.
		memset(dd->to, 0, dd->limit - dd->to);
	}
} // memresize()

size_t
memlinestostr(mdata *md)
{ /* In the block of memory enumerated by md, replace all '\n' with
   * '\0' and return the number of replacements done.
  */
	size_t lcount = 0;
	char *cp = md->fro;
	while (cp < md->to) {
		if (*cp == '\n') {
			*cp = 0;
			lcount++;
		}
		cp++;
	}
	return lcount;
} // memlinestostr()

size_t
memstrtolines(mdata *md)
{ /* In the block of memory enumerated by md, replace all '\0' with
   * '\n' and return the number of replacements done.
  */
	size_t scount = 0;
	char *cp = md->fro;
	while (cp < md->to) {
		if (*cp == 0) {
			*cp = '\n';
			scount++;
		}
		cp++;
	}
	return scount;
} // memstrtolines()

void
strjoin(char *left, char sep, char *right, size_t max)
{/*	Join right onto left, ensuring that sep is between left and right.
	* Left is a buffer of length max bytes. However strlen(left) may be
	* 0 and also sep may be '\0' and if it is this will have the effect
	* of a simple strcat().
	* If the total length of the join would exceed max, quit with
	* an error message.
*/
	size_t llen = strlen(left);
	if (!right) return;
	size_t rlen = strlen(right);
	size_t tlen = llen + rlen + 1;	// ignore sep == 0
	if ( tlen >= max) {
		fprintf(stderr, "String length %lu, to big for buffer %lu\n",
				tlen, max);
		exit(EXIT_FAILURE);
	}
	if (llen == 0 && sep == 0) {
		strcpy(left, right);
	} else if (sep == 0) {
		strcpy(left + llen, right);	// slightly faster than strcat()??
	} else {
		char *joinp = left + llen;
		if (*(joinp - 1) == sep) {	// done by caller
			strcpy(joinp, right);
		} else {
			*joinp = sep;
			strcpy(joinp + 1, right);
		}
	}
} // strjoin()

char
*xstrdup(char *s)
{	/* strdup() with error handling */
	char *p = strdup(s);
	if (!p) {
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	return p;
} // xstrdup()

void
*xmalloc(size_t s)
{	// malloc with error handling
	void *p = malloc(s);
	if (!p) {	// Better forget perror in this circumstance.
		fputs("Out of memory.\n", stderr);
		exit(EXIT_FAILURE);
	}
	return p;
} // xmalloc()

char
*getcfgdata(mdata *cfdat, char *cfgid)
{/* Read selected line */
	static char buf[NAME_MAX];
	buf[0] = 0;
	char *cp = cfdat->fro;
	size_t slen = strlen(cfgid);
	cp = memmem(cp, cfdat->to - cp, cfgid, slen);
	if (!cp) {
		fprintf(stderr, "No such parameter in config: %s\n", cfgid);
		exit(EXIT_FAILURE);
	}
	cp = memchr(cp, '=', cfdat->to - cp);
	if (!cp) {
		fprintf(stderr, "Malformed line in config: %s\n", cfgid);
		exit(EXIT_FAILURE);
	}
	cp++;	// step past '='
	char *ep = memchr(cp, '\n', cfdat->to - cp);
	if (!cp) {
		fprintf(stderr, "WTF??? no line feed in config: %s\n", cfgid);
		exit(EXIT_FAILURE);
	}
	size_t dlen = ep - cp;
	strncpy(buf, cp, dlen);
	buf[dlen] = 0;
	return buf;
} // getgfgdata()

void
free_mdata(mdata *md)
{/* Free the data pointed to by md, then free md itself. */
	free(md->fro);
	free(md);
} // freemdata()

void
vfree(void *p, ...)
{/* free as many as are listed, terminate at NULL */
	va_list ap;
	va_start(ap, p);
	free(p);
	void *vp;
	while (1) {
		vp = va_arg(ap, void *);
		if (!vp) break;	// terminating NULL
		free(vp);
	}
	va_end(ap);
}

char
**list2array(char *items, char sep)
{ /* Operates on a list of items, separated by sep, and returns a NULL
   * terminated array of strings. Deals with lists that begin with sep
   * or not and handles comma separated lists that may also have spaces
   * before and/or after the actual text items.
  */
	size_t lcount = 0;
	size_t ilen = strlen(items);
	if (ilen > PATH_MAX) {
		fputs("Input string too long, quitting!\n", stderr);
		exit(EXIT_FAILURE);
	}
	char list[PATH_MAX];
	strcpy(list, items);
	size_t i;
	for (i = 0; i < ilen; i++) {
		if(list[i] == sep) lcount++;
	}
	if (list[0] != sep) lcount++;	// number of list items
	// make list of char*
	char **result = xmalloc((lcount+1) * sizeof(char *));
	result[lcount] = (char *)NULL;	// terminator
	char *wbegin = list;
	while (*wbegin == sep) wbegin++;
	char *wend = strchr(wbegin, sep);
	char word[PATH_MAX];
	i = 0;
	while (wend) {
		*wend = 0;
		strcpy(word, wbegin);
		trimspace(word);
		result[i] = xstrdup(word);
		i++;
		wbegin = wend + 1;
		wend = strchr(wbegin, sep);
	}
	if (strlen(wbegin)) { // the last item
		strcpy(word, wbegin);
		trimspace(word);
		result[i] = xstrdup(word);
	}
	return result;
} // list2array()

void
trimspace(char *buf)
{/* Lops any isspace(char) off the front and back of buf. */
	size_t buflen = strlen(buf);
	if (buflen > PATH_MAX) {
		fputs("Input string too long, quitting!\n", stderr);
		exit(EXIT_FAILURE);
	}
	char work[PATH_MAX];
	char *begin = buf;
	char *end = begin + buflen;
	while (isspace(*begin) && begin < end) begin++;
	strcpy(work, begin);
	size_t worklen = strlen(work);
	begin = work;
	end = begin + worklen;
	while (isspace(*end) && end > begin) end--;
	strcpy(buf, work);
} // trimws()

void
destroystrarray(char **wordlist, size_t count)
{/* Frees list of strings. If count is non-zero, frees count strings,
  * otherwise it assumes wordlist has a NULL terminator.
*/
	size_t i = 0;
	if (count) {
		for (i = 0; i < count; i++) free(wordlist[count]);
	} else {
		while (wordlist[i]) {
			free(wordlist[i]);
			i++;
		}
	}
	free(wordlist);
} // destroystrarray()

char
*cfg_pathtofile(const char *prn, const char *fn)
{/* return path to a named config file. */
	char buf[PATH_MAX];
	sprintf(buf, "%s/.config/%s/%s", getenv("HOME"), prn, fn);
	return xstrdup(buf);
} // cfg_pathtofile()

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

int
in_uch_array(const unsigned char c, unsigned char *buf)
{ /* Return 1 if c is in buf, 0 otherwise. Buf must be 0 terminated. */
	unsigned char *cp = buf;
	while (*cp != 0) {
		if(*cp == c) return 1;
		cp++;
	}
	return 0;
} // in_uch_array()
