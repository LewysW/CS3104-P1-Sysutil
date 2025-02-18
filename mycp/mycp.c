#define _GNU_SOURCE
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
//Defines system call numbers for system calls used in the solution
#define READ_SYSCALL 0
#define WRITE_SYSCALL 1
#define STAT_SYSCALL 4
#define OPEN_SYSCALL 2
#define CLOSE_SYSCALL 3
#define CREAT_SYSCALL 85
#define UNLINK_SYSCALL 87
#define RMDIR_SYSCALL 84
#define MKDIR_SYSCALL 83
#define TRUNCATE_SYSCALL 76

/*Maximum directory name size in linux + length of error message. Used to
store path argument for files/directories as well as error message if path does
not exist*/
#define BUF_SIZE 4145

#define BLOCK_SIZE 4096

/*Maximum number of digits used to represent a standard integer is 10. Used to define
a char[] buffer that is used to store a string representation of an integer*/
#define MAX_INT_DIGITS 10

/*Defines the number required to convert from an integer representation of a
number to the ASCII code of that number.*/
#define ASCII_CONVERSION_INT 48

//std streams
#define stdout 1
#define stderr 2

//Defines red and green character codes for test output
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define WHITE   "\033[39m"

//Defines number of tests to be run by test suite
#define NUM_TESTS 33

//Defines error flags for writeErrorMsg
#define ERRSTAT -1
#define ERRREC -2
#define ERRDIR -3
#define ERRDEST -4
#define ERRPERM -13

//Headers for system call wrapper functions containing inline assembly
int myStat(char* fileName, struct stat* meta_data);
int myWrite(int fd, const void* buf, size_t count);
int myOpen(char* fileName, mode_t mode);
int myClose(long fd);
int myCreat(const char* pathname, mode_t mode);
int myUnlink(const char* pathname);
int myrmdir(const char* pathname);
int mymkdir(const char* pathname, mode_t mode);
int myRead(int fd, void* buf, size_t count);
int myTruncate(const char* path, off_t length);

//Custom implementations of useful string functions
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
bool strEqual(char* str1, char* str2);
void myitoa(unsigned int num, char* str);
void myPrint(char* str);

//Gets character representing type of file
void getDirChar(struct stat meta_data, char* dir);

//Writes error message to stdout if mycp fails
void writeErrorMsg(char* fileName, int flag);

//Carries out cp operation
void mycp(char* dest, char* src);

//Performs operation of writing data to file
void writeToFile(int dest, int src);

//Functions for unit tests
int runTests(bool (*testFunctions[]) (), int numTests);
void initTests(bool (*testFunctions[]) ());
bool strEqualTest1();
bool strEqualTest2();
bool strEqualTest3();
bool strEqualTest4();
bool myitoaTest1();
bool myitoaTest2();
bool myitoaTest3();
bool myitoaTest4();
bool myStrCpyTest1();
bool myStrCpyTest2();
bool myStrCpyTest3();
bool myStrCpyTest4();
bool myStrLenTest1();
bool myStrLenTest2();
bool myStrLenTest3();
bool myStatTest1();
bool myStatTest2();
bool myStatTest3();
bool myOpenTest1();
bool myOpenTest2();
bool myCloseTest1();
bool myCloseTest2();
bool myWriteTest1();
bool myWriteTest2();
bool myWriteTest3();
bool myCreatTest1();
bool myCreatTest2();
bool myUnlinkTest1();
bool myUnlinkTest2();
bool myMkdirTest1();
bool myMkdirTest2();
bool myRmdirTest1();
bool myRmdirTest2();

/**
Main function.
Decides whether to run program to produce output similar to "ls -n" command
or if no arguments are specified, runs unit tests.
@argc - number of arguments
@argv - list of arguments
**/
int main(int argc, char** argv)
{

    if (argc >= 3) {
        //Struct to store meta data of file specified as argument
        struct stat meta_data;

        //Checks if final argument is directory for multiple file copying
        if (argc > 3) {
            if (!myStat(argv[argc - 1], &meta_data)) {
                if (!S_ISDIR(meta_data.st_mode)) {
                    writeErrorMsg(argv[argc - 1], ERRDIR);
                    return ERRDIR;
                }
            } else {
                writeErrorMsg(argv[argc - 1], ERRSTAT);
                return ERRSTAT;
            }
        }

        //Iterates over each file and checks for error, otherwise copies
        for (int i = 1; i < argc - 1; i++) {
            int status = myStat(argv[i], &meta_data);

            //If myStat failed write error
            if (status != 0) {
                writeErrorMsg(argv[i], ERRSTAT);
            } else {
                //Otherwise if file is directory, tell user that -r is not enabled
                if (S_ISDIR(meta_data.st_mode)) {
                    writeErrorMsg(argv[i], ERRREC);
                //Otherwise copy file
                } else {
                    mycp(argv[argc - 1], argv[i]);
                }
            }
        }
    //If single file argument, write error to user
    } else if (argc == 2) {
        writeErrorMsg(argv[1], ERRDEST);
    //If one argument, run unit tests
    } else if (argc == 1) {
        //Creates list of bool functions to store test functions
        bool (*unitTests[NUM_TESTS]) ();
        initTests(unitTests);
        runTests(unitTests, NUM_TESTS);
    }

    return 0;
}

/**
Copies source file to destination file/directory
@dest - destination to copy to
@src - source to be copied
**/
void mycp(char* dest, char* src) {
    int destFd;
    struct stat dest_meta_data;
    struct stat src_meta_data;
    myStat(dest, &dest_meta_data);
    myStat(dest, &src_meta_data);

    //If destination is a file
    if (S_ISREG(dest_meta_data.st_mode)) {
        //Truncate file to size 0 and open destination for writing
        myTruncate(dest, 0);
        destFd = myOpen(dest, O_RDWR);

    //Otherwise if destination is directory
    } else if (S_ISDIR(dest_meta_data.st_mode)) {
        //Create path of new file relative to directory path
        char buf[BUF_SIZE];
        myStrCpy(buf, dest, myStrLen(dest));
        myStrCpy(buf + myStrLen(dest), "/", 1);
        myStrCpy(buf + myStrLen(dest) + 1, src, myStrLen(src));

        //And create file with name of old file and the same permissions
        destFd = myCreat(buf, src_meta_data.st_mode);
    }

    //Open source file for reading
    int srcFd = myOpen(src, O_RDONLY);

    if (srcFd < 0) writeErrorMsg(src, srcFd);

    //Write data from source file to destination file if any
    if (src_meta_data.st_size > 0) writeToFile(destFd, srcFd);

    myClose(destFd);
    myClose(srcFd);
}

/**
Writes data from src to dest
@dest - fd of destination file
@src - fd of source file
**/
void writeToFile(int dest, int src) {
    int bytesRead;
    char buf[BLOCK_SIZE];
    while ((bytesRead = myRead(src, buf, BLOCK_SIZE)) != 0) myWrite(dest, buf, bytesRead);
}



/**
Custom wrapper function for stat system call using inline assembly
@fileName - name of file to get meta data about
@meta_data - struct to store file meta data in
@return - status code
**/
int myStat(char* fileName, struct stat* meta_data) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)STAT_SYSCALL), "r"(fileName), "r"(meta_data) :
         "%rax","%rdi", "%rsi","memory" );

    return ret;
}

/**
Custom wrapper function for open system call using inline assembly
@fileName - name of file to open
@mode - access mode of file
@return - file descriptor if successful, -1 if error occurred
**/
int myOpen(char* fileName, mode_t mode) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)OPEN_SYSCALL), "r"(fileName), "r"((long)mode) :
         "%rax","%rdi", "%rsi","memory" );

    return ret;
}

/**
Custom wrapper function for close system call using inline assembly
@fd - file descriptor of file to close
@return - 0 if successful, -1 if error and errno is set
**/
int myClose(long fd) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)CLOSE_SYSCALL), "r"(fd) :
         "%rax","%rdi","memory" );

    return ret;
}

/**
Custom wrapper function for write system call using inline assembly
@fd - stream to write to
@buf - to write from
@count - size to write
**/
int myWrite(int fd, const void* buf, size_t count) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)WRITE_SYSCALL),"r"((long)fd), "r"(buf), "r"(count) :
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/**
Custom wrapper function for creat system call using inline assembly
@pathname - path of file to create
@mode - access mode of file
@return - file descriptor if successful, -1 if error occurred
**/
int myCreat(const char* pathname, mode_t mode) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)CREAT_SYSCALL), "r"(pathname), "r"((long)mode) :
         "%rax","%rdi", "%rsi","memory" );

    return ret;
}

/**
Custom wrapper function for mkdir system call using inline assembly
@pathname - path of directory to create
@mode - access mode of directory
@return - file descriptor if successful, -1 if error occurred
**/
int mymkdir(const char* pathname, mode_t mode) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)MKDIR_SYSCALL), "r"(pathname), "r"((long)mode) :
         "%rax","%rdi", "%rsi","memory" );

    return ret;
}

/**
Custom wrapper function for unlink system call using inline assembly
@pathname - path of file to delete
@return - result of deletion
**/
int myUnlink(const char* pathname) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)UNLINK_SYSCALL), "r"(pathname) :
         "%rax","%rdi","memory" );

    return ret;
}

/**
Custom wrapper function for rmdir system call using inline assembly
@pathname - path of directory to delete
@return - result of deletion
**/
int myrmdir(const char* pathname) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)RMDIR_SYSCALL), "r"(pathname) :
         "%rax","%rdi","memory" );

    return ret;
}

/**
Custom wrapper function for read system call using inline assembly
@pathname - path of directory to delete
@return - result of deletion
**/
int myRead(int fd, void* buf, size_t count) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)READ_SYSCALL), "r"((long)fd), "r"(buf), "r"(count) :
         "%rax","%rdi", "%rsi", "%rdx", "memory" );

    return ret;
}

/**
Custom wrapper function for truncate system call using inline assembly
@path - path of file to truncate
@length - length to truncate file to
**/
int myTruncate(const char* path, off_t length) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)TRUNCATE_SYSCALL), "r"(path), "r"((long)length) :
         "%rax","%rdi", "%rsi","memory" );

    return ret;
}

/**
Custom implementation of strlen function
@str - string to get the length of
@return - length of string not including '\0'
**/
int myStrLen(char* str) {
    if (str == NULL) return -1;

    int i;
    for (i = 0; str[i] != '\0'; i++);
    return i;
}

/**
Custom implementation of strcpy function
@dest - destination to copy string to
@src - source to copy string from
@n - size of string to copy in bytes
**/
void myStrCpy(char* dest, const char* src, size_t n) {
    //If src is NULL then operation is terminated.
    if (src == NULL) {
        return;
    }

    size_t i;
    for (i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

/**
Compares two strings for equality - used for unit testing
@str1 - first string
@str2 - second string
return - whether string is equal
**/
bool strEqual(char* str1, char* str2) {
    if (str1 == NULL || str2 == NULL) return (str1 == str2);

    if (myStrLen(str1) != myStrLen(str2)) return false;

    for (int i = 0; str1[i] != '\0'; i++) {
        if (str1[i] != str2[i]) return false;
    }

    return true;
}

/**
Custom implementation of itoa function
@num - positive integer to convert to string
@str - char* to store converted string
**/
void myitoa(unsigned int num, char* str) {
    char intStr[MAX_INT_DIGITS + 1];
    int i = 0;

    //Assigns 0 string if num is zero
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    //Gets digits from least to most significant (reverse order in array)
    while (num) {
        //Gets least significant bit
        intStr[i++] = num % 10;
        //Reduces size of num by factor of 10
        num /= 10;
    }
    intStr[i] = '\0';

    //Converts digits to ASCII and reorders in new array
    for (int j = 0, k = i - 1; j < i; j++, k--) {
        str[k] = intStr[j] + ASCII_CONVERSION_INT;
    }

    str[i] = '\0';
}

/**
Gets character signifying whether a file is a directory or not
@meta_data - meta data of file
@dir - character array to store directory character
**/
void getDirChar(struct stat meta_data, char* dir) {
    dir[0] = S_ISDIR(meta_data.st_mode) ? 'd' : '-';
    dir[1] = '\0';
}

/**
Convenient wrapper for myWrite that prints to stdout
@str - string to print to stdout
**/
void myPrint(char* str) {
    myWrite(stdout, str, myStrLen(str));
}

/**
 Prints cp error message
 @fileName - file name which causes error
 @flag - type of error
**/
void writeErrorMsg(char* fileName, int flag) {
    if (flag == ERRSTAT)  {
        myPrint("mycp: cannot stat '");
        myPrint(fileName);
        myPrint("' No such file or directory");
        myPrint("\n");
    } else if (flag == ERRREC) {
        myPrint("mycp: -r not specified; omitting directory '");
        myPrint(fileName);
        myPrint("'\n");
    } else if (flag == ERRDEST) {
        myPrint("mycp: missing destination file operand after '");
        myPrint(fileName);
        myPrint("'\n");
    } else if (flag == ERRDIR) {
        myPrint("mycp: target '");
        myPrint(fileName);
        myPrint("' is not a directory\n");
    } else if (flag == ERRPERM) {
        myPrint("mycp: cannot open '");
        myPrint(fileName);
        myPrint("' for reading: Permission denied\n");
    }
}


/**
Runs a given list of unit tests.
Iterates over each unit test in the list of test functions. If calling the
current function returns true then the test has passed, otherwise it has failed.
@testFunctions - list of unit tests to run
@numTests - num tests to run
@returns - number of passing unit tests
*/
int runTests(bool (*testFunctions[]) (), int numTests) {
    //Number of tests that have passed
    int numPassingTests = 0;
    char printBuf[MAX_INT_DIGITS];

    //Lists result of each unit test
    int i;
    for (i = 0; i < numTests; i++) {
        if ((*testFunctions[i]) ()) {
            numPassingTests += 1;
            myPrint(GREEN);
            myPrint("\n***TEST ");
            myitoa(i + 1, printBuf);
            myPrint(printBuf);
            myPrint(" PASSED***\n");
        } else {
            myPrint(RED);
            myPrint("\n***TEST ");
            myitoa(i + 1, printBuf);
            myPrint(printBuf);
            myPrint(" FAILED***\n");
        }
    }

    //Displays total number of unit tests which have passed
    myPrint("\n***");
    (numPassingTests > 0) ? myitoa(numPassingTests, printBuf) : myStrCpy(printBuf, "0", 1);
    myPrint(printBuf);
    myPrint("/");
    myitoa(i, printBuf);
    myPrint(printBuf);
    myPrint(" TESTS PASSED***\n");

    myPrint(WHITE);
    return numPassingTests;
}

/**
Initialises the array of tests to be run with the address of each unit test function.
@testFunctions - list of test functions to be called
**/
void initTests(bool (*testFunctions[]) ()) {
    testFunctions[0] = strEqualTest1;
    testFunctions[1] = strEqualTest2;
    testFunctions[2] = strEqualTest3;
    testFunctions[3] = strEqualTest4;
    testFunctions[4] = myitoaTest1;
    testFunctions[5] = myitoaTest2;
    testFunctions[6] = myitoaTest3;
    testFunctions[7] = myitoaTest4;
    testFunctions[8] = myStrCpyTest1;
    testFunctions[9] = myStrCpyTest2;
    testFunctions[10] = myStrCpyTest3;
    testFunctions[11] = myStrCpyTest4;
    testFunctions[12] = myStrLenTest1;
    testFunctions[13] = myStrLenTest2;
    testFunctions[14] = myStrLenTest3;
    testFunctions[15] = myStatTest1;
    testFunctions[16] = myStatTest2;
    testFunctions[17] = myStatTest3;
    testFunctions[18] = myOpenTest1;
    testFunctions[19] = myOpenTest2;
    testFunctions[20] = myCloseTest1;
    testFunctions[21] = myCloseTest2;
    testFunctions[22] = myWriteTest1;
    testFunctions[23] = myWriteTest2;
    testFunctions[24] = myWriteTest3;
    testFunctions[25] = myCreatTest1;
    testFunctions[26] = myCreatTest2;
    testFunctions[27] = myUnlinkTest1;
    testFunctions[28] = myUnlinkTest2;
    testFunctions[29] = myMkdirTest1;
    testFunctions[30] = myMkdirTest2;
    testFunctions[31] = myRmdirTest1;
    testFunctions[32] = myRmdirTest2;
}

//Tests that strEqual returns true if two strings are equal
bool strEqualTest1() {
    return (strEqual("String", "String"));
}

//Tests that strEqual returns false if two strings of different lengths are unequal
bool strEqualTest2() {
    return (!strEqual("String", "Not String"));
}

//Tests that strEqual returns false if two strings of the same length are unequal
bool strEqualTest3() {
    return (!strEqual("String", "Hello!"));
}

//Test that strEqual returns true if provided with two NULL strings
bool strEqualTest4() {
    return (strEqual(NULL, NULL));
}

//Tests that myitoa returns string representation of 0
bool myitoaTest1() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(0, buf);
    return (strEqual(buf, "0"));
}

//Tests that myitoa returns string representation of 1
bool myitoaTest2() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(1, buf);
    return (strEqual(buf, "1"));
}

//Tests that myitoa does not return a negative string as an unsigned int is used
bool myitoaTest3() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(-1, buf);
    return (!strEqual(buf, "-1"));
}

//Tests that myitoa converts normal data to string
bool myitoaTest4() {
    char buf[MAX_INT_DIGITS + 1];
    myitoa(1024, buf);
    return (strEqual(buf, "1024"));
}

//Tests that myStrCpy can copy a normal string from one buffer to another
bool myStrCpyTest1() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "Hello";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, "Hello"));
}

//Tests that myStrCpy can copy an empty string
bool myStrCpyTest2() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, ""));
}

//Tests that myStrCpy can copy a pathname
bool myStrCpyTest3() {
    char buf[BUF_SIZE];
    char str[BUF_SIZE] = "~/Documents/CS3104/practicals/CS3104-P1-Sysutil";
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, "~/Documents/CS3104/practicals/CS3104-P1-Sysutil"));
}

//Tests to make sure that myStrCpy refuses to copy a NULL pointer
bool myStrCpyTest4() {
    char buf[BUF_SIZE] = "Original String";
    char* str = NULL;
    myStrCpy(buf, str, myStrLen(str));
    return (strEqual(buf, "Original String"));
}

//Test myStrCpy with some normal test data
bool myStrLenTest1() {
    char buf[6] = "Hello";
    return (myStrLen(buf) == 5);
}

//Tests myStrLen with an empty string
bool myStrLenTest2() {
    char buf[10] = "";
    return (myStrLen(buf) == 0);
}

//Tests myStrLen with NULL
bool myStrLenTest3() {
    char* buf = NULL;
    return (myStrLen(buf) == -1);
}

//Tests that myStat returns 0 for a valid file
bool myStatTest1() {
    struct stat meta_data;
    int status = myStat("mycp.c", &meta_data);
    return (status == 0);
}

//Tests that myStat returns an error number for a NULL file pointer
bool myStatTest2() {
    struct stat meta_data;
    int status = myStat(NULL, &meta_data);
    return (status != 0);
}

//Tests that myStat returns an error number for a non-existent file
bool myStatTest3() {
    struct stat meta_data;
    int status = myStat("Non-existent file", &meta_data);
    return (status != 0);
}

//Tests that valid file opens successfully for read only
bool myOpenTest1() {
    int fd = myOpen("mycp.c", O_RDONLY);
    myClose(fd);
    return (fd != -1);
}

//Tests that invalid file failes to open successfully and return positive fd
bool myOpenTest2() {
    int fd = myOpen("Non-existent file", O_RDONLY);
    myClose(fd);
    return (fd < 0);
}

//Tests that valid open file is closed successfully
bool myCloseTest1() {
    int fd = myOpen("mycp.c", O_RDONLY);
    int status =  myClose(fd);
    return (status == 0);
}

//Tests that unopened file cannot be closes successfully
bool myCloseTest2() {
    int fd = myOpen("Non-existent file", O_RDONLY);
    int status = myClose(fd);
    return (status != 0);
}

//Tests that zero bytes are written for empty string
bool myWriteTest1() {
    char buf[BUF_SIZE] = "";
    int bytesWritten = myWrite(stdout, buf, myStrLen(buf));
    return (bytesWritten == 0);
}

//Tests that valid positive number of bytes are written for string
bool myWriteTest2() {
    char buf[BUF_SIZE] = "\n~/Documents/CS3104/practicals/CS3104-P1-Sysutil\n";
    int bytesWritten = myWrite(stdout, buf, myStrLen(buf));
    return (bytesWritten == myStrLen(buf));
}

//Tests that error code is returned when trying to write NULL string
bool myWriteTest3() {
    char* buf = NULL;
    int bytesWritten = myWrite(stdout, buf, myStrLen(buf));
    return (bytesWritten < 0);
}

//Tests that new file can be created successfully
bool myCreatTest1() {
    int fd = myCreat("NewFile.txt", O_RDONLY);
    myUnlink("NewFile.txt");
    return (fd > 0);
}

//Tests that error is returned when file name is NULL
bool myCreatTest2() {
    int fd = myCreat(NULL, O_RDONLY);
    return (fd < 0);
}

//Tests that file is successfully deleted
bool myUnlinkTest1() {
    myCreat("NewFile.txt", O_RDONLY);
    int status = myUnlink("NewFile.txt");
    return (status == 0);
}

//Tests that error is returned when attempt is made to delete non-existent file
bool myUnlinkTest2() {
    int status = myUnlink("Non-existent.txt");
    return (status < 0);
}

//Tests that directory can be created successfully
bool myMkdirTest1() {
    int status = mymkdir("Test Directory", O_RDONLY);
    myrmdir("Test Directory");
    return (status == 0);
}

//Tests that error is returned for invalid directory creation
bool myMkdirTest2() {
    int status = mymkdir(NULL, O_RDONLY);
    return (status < 0);
}

//Tests that empty directory can be successfully deleted
bool myRmdirTest1() {
    mymkdir("Test Directory", O_RDONLY);
    int status = myrmdir("Test Directory");
    return (status == 0);
}

//Tests that non-empty directory cannot be deleted
bool myRmdirTest2() {
    //Gives permissiosn in order to be able to delete file
    mymkdir("TestDirectory", 0775);
    int fd = myCreat("TestDirectory/NewFile.txt", O_RDONLY);
    myClose(fd);
    int status = myrmdir("TestDirectory");

    //Clean up
    myUnlink("TestDirectory/NewFile.txt");
    myrmdir("TestDirectory");

    return (status < 0);
}
