/*      gopt.c
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
#include "str.h"
#include "files.h"
#include "gopt.h"


options_t process_options(int argc, char **argv)
{
	synopsis = thesynopsis();
	helptext = thehelp();

	optstring = ":hd:ox:n:";	// initialise

	/* declare and set defaults for local variables. */

	/* set up defaults for opt vars. */
	options_t opts = {0};	// assumes defaults all 0/NULL
	// initialise non-zero defaults below

	int c;
	const int max = PATH_MAX;
	char joinbuffer[PATH_MAX];	// collects list of extra software.
	joinbuffer[0] = 0;
	char databuffer[PATH_MAX];	// collects list of other data.
	databuffer[0] = 0;
	char optionsbuffer[PATH_MAX];	// collects list of options codes.
	optionsbuffer[0] = 0;

	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
		{"help",		0,	0,	'h' },
		{"depends",		1,	0,	'd' },
		{"extra-dist",	1,	0,	'x' },
		{"with-options",0,	0,	'o' },
		{"options-list",1,	0,	'n' },
		{0,	0,	0,	0 }
		};

		c = getopt_long(argc, argv, optstring,
                        long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0:
			switch (option_index) {
			} // switch()
		break;
		case 'h':
		dohelp(0);
		break;
		case 'd':	// other software dependencies for Makefile.am
		strjoin(joinbuffer, ' ',optarg, max);
		break;
		case 'o':	// just set a flag for main()
		// deal with -n seen before -o, or -o not done.
		opts.hasopts = 1;
		break;
		case 'n':	// code strings for options generation.
		strjoin(optionsbuffer, ' ', optarg, max);
		opts.hasopts = 1;	// generates -o option anyway
		break;
		case 'x':	// other data for Makefile.am
		strjoin(databuffer, ' ',optarg, max);
		break;
		case ':':
			fprintf(stderr, "Option %s requires an argument\n",
					argv[this_option_optind]);
			dohelp(1);
		break;
		case '?':
			fprintf(stderr, "Unknown option: %s\n",
					 argv[this_option_optind]);
			dohelp(1);
		break;
		} // switch()
	} // while()
	opts.software_deps = xstrdup(joinbuffer);
	if (strlen(databuffer)) {
		opts.extra_data = xstrdup(databuffer);
	}
	if (strlen(optionsbuffer)) {
		opts.options_list = xstrdup(optionsbuffer);
	}
	return opts;
} // process_options()

void dohelp(int forced)
{
  char command[PATH_MAX];
  char *dev = "./newprogram.1";
  char *prd = "newprogram";
  if (exists_file(dev)) {
    sprintf(command, "man %s", dev);
  } else {
    sprintf(command, "man 1 %s", prd);
  }
  xsystem(command, 1);
  exit(forced);
} // dohelp()
