# CS3104-P1-Sysutil
System utility practical for CS3104 Operating Systems module.

#Compilation
1. Open a command prompt in the practical directory containing the source file "mycp.c" and the make file "Makefile"

2. Run the command "make" in the command line

3. If that doesn't work, try running "make clean" and then repeating the previous step

#Execution - System Utility
To execute the "cp" style system utility:

To copy one file to another:

1. Open a command prompt in the directory containing the executable "mycp"

2. Run the command "./mycp <path of file> <path of file>" in the command line
	
	e.g. "./mycp file1 file2"

To copy one or more files to a directory:

1. Open a command prompt in the directory containing the executable "mycp"

2. Run the command "./mycp <path of file> ... <path of file> <path of directory>" in the command line (one or more files followed by a directory)

	e.g. "./mycp file1 file2 directory1"


#Execution - Unit Tests
To execute the automated unit tests of the solution:

1. Open a command prompt in the directory containing the executable "mycp"

2. Run the command "./mycp" in the command line (no command line arguments)

