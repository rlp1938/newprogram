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
		case 'o':	// software dependencies gopt.[c|h]for Makefile.am
		// deal with -n seen before -o, or -o not done.
		if (opts.hasopts) break;
		strjoin(joinbuffer, ' ',"gopt.h", max);
		strjoin(joinbuffer, ' ',"gopt.c", max);
		opts.hasopts = 1;
		break;
		case 'n':	// code strings for options generation.
		// deal with -o not done, or done out of order.
		strjoin(optionsbuffer, ' ', optarg, max);
		if (opts.hasopts) break;
		strjoin(joinbuffer, ' ',"gopt.h", max);
		strjoin(joinbuffer, ' ',"gopt.c", max);
		opts.hasopts = 1;
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
  if(strlen(synopsis)) fputs(synopsis, stderr);
  fputs(helptext, stderr);
  exit(forced);
} // dohelp()

char *thesynopsis()
{ /* Only purpose is to get this stuff off the top of the page. */
	char *ret =
  "\tSYNOPSIS\n"
  "\tnewprogram [options] program_name\n"
  "\tSets up a new C program according to program_name.\n"
  "\tIf the name given is eg someProg, it will create a dir, Someprog"
  " under\n\tthe user defined standard programs directory and set up "
  "a Makefile.am\n\twithin that directory. The names placed in "
  "Makefile.am will be\n\t'someprog' for the bin name, 'someprog.c' for"
  " source code and\n\t'someprog.1' for the man page. The required "
  "three letter names used\n\tby autotools will be 'som' in this "
  "example. The cases shown above will\n\tbe always used regardless"
  " off the case set in the user input.\n\n"
  "\tDESCRIPTION\n"
  "\tOn first run some configuration files will be copied into: \n"
  "\t$HOME/.config/newprogram. You will be requested to edit "
  "'paths.cfg' to\n\tprovide the pathname of your programs dir, and "
  " 2 subdirs, one for\n\tboilerplate code to be copied into the "
  "new project dir, and the other\n\tfor your library source code "
  "which will be hard linked into your\n\tproject dir.\n"
  "\tIn the end, once the autotools programs are run, the generated"
  "\n\tprogram should make and be able to be run, choosing the "
  "specified\n\toptions.\n\n"
  ;
  return ret;
}

char *thehelp()
{
	char *ret =
  "\tOPTIONS\n"
  "\t-h, --help\n"
  "\tOutputs this help message and then quits.\n\n"
  "\t--depends, -d software_name\n"
  "\tSpecify additional software dependencies to be recorded in "
  "Makefile.am.\n\tNames that have both a header file and C program "
  "file may be input as\n\t'name.h+c'. "
  "You may use the option as -d \"name1.h+c name2.c+h ...\" or\n\tuse "
  "the option multiple times on different names.\n\n"

  "\t--with-options, -o\n"
  "\tThe files gopt.c and gopt.h will automatically be included in "
  "the\n\tsoftware dependencies list. These will be generated from"
  " stub files\n\tlocated in your config dir.\n\n"

  "\t--extra-dist, -x data_file or 'file1 file2 ...'\n"
  "\tFile(s) to be installed as data such as config files. "
  "Extra-dist may\n\tbe invoked more than once if needed or the list "
  "of files may be quote\n\tprotected for a single invocation.\n\n"

  "\t--options-list, -n optcode\n"
  "\twhere optcode looks like this 'xextra:', x is the short "
  "option name,\n\textra is the long option name, and the string may be"
  " ended with 0, 1\n\tor 2 occurrences of ':', indicating 0; no option"
  " argument,\n\t1; option argument is required, and 2; an option "
  "argument is optional.\n"
  "\tAll code required to process your named options will be "
  "generated\n\tincluding some 'nonsense' help text to describe these"
  " options."
  "\n\n"
  ;
  return ret;
}
