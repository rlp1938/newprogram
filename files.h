/*    files.h
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
#ifndef _FILES_H
#define _FILES_H

#define _GNU_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

#include "str.h"
void
writestrarray(char **list);

char
**getfile_str(const char *path);

time_t
getfile_mtime(const char *path);

int
xsystem(const char *command, int fatal);

void
dumpstrblock(const char *tmpfn, mdata *md);

ino_t
getinode(char *path);

void
touch(const char* fn);

void
str2file(const char *fn, const char *s);

mdata
*readfile(const char *fn, int fatal, size_t extra);

FILE
*dofopen(const char *fn, const char *opnmode);

void
dofclose(FILE *fp);

void
writefile(const char *fn, char *fro, char *to, const char *opnmode);

void
strblocktolines(char *fro, char *to);

int
exists_file(const char *);

off_t
getfsize(const char *fn);

mdata
*getconfigfile(char *progname, char *cfgfn);

void
copyfile(const char *pathfro, const char *pathto);

void
dolink(const char *fro, const char *to);

char
*cfg_getparameter(const char *prn, const char *fn, const char *param);

#endif
