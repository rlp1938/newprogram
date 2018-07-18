/*      gopt.h
 *
 *  Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
*/

#ifndef GOPT_H
#define GOPT_H
#include "str.h"
char *optstring;
char *helptext;
char *synopsis;

typedef struct options_t {
	int hasopts;			// main() needs this flag.
	char *software_deps;	// source files to include.
	char *extra_data;		// eg stuff like config files.
	char *options_list;		// code to describe options for new program.
} options_t;

void dohelp(int forced);
options_t process_options(int argc, char **argv);

#endif
