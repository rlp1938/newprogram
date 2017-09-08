# newprogram
A C program to initiate a new C programming project.
## What it will do for you
1. It will create a new named project directory within your designated programs directory.
2. It will generate your _Makefile.am_ within your new project directory.
3. Optionally provide a list of software dependencies within the _Makefile.am_ and hardlink those software files into your project direcory if they reside within a named software source library or copy boilerplate files into the project directory if they reside in a named _Stubs_ directory.
4. Optionally name files for _extra distribution_ within the _Makefile.am._
5. Automatically provide the code in _Makefile.am_ to name the project manpage.

### Naming conventions
The names within the project are determined by the name you use when you run the program.  
For example
```
newprogram [option] SomeThing
```
1. Source directory will be: _Something_
2. The binary will be: _something_
3. And the manpage: _something.1_

No matter what case you use for the target name, the case of the resulting objects will be as shown above.

