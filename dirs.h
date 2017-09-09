/*    dirs.h
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
#ifndef _DIRS_H
#define _DIRS_H
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
#include "str.h"
#include "files.h"

unsigned char fsoselect[9];
int meminc;
char **except;

DIR
*dopendir(const char *);

void
doclosedir(DIR *);

int
recursedir(char *, mdata *);

void
init_recursedir(mdata *, char **, ...);

int
inlist(const char *, char **);

int
inarray(const unsigned char, unsigned char *);

char
*join(const char *, const char *);

void
initmdat(mdata *);

void
initfsoslect(unsigned char[]);

void
newdir(const char *, int);

void
xchdir(const char *);

int
exists_dir(const char *);

#endif
