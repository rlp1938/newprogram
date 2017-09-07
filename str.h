/*    str.h
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
#ifndef _STR_H
#define _STR_H
#define _GNU_SOURCE 1
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

typedef struct mdata {
	char *fro;
	char *to;
	char *limit;
} mdata;

void
*xmalloc(size_t);

void
meminsert(const char *, mdata *, size_t);

void
memreplace(mdata *, char *, char *, off_t);

void
memresize(mdata *, off_t);

size_t
memlinestostr(mdata *);

size_t
memstrtolines(mdata *);

void
strjoin(char *, char, char *, size_t);

char
*xstrdup(char *);

char
*getcfgdata(mdata *, char *);

void
freemdata(mdata *);

void
vfree(void *, ...);

char
**list2array(char *, char);

void
trimspace(char *);

void
destroystrarray(char **);

#endif
