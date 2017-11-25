/*    files.c
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

/* The purpose of files.[h|c] is to provide utility functions for
 * manipulating file system objects other than directories. For the
 * latter see dirs.[h|c].
 * */

#include "files.h"

int
xsystem(const char *cmd, int fatal)
{ /* Runs system() and processes the results.
   * If fatal is non zero all non zero results from the child will be
   * fatal but there can be circumstances where the result is needed by
   * the caller.
*/
	const int status = system(cmd);
	if (status == -1) {	// this always fatal
		fprintf(stderr, "system failed to execute: %s\n", cmd);
		exit(EXIT_FAILURE);
	}
	int res;
	if (WIFEXITED(status)) {	// Child has terminated
		res = WEXITSTATUS(status);
		if (fatal && res) {
			fprintf(stderr, "Command \"%s\" returned non-zero result:"
						" %d\n" ,cmd, res);
			exit(EXIT_FAILURE);
		}
	}
	return res;
} // xsystem()

void
dumpstrblock(const char *tmpfn, mdata *md)
{ /* Dumps the block of C strings named by md to the file named by
   * tmpfn. The block will be altered to a block of lines terminated by
   * '\n', then written to tmpfn and restored to C strings before
   * returning. Tmpfn may be "-" to write to stdout.
*/
	memstrtolines(md);
	writefile(tmpfn, md->fro, md->to, "w");
	memlinestostr(md);
} // dumpstrblock()

ino_t
getinode(char *path)
{	/* return the inode number if the path exists, if not abort */
	struct stat sb;
	if (lstat(path, &sb) == -1) {
		perror(path);
		exit(EXIT_FAILURE);
	}
	return sb.st_ino;
} // getinode()

void
touch(const char *fn)
{/* Emulates the simplest use of the shell touch command. */
	FILE *fp = dofopen(fn, "a");	// avoid zeroing an existing file.
	dofclose(fp);
} // touch()

void
str2file(const char *fn, const char *s)
{/* Writes the C string s to file fn. The terminating '\0' of s will be
  * replaced by '\n'.
*/
	size_t len = strlen(s);
	char *buf = xmalloc(len + 1);
	strcpy(buf, s);
	buf[len] = '\n';
	writefile(fn, buf, buf+len+1, "w");
	free(buf);
} // str2file()

FILE
*dofopen(const char *fn, const char *fmode)
{	/* fopen() with error handling. */
	FILE *fpx = fopen(fn, fmode);
	if (!fpx) {
		perror(fn);
		exit(EXIT_FAILURE);
	}
	return fpx;
} // untitled()

void
dofclose(FILE *fp)
{	// fclose() and aborts on error.
	int res = fclose(fp);
	if (res == EOF) {
		perror("fclose()");
		exit(EXIT_FAILURE);
	}
} // dofclose()

void
writefile(const char *filename, char *fro, char *to, const char *fmode)
{	/* write a block of data starting at fro, up to 'to', using mode
	*  fmode. Do nothing if to - fro <= 0.
	*/
	off_t len = to - fro;
	if (len <= 0) return;
	FILE *fpo;
	int closeit = 1;
	if (strcmp("-", filename) == 0) {
		fpo = stdout;
		closeit = 0;
	}
	else fpo = dofopen(filename, fmode);
	size_t written = fwrite(fro, 1, (size_t)len, fpo);
	if (written != (size_t)len) {
		perror("writefile");
		fprintf(stderr,
				"Expected to write: %ld bytes, but wrote %lu bytes.\n",
				len, written);
		exit(EXIT_FAILURE);
	}
	if (closeit) dofclose(fpo);
} // writefile()

void
strblocktolines(char *fro, char *to)
{	/* takes a block of contiguous C strings and formats them to
	 * printable lines, ie replace '\0' with '\n'.
	*/
	char *cp = fro;
	while (cp < to) {
		if (*cp == 0) *cp = '\n';
		cp++;
	}
} // strblocktolines()


mdata
*readfile(const char *path, int fatal, size_t extra)
{	/* If fatal is 0 will not terminate with error if path does not
	 * exist, but will return NULL instead. All other errors are always
	 * fatal. If extra is non-zero will provide extra space, init to 0.
	*/
	mdata  *ret = NULL;
	if (exists_file(path)) {
		ret = malloc(sizeof(mdata ));
		if (!ret) {
			fputs("Out of memory.\n", stderr);
			exit(EXIT_FAILURE);
		}
		size_t fsize = getfsize(path);
		size_t blocksize = fsize + extra;
		ret->fro = malloc(blocksize);
		memset(ret->fro, 0, blocksize);
		FILE *fp = dofopen(path, "r");
		size_t bread = fread(ret->fro, 1, fsize, fp);
		dofclose(fp);
		if (bread != fsize) {
			fprintf(stderr,
			"Expected to get %lu bytes, but got %lu bytes.\n",
			fsize, bread);
			exit(EXIT_FAILURE);
		}
		ret->to = ret->fro + fsize;
		ret->limit = ret->fro + blocksize;
	} else if (fatal) {
		perror(path);
		exit(EXIT_FAILURE);
	}
	return ret;
} //readfile()

int
exists_file(const char *path)
{	/* returns 1 if I can stat the file, 0 otherwise */
	struct stat sb;
	int res = stat(path, &sb);
	if (res == -1) {
		res = 0;
	} else {
		res = 1;
	}
	if (res && S_ISREG(sb.st_mode)) {
		res = 1;
	} else {
		res = 0;
	}
	return res;
} // exists_file()

off_t
getfsize(const char *path)
{	/* returns file size if path exists, fatal otherwise */
	struct stat sb;
	int res = stat(path, &sb);
	if (res == -1) {
		perror(path);
		exit(EXIT_FAILURE);
	}
	return sb.st_size;
} // getfsize()

mdata
*getconfigfile(char *pname, char *cfgfile)
{/* Read the content of $HOME/.config/pname/cfgfile */
	char path[PATH_MAX];
	sprintf(path, "%s/.config/%s/%s", getenv("HOME"), pname, cfgfile);
	mdata *cfd = readfile(path, 1, 0);
	return cfd;
} // getconfigfile()

void
copyfile(const char *pathfro, const char *pathto)
{/* Does a file copy in user space. */
	mdata *fd = readfile(pathfro, 1, 0);
	writefile(pathto, fd->fro, fd->to, "w");
} // copyfile()

void
dolink(const char *fr, const char *to)
{/* link() with error handling. */
	if (link(fr, to) == -1) {
		perror(to);
		perror(fr);	// don't know what caused the snafu
		exit(EXIT_FAILURE);
	}
} // dolink()

char
*cfg_getparameter(const char *prn, const char *fn, const char *param)
{ /* Return the string that param points to. */
	char *pp = cfg_pathtofile(prn, fn);
	mdata *md = readfile(pp, 1, 0);
	free(pp);
	memlinestostr(md);
	char *p = memmem(md->fro, md->to - md->fro, param, strlen(param));
	if (!p) {
		fprintf(stderr, "No such parameter: %s\n", param);
		free_mdata(md);
		exit(EXIT_FAILURE);
	}
	char *eq = strchr(p, '=');
	if (!eq) {
		fprintf(stderr, "Malformed parameter line: %s\n", p);
		free_mdata(md);
		exit(EXIT_FAILURE);
	}
	eq++;	// get past '='
	trimspace(eq);
	char *ret = xstrdup(eq);
	free_mdata(md);
	return ret;
} // cfg_getparameter()
