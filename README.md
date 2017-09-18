# newprogram

**newprogram** generates a new C program in it's own named directory.

The generated code comprises the main C source file, optional options
processing and all the autotools necessary to build and run this new
program. Code generated includes accurate but most likely useless help
text for the specified options. Named source file code is hard linked
into the new project directory.

## SYNOPSIS

**newprogram** \[options] new_pr_name


## Example invocation

```
newprogram --with-options \
--options-list 'nno-optarg mmust-have-optarg: ccould-have-optarg::' \
--depends 'dirs.c+h files.h+c str.c+h firstrun.h+c' \
--extra-dist 'data-file1 data-file2 ... data-fileN' \
TestProgram
```

## Results

1. A directory named _Testprogram_ will be created in your named program
directory.

2. A main C program _testprogam.c_ will be created in the new directory.

3. All necessary autotools files will be generated to allow the usual
**./configure** && **make** && **sudo make install** to be performed.

4. Because *--with-options* was invoked, the files gopt.c and gopt.h
will be created also. By default *--help* option is provided
automatically.

5. The *--options-list* options in this example:

    a. *nno-optarg* - the beginning **n** is the short option name, the
    next word **no-optarg** is the long option name and the absence
    of any **:** suffix means that there is no options argument.

    b. *mmust-have-optarg:* - **m** is the short name and
    **must-have-optarg:** the long name. The suffix **:** means that an
    options argument _must_ be provided if the option is selected.

    c. *ccould-have-optarg::* - naming (**c** and **could-have-optarg**)
    is the same as above. The **::** suffix generates code that
    supports that the user may _optionally_ provide an option argument.

    d. The example above invokes the *--options-list* option with a
    quote protected space separated list. It would be just as valid to
    invoke the option 3 times, each time with a single option specifier.

    e. When this option is invoked the *--with-options* invocation is
    redundant but harmless.

6. The *--depends* option is followed by a quote protected space
separated list of software names. The names in this example all have
'c+h' on the end. In processing eg _dirs.c+h_ will be expanded to
_dirs.c_ and _dirs.h_.

    a. This expanded software list will be entered as dependency names
    in the completed _Makefile.am_.

    b. Also, two named directories will searched for this software
    list, a *boilerplate* dir and a *components* dir. Items found in
    the *boilerplate* dir will be copied into the new project dir and
    those found in the *components* dir will be hardlinked there.

    c. You can enter software names that do not yet exist in the
    searched dirs and warning messages will be issued for any such
    items.

    d. The option may be invoked any number of times on single software
    items or on shorter lists.

7. The *--extra-dist* option may be invoked many times on single items
or, as in the example a quote protected space separated list of
filenames. These files will when **sudo make install** is run will be
installed in */usr/local/share/testprogram/*.

8. An empty man page, in this example, *testprogram.1* is generated and
when **sudo make install** is run, it will be installed in
 */usr/local/share/man/man1/*. It is also automatically inserted in the
 *extra-dist* list so that **sudo make distcheck** will not fail.<br />
 Consequently a redundant copy of *testprogram.1* is also placed in
 */usr/local/share/testprogram/*. Nothing's perfect!

