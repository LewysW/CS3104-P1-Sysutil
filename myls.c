#define _GNU_SOURCE
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <stdbool.h>

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
//Defines system call numbers for system calls used in the solution
#define WRITE_SYSCALL 1
#define STAT_SYSCALL 4
#define OPEN_SYSCALL 2
#define GETDENTS_SYSCALL 78
#define TIME_SYSCALL 201
#define CREAT_SYSCALL 85

/*Maximum directory name size in linux + length of error message. Used to
store path argument for files/directories as well as error message if path does
not exist*/
#define BUF_SIZE 4145

/*Maximum number of digits used to represent a standard integer is 10. Used to define
a char[] buffer that is used to store a string representation of an integer*/
#define MAX_INT_DIGITS 10

/*Defines the number required to convert from an integer representation of a
number to the ASCII code of that number.*/
#define ASCII_CONVERSION_INT 48

//Defines number of characters needed to represent month strings (e.g. "Jan")
#define MONTH_LENGTH 3

//Defines the year that localtime starts counting from for printing purposes (e.g. 1955 is stored as 55)
#define STARTING_YEAR 1900

//Defines upperbound of single digits for formatting check when printing time
#define SINGLE_DIGIT 9

//Defines list of month strings which are indexed using month integer returned by localtime
static const char *MONTH_STRING[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

//Defines red and green character codes for test output
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define WHITE   "\033[39m"

//Defines number of tests to be run by test suite
#define NUM_TESTS 10

//Directory entry Struct from getdents man page
struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
                      /* length is actually (d_reclen - 2 -
                         offsetof(struct linux_dirent, d_name)) */
};

//Headers for system call wrapper functions containing inline assembly
int myStat(char* fileName, struct stat* meta_data);
int myWrite(char* str);
int myGetDents(unsigned long fd, char* buf, unsigned long bufferSize);
int myOpen(char* fileName, mode_t mode);
time_t myTime(time_t* tloc);
int myCreat(const char* pathname, mode_t mode);

//Custom implementations of useful string functions
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
bool strEqual(char* str1, char* str2);
void myitoa(unsigned int num, char* str);

//Given an integer month (0-11), populates monthStr with a string month
void monthToStr(unsigned int month, char* monthStr);

//Writes error message to stdout if stat fails on a filename
void writeErrorMsg(char* fileName);

//Given a stat struct, populates a char* with file permissions of a file
void getFilePerm(struct stat meta_data, char* filePerm);

/*Given a stat struct, populates a char* with a character signifying whether a
file is a directory or not */
void getDirChar(struct stat meta_data, char* dir);

/*Functions to print data about files, including time modified, meta data about
file, as well as meta data of all files in a directory*/
void printModifiedTime(struct stat meta_data);
void printMetaData(struct stat meta_data);
void printDirEntries(char* dirName);

//Functions for unit tests
int runTests(bool (*testFunctions[]) (), int numTests);
void initTests(bool (*testFunctions[]) ());
bool myitoaTest1();
bool myitoaTest2();
bool myitoaTest3();
bool myitoaTest4();
bool myStrCpyTest1();
bool myStrCpyTest2();
bool myStrCpyTest3();
bool myStrLenTest1();
bool myStrLenTest2();
bool myStrLenTest3();

int main(int argc, char** argv)
{
    //Struct to store meta data of file specified as argument
    struct stat meta_data;

    //If file specified, get the file name
    if (argc == 2) {
        /*Gets size of file name, copies file name to buffer, and then calls myStat
        on file name to get file meta data */
        int size = myStrLen(argv[1]);
        char fileName[BUF_SIZE];
        myStrCpy(fileName, argv[1], size);
        int status = myStat(fileName, &meta_data);

        //If myStat returned successfully then check if file is directory or not
        if (!status) {
            //If file is a directory, then write data about all files in that directory
            if (S_ISDIR(meta_data.st_mode)) {
                printDirEntries(fileName);
            //Otherwise write data about that file
            } else {
                printMetaData(meta_data);
                myWrite("\n");
            }
        //Otherwise write error message to user
        } else {
            writeErrorMsg(fileName);
        }
    } else if (argc == 1) {
        //Creates list of bool functions to store test functions
        bool (*unitTests[NUM_TESTS]) ();
        initTests(unitTests);
        runTests(unitTests, NUM_TESTS);
    }

    return 0;
}

/**
Custom wrapper function for getdents system call using inline assembly
@fd - file descriptor of file to get directory entries
@buf - buffer to store directory entry data in
@bufferSize - size of buffer
@return - number of bytes read
**/
int myGetDents(unsigned long fd, char* buf, unsigned long bufferSize) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)GETDENTS_SYSCALL), "r"(fd), "r"(buf), "r"(bufferSize) :
         "%rax","%rdi", "%rsi", "%rdx", "memory" );

    return ret;
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
Custom wrapper function for write system call using inline assembly
Has been simplified to allow for easy printing
@str - string to be written to stdout
**/
int myWrite(char* str) {
    //Length of string to be printed
    size_t len = myStrLen(str);
    //Handle is 1 for stdout
    long handle = 1;
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)WRITE_SYSCALL),"r"(handle), "r"(str), "r"(len) :
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/**
Custom wrapper function for time system call using inline assembly
@tloc - location to store time in seconds since Epoch if not NULL
@return - time in seconds since Epoch
**/
time_t myTime(time_t* tloc) {
    long ret = -1;

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)TIME_SYSCALL),"r"(tloc) :
         "%rax","%rdi","memory" );

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
    if (src == NULL) {
        dest = NULL;
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
Prints meta data of file in the format of ls -n
@meta_data - meta data of file to print
**/
void printMetaData(struct stat meta_data) {
    char tempStr[BUF_SIZE];

    /* Gets the permissions and directory character of file and prints it using
    myWrite*/
    getDirChar(meta_data, tempStr);
    myWrite(tempStr);
    getFilePerm(meta_data, tempStr);
    myWrite(tempStr);

    /*Gets number of hard links, user id, group id, and size, converts all values
    to strings, and then prints using myWrite*/
    myWrite(" ");
    myitoa(meta_data.st_nlink, tempStr);
    myWrite(tempStr);
    myWrite(" ");
    myitoa(meta_data.st_uid, tempStr);
    myWrite(tempStr);
    myWrite(" ");
    myitoa(meta_data.st_gid, tempStr);
    myWrite(tempStr);
    myWrite(" ");
    myitoa(meta_data.st_size, tempStr);
    myWrite(tempStr);
    myWrite(" ");

    //Prints the time the file was last modified
    printModifiedTime(meta_data);
}

/**
Prints meta data of files in directory by making repeated calls to printMetaData
for each file name returned by myGetDents
@dirName - name of directory to print meta data of files
**/
void printDirEntries(char* dirName) {
    //Struct to store file meta data
    struct stat meta_data;
    //Struct to store directory entries returned by myGetDents
    struct linux_dirent *d;
    //Buffer to store raw data returned by myGetDents
    char buf[BUF_SIZE];

    //Opens directory for reading
    int fd = myOpen(dirName, O_RDONLY);

    //If directory opened successfully:
    if (fd != -1) {
        //Makes call to myGetDents
        int bytesRead = myGetDents(fd, buf, BUF_SIZE);

        //If data has been read successfully:
        if (bytesRead > 0) {

            //Adapted man 2 getdents code to produce following:

            /*Iterates through directory entries in buffer and stops when the
            number of bytes read is reached */
            for (int bpos = 0; bpos < bytesRead; bpos += d->d_reclen) {
                //Gets current directory entry
                d = (struct linux_dirent *) (buf + bpos);

                /* Creates buffer to store name of current directory entry,
                then appends name of directory at beginning, as well as '/', and
                finally the file name. */
                char name[BUF_SIZE];
                myStrCpy(name, dirName, myStrLen(dirName));
                myStrCpy(name + myStrLen(dirName), "/", myStrLen("/"));
                myStrCpy(name + myStrLen(dirName) + myStrLen("/"), d->d_name, myStrLen(d->d_name));

                //Calls myStat on current file to get meta data
                int status = myStat(name, &meta_data);

                /*If myStat returned successfully then the meta data is printed
                along with the file name*/
                if (!status) {
                    printMetaData(meta_data);
                    myWrite(" ");
                    myWrite(d->d_name);
                    myWrite("\n");
                }


            }
        }
    }
}

/**
 Prints error message similar to that printed by ls if file does not exist
@fileName - name of file which does not exist
**/
void writeErrorMsg(char* fileName) {
    //Assigns beginning of error message to buffer
    char error[BUF_SIZE] = "myls: cannot access '";

    //Copies file name to error message buffer
    int size = myStrLen(error);
    myStrCpy(error + size, fileName, myStrLen(fileName));

    //Appends end of error message to buffer
    size = myStrLen(error);
    myStrCpy(error + size, "': No such file or directory\n", myStrLen("': No such file or directory\n"));

    //Prints error message using myWrite
    myWrite(error);
}

//Returns a particular character depending on whether a file is a directory or not.
//Takes a stat struct as a parameter

/**
Gets character signifying whether a file is a directory or not
@meta_data - meta data of file
@dir - character array to store directory character
**/
void getDirChar(struct stat meta_data, char* dir) {
    dir[0] = S_ISDIR(meta_data.st_mode) ? 'd' : '-';
    dir[1] = '\0';
}

//BEGIN CITATION: Learned how to use the masks from this page
//- https://stackoverflow.com/questions/8812959/how-to-read-linux-file-permission-programmatically-in-c-c
/**
Gets read, write and execute permissions of file for user, group and others
@meta_data - meta data of file
@filePerm - char array to populate with file permissions
**/
void getFilePerm(struct stat meta_data, char* filePerm) {
    /*Performs bit mask operation on file mode bits defined by stat and meta
    data mode values to determine file permissions. At each stage assigns 'r',
    'w', or 'x' if permission is enabled, or '-' if not.*/
    filePerm[0] = (S_IRUSR & meta_data.st_mode) ? 'r' : '-';
    filePerm[1] = (S_IWUSR & meta_data.st_mode) ? 'w' : '-';
    filePerm[2] = (S_IXUSR & meta_data.st_mode) ? 'x' : '-';
    filePerm[3] = (S_IRGRP & meta_data.st_mode) ? 'r' : '-';
    filePerm[4] = (S_IWGRP & meta_data.st_mode) ? 'w' : '-';
    filePerm[5] = (S_IXGRP & meta_data.st_mode) ? 'x' : '-';
    filePerm[6] = (S_IROTH & meta_data.st_mode) ? 'r' : '-';
    filePerm[7] = (S_IWOTH & meta_data.st_mode) ? 'w' : '-';
    filePerm[8] = (S_IXOTH & meta_data.st_mode) ? 'x' : '-';
    filePerm[9] = '\0';
}
//END CITATION

/**
Prints last time file was modified
@meta_data - meta data of file whose modified time is to be printed
**/
void printModifiedTime(struct stat meta_data) {
    //Time structs to store file modification time and current time
    struct tm* fileTime;
    struct tm* currentTime;

    //Integers to store current year and file year
    int currentYear;
    int fileYear;

    //Buffer to store time data
    char tempStr[MAX_INT_DIGITS];

    //Gets the current time and year
    time_t current;
    current = myTime(NULL);
    currentTime = localtime(&current);
    currentYear = currentTime->tm_year + STARTING_YEAR;

    //Gets the time and year of the last modification to the file
    fileTime = localtime(&meta_data.st_mtime);
    fileYear = fileTime->tm_year + STARTING_YEAR;

    //Converts month to a string and writes it using myWrite
    monthToStr(fileTime->tm_mon, tempStr);
    myWrite(tempStr);
    myWrite(" ");

    //Converts day to string and prints it using myWrite
    myitoa(fileTime->tm_mday, tempStr);
    myWrite(tempStr);
    myWrite(" ");

    //If the modified year is the current year, prints the time of modification
    if (fileYear == currentYear) {
        //Sets initial hour value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';
        tempStr[2] = '\0';

        /*Formats hour depending on if hour is a single or double digit. Then
        prints hour of modification appended with a ':'*/
        myitoa(fileTime->tm_hour, (fileTime->tm_hour > SINGLE_DIGIT ? tempStr : tempStr + 1));
        myWrite(tempStr);
        myWrite(":");

        //Sets initial minute value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';

        /*Formats minutes depending on if minutes are single or double digit. Then
        prints minute of modification*/
        myitoa(fileTime->tm_min, (fileTime->tm_min > SINGLE_DIGIT ? tempStr : tempStr + 1));
        myWrite(tempStr);

    //Otherwise the year of modification is printed as in ls -n
    } else {
        myitoa(fileYear, tempStr);
        myWrite(tempStr);
    }

}

/**
Converts an int representation of a month to the equivalent string
(e.g. 0 is 'Jan', 1 is 'Feb')
@month - int representation of month
@monthStr - character array to populate with string month
**/
void monthToStr(unsigned int month, char* monthStr) {
    myStrCpy(monthStr, MONTH_STRING[month], MONTH_LENGTH + 1);
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
            myWrite(GREEN);
            myWrite("\n***TEST ");
            myitoa(i + 1, printBuf);
            myWrite(printBuf);
            myWrite(" PASSED***\n");
        } else {
            myWrite(RED);
            myWrite("\n***TEST ");
            myitoa(i + 1, printBuf);
            myWrite(printBuf);
            myWrite(" FAILED***\n");
        }
    }

    //Displays total number of unit tests which have passed
    myWrite("\n***");
    (numPassingTests > 0) ? myitoa(numPassingTests, printBuf) : myStrCpy(printBuf, "0", 1);
    myWrite(printBuf);
    myWrite("/");
    myitoa(i, printBuf);
    myWrite(printBuf);
    myWrite(" TESTS PASSED***\n");

    myWrite(WHITE);
    return numPassingTests;
}

/**
Initialises the array of tests to be run with the address of each unit test function.
@testFunctions - list of test functions to be called
**/
void initTests(bool (*testFunctions[]) ()) {
    testFunctions[0] = myitoaTest1;
    testFunctions[1] = myitoaTest2;
    testFunctions[2] = myitoaTest3;
    testFunctions[3] = myitoaTest4;
    testFunctions[4] = myStrCpyTest1;
    testFunctions[5] = myStrCpyTest2;
    testFunctions[6] = myStrCpyTest3;
    testFunctions[7] = myStrLenTest1;
    testFunctions[8] = myStrLenTest2;
    testFunctions[9] = myStrLenTest3;
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
