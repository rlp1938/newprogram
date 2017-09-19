% newprogram(1) | General Command
% Robert L Parker
% 2017-09-19

# NAME

**newprogram** - generates a new C program with properties determined by
options chosen by the user.

# SYNOPSIS

**newprogram** \[**-h**] \[**--help**]

**newprogram** \[options] project_name

# DESCRIPTION

**newprogram** creates a new directory under the users designated
_programs directory_, creates the main C program based on the
project_name input. If the project_name was _someThing_ then the new
project dir will be _Something_ and the main program _something.c_.
The resulting binary will be _something_ and the manpage _something.1_.
All required _autotools_ files will be generated and any optional
software dependencies will be entered into the _Makefile.am_. Such
dependenies, if found within the user designated source file sub-dir
will be hard linked into the project dir, or if found within the user
designated boilerplate dir will be copied there for subsequent editing.

# OPTIONS

**-h, --help**
:   Displays this help message then quits.


**--depends, -d** software_name or 'list of software names'
:   Specify additional software dependencies to be recorded in
Makefile.am. Names that have both a header file and C program file may
be input as 'name.h+c'. You may use the option as -d 'name1.h+c
name2.c+h ...' or use the option multiple times on different names.

**--with-options, -o**
:    The files gopt.c and gopt.h will automatically be included in the
software dependencies list. These will be generated from stub files
located in your config dir. These stubs will be installed there on the
first run of this program. Help text for the target program is provided
 automatically for when the **--help** option is selected.

**--options-list, -n** optcode
:    where **optcode** looks like this *xextra:*, x is the short option
name, *extra* is the long option name, and the string may be ended with
0, 1 or 2 occurrences of **':'**, indicating 0; no option argument,
1; option argument is required, and 2; an option argument is optional.
All code required to process your named options will be generated
including some 'nonsense' help text to describe these options.
As for the **--depends** option this option may be invoked once on a
quote protected, space separated list of **opcodes** or invoked several
times on single items or shorter lists.
If this option is invoked, the **--with-options** option is redundant
but harmless.

**--extra-dist, -x** data_file or 'file1 file2 ...'
:    File(s) to be installed as data in
_/usr/local/share/<program_name>_ such as config files. Extra-dist
may be invoked more than once if needed or the list of files may be
quote protected for a single invocation.


# NOTE

There is no need for any action to be taken about the manpage.
At present it will be empty at creation time and should be written up
as the target prgram is being developed. In future a rudimentary
manpage will be generated based on the generated options.
