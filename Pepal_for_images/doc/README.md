Points to keep in mind for setup phase when installing libraries

1. make sure prereqs/dependencies are installed before installing intended lib.

2. Python based libraries require an additional step before they are ready for use

   ***basic steps***
   
	- download tarball
	- "tar vxf <filename>"
	- cd <extracted folder>
	- look for file named "configure" or "configure.sh" & run it.
	- make
	- make check
	- make install
	- make test (if available)
	- ldconfig
	- Note: Use 'sudo' where needed

   ***extra steps for Python libs***

	- check for file called "setup.py" in source dir
	- make sure you are in the same directory as the file when running below command.
	- python setup.py install
	- the above command will edit the sys.path for the current Python installation so the module/lib can be called from other files in the system. 

Useful links:

https://jhuisi.github.io/charm/cryptographers.html

http://stackoverflow.com/questions/14295680/cannot-import-a-python-module-that-is-definitely-installed-mechanize


https://docs.python.org/2/install/
