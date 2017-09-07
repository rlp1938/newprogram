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

	optstring = ":hd:ox:";	// initialise

	/* declare and set defaults for local variables. */

	/* set up defaults for opt vars. */
	options_t opts = {0};	// assumes defaults all 0/NULL
	// initialise non-zero defaults below

	int c;
	char joinbuffer[PATH_MAX];	// collects list of extra software.
	joinbuffer[0] = 0;
	const int max = PATH_MAX;
	char databuffer[PATH_MAX];	// collects list of other data.
	databuffer[0] = 0;

	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
		{"help",		0,	0,	'h' },
		{"depends",		1,	0,	'd' },
		{"extra-dist",	1,	0,	'x' },
		{"with-options",0,	0,	'o' },
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
		case 'o':	// software dependencies gopt.[c|h]for Makefile.am
		strjoin(joinbuffer, ' ',"gopt.h", max);
		strjoin(joinbuffer, ' ',"gopt.c", max);
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
	opts.software_dependencies = xstrdup(joinbuffer);
	if (strlen(databuffer)) {
		opts.extra_data = xstrdup(databuffer);
	}
	return opts;
} // process_options()

void dohelp(int forced)
{
  if(strlen(synopsis)) fputs(synopsis, stderr);
  fputs(helptext, stderr);
  exit(forced);
} // dohelp()

char *thesynopsis(void)
{	/* Does nothing except get this text off the top of the page */
	char *ret =
  "\tSYNOPSIS\n"
  "\tmakenewprogram prname\n"
  "\tSets up a new C program according to prname.\n"
  "\tIf the name given is eg someProg, it will create a dir, \n"
  "\tSomeprog under the user defined standard programs directory\n"
  "\tand set up a Makefile.am within that directory. The names placed\n"
  "\tin Makefile.am will be someprog for the bin name, someprog.c for\n"
  "\tsource code and someprog.1 for the man page. The required three\n"
  "\tletter names used by autotools will be 'som' in this example.\n"
  "\tThe cases shown above will be always used regardless off the\n"
  "\tcase set in the user input.\n\n"
  ;
	return ret;
} // thesynopsis()

char *thehelp(void)
{	/* Does nothing except get this text off the top of the page */
	char *ret =
  "\tOPTIONS\n"
  "\t-h, --help\n"
  "\tOutputs this help message and then quits.\n\n"
  "\t--depends, -d software_name\n"
  "\tSpecify additional software dependencies to be recorded in\n"
  "\tMakefile.am. Names that have both a header file and C program\n"
  "\tfile may be input as 'name.h+c'.\n"
  "\tYou may use the option as -d \"name1.h+c name2.c+h ...\" or use\n"
  "\tthe option multiple times on different names.\n\n"
  "\t--with-options, -o\n"
  "\tThe files gopt.c and gopt.h will automatically be included in\n"
  "\tthe sofware dependencies list. Also, the stubs having that name\n"
  "\twill be copied into the new program dir from the specified stub\n"
  "\tlibrary.\n\n"
  "\t--extra-dist, -x data_file or 'file1 file2 ...'\n"
  "\tFile(s) to be installed as data such as config files.\n"
  "\t--extra-dist may be invoked more than once if needed or the list\n"
  "\tof files may be quote protected for a single invocation.\n\n"
  ;
	return ret;
} // thehelp()
