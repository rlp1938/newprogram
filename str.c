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
#include "str.h"

void
meminsert(const char *line, mdata *dd, size_t meminc)
{	/* insert line into the data block described by dd, taking care of
	 * necessary memory reallocation as needed.
	*/
	int len = strlen(line);
	if (len > dd->limit - dd->to) {
		memresize(dd, meminc);
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
freemdata(mdata *md)
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
destroystrarray(char **wordlist)
{/* Frees NULL terminated list of strings. */
	size_t i = 0;
	while (wordlist[i]) {
		free(wordlist[i]);
		i++;
	}
	free(wordlist);
} // destroystrarray()

