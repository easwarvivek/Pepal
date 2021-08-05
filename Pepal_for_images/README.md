# Cryptographic NDA #

***

This folder includes the implementation for the NDA. 

Prerequisite libs:

	1. GMP
	2. PBC
	3. PBC Wrapper
	4. Cryptopp

Setup on Ubuntu:

	- install m4 package
		- sudo apt-get install m4
	- install gmp (download the .tar.lz file in /usr/local/src, unzip it there and cd into it to start installation)
		- sudo ./configure
		- make
		- make install 
		- make check
	- install lex & bison
	- install PBC
	- install Cryptopp package
		- apt-cache pkgnames | grep -i crypto++ (this will give you the latest package names)
		- sudo apt-get install libcrypto++-utils libcrypto++9v5-dbg libcrypto++-dev libcrypto++-doc libcrypto++9v5
	- git clone the NDA repo
	- cd into implementation folder and run Make to create an executable called NDA
	- There are existing issues with linking the cryptopp library. use the folder in the repository's cryptopp folder and use -L./cryptopp in the makefile
		

***
Project implemented within the Wrapper class due to the preexisting makefile. In order to compile:

	- make sure prerequisite libs are installed
	- have cryptopp *.so files in /usr/lib folder
	- missing netpbm library to run external watermarking code


