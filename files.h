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
#ifndef _FILES_H
#define _FILES_H

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
void
touch(const char*);

void
str2file(const char *, const char *);

mdata
*readfile(const char *, int, size_t);

FILE
*dofopen(const char *, const char *);

void
dofclose(FILE *);

void
writefile(const char *, char *, char *, const char *);

void
strblocktolines(char *, char *);

int
exists_file(const char *);

off_t
getfsize(const char *);

mdata
*getconfigfile(char *, char *);

void
copyfile(const char *pathfro, const char *pathto);

void
dolink(const char *, const char *);

void
xsystem(const char *, int);

#endif
