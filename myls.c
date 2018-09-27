#define _GNU_SOURCE
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/syscall.h>

#include <stdio.h> //TODO - REMOVE THIS IMPORT STATEMENT AND ALL STDLIB FUNCTIONS

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1
#define GETDENTS_SYSCALL 78
//Maximum directory name size in linux + length of error message
#define MAX_ERROR_SIZE 4145
#define MAX_INT_DIGITS 10
#define ASCII_CONVERSION_INT 48
#define MONTH_LENGTH 3
#define STARTING_YEAR 1900
#define SINGLE_DIGIT 9
#define BUF_SIZE 1024

static const char *MONTH_STRING[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
                      /* length is actually (d_reclen - 2 -
                         offsetof(struct linux_dirent, d_name)) */
};

int myStat();
int myWrite(char* str);
int myOpen();
int myGetDents(int fd, char* buf, int bufferSize);
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
void myitoa(int num, char* str);

void writeErrorMsg(char* fileName);
void getFilePerm(struct stat meta_data, char* filePerm);
void getDirChar(struct stat meta_data, char* dir);
void printAccessTime(struct stat meta_data);
char* monthToStr(int month, char* monthStr);
void printMetaData(struct stat meta_data);
void printDirEntries(char* dirName);

int main(int argc, char** argv)
{
    struct stat meta_data;

    //If file specified, get the file name
    if (argc == 2) {
        int size = myStrLen(argv[1]);
        char fileName[size];
        myStrCpy(fileName, argv[1], size);
        int status = myStat(fileName, &meta_data);

        //If stat returned successfully then get convert meta data to string and write,
        //otherwise write error message.
        if (!status) {
            if (S_ISDIR(meta_data.st_mode)) {
                printDirEntries(fileName);
            } else {
                printMetaData(meta_data);
                myWrite("\n");
            }
        } else {
            writeErrorMsg(fileName);
        }
    }


    return 0;
}

int myGetDents(int fd, char* buf, int bufferSize) {
    return syscall(GETDENTS_SYSCALL, fd, buf, bufferSize);
}

int myStat(char* fileName, struct stat* meta_data) {
    return stat(fileName, meta_data);
}

void printDirEntries(char* dirName) {
    struct stat meta_data;
    struct linux_dirent *d;
    char buf[BUF_SIZE];

    int fd = open(dirName, O_RDONLY);

    //Adapted 'man 2 getdents' man page code which reads in the directory entries.
    // Then stores then in a file and prints their meta data and name.
    if (fd > -1) {
        int bytesRead = myGetDents(fd, buf, BUF_SIZE);
        if (bytesRead > 0) {
            for (int bpos = 0; bpos < bytesRead;) {
                d = (struct linux_dirent *) (buf + bpos);
                int status = myStat(d->d_name, &meta_data);

                if (!status) {
                    printMetaData(meta_data);
                    myWrite(" ");
                    myWrite(d->d_name);
                    myWrite("\n");
                }

                bpos += d->d_reclen;
            }
        }
    }
}

//Wrapper for the 'write' system call
//Takes a string as an argument and calculates the length using myStrLen
int myWrite(char* str) {
    return write(WRITE_SYSCALL, str, myStrLen(str));
}

//Custom implementation of strlen() function.
//Takes char* parameter to return length of (ignores '\0')
int myStrLen(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++);
    return i;
}

//Custom strcpy impelmentation
//Takes char* for destination to copy to and const char* to copy from, and
//number of bytes to copy n.
void myStrCpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

//Converts an integer to a character array that can be output using write().
//Takes an integer as a parameter.
void myitoa(int num, char* str) {
    char intStr[MAX_INT_DIGITS];
    int i = 0;

    //Gets digits from least to most significant (reverse order in array)
    while (num) {
        intStr[i++] = num % 10;
        num /= 10;
    }
    intStr[i] = '\0';

    //Converts digits to ASCII and reorders in new array
    for (int j = 0, k = i - 1; j < i; j++, k--) {
        str[k] = intStr[j] + ASCII_CONVERSION_INT;
    }

    str[i] = '\0';
}

void printMetaData(struct stat meta_data) {
    char tempStr[MAX_INT_DIGITS];

    getDirChar(meta_data, tempStr);
    myWrite(tempStr);
    getFilePerm(meta_data, tempStr);
    myWrite(tempStr);

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
    printAccessTime(meta_data);
}

//Writes an error message to the terminal if the file/directory does not exist
//Takes in the name of the file.
void writeErrorMsg(char* fileName) {
    char error[MAX_ERROR_SIZE] = "myls: cannot access ";
    int size = myStrLen(error);
    myStrCpy(error + size, fileName, myStrLen(fileName));
    size = myStrLen(error);
    myStrCpy(error + size, ": No such file or directory\n", myStrLen(": No such file or directory\n"));
    write(WRITE_SYSCALL, error, myStrLen(error)); //TODO replace with my write wrapper
}

//Returns a particular character depending on whether a file is a directory or not.
//Takes a stat struct as a parameter
void getDirChar(struct stat meta_data, char* dir) {
    dir[0] = S_ISDIR(meta_data.st_mode) ? 'd' : '-';
    dir[1] = '\0';
}

//Gets the permission for user, group and others using the bitmasks provided by stat
//Takes a stat struct as a parameter, and destination to store permissions
//BEGIN CITATION: Learned how to use the masks from this page
//- https://stackoverflow.com/questions/8812959/how-to-read-linux-file-permission-programmatically-in-c-c
void getFilePerm(struct stat meta_data, char* filePerm) {
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

void printAccessTime(struct stat meta_data) {
    struct tm* fileTime;
    struct tm* currentTime;
    int currentYear;
    int fileYear;
    char tempStr[MAX_INT_DIGITS];

    //Gets the current time and year
    time_t current;
    current = time(NULL);
    currentTime = localtime(&current);
    currentYear = currentTime->tm_year + STARTING_YEAR;

    //Gets the time and year of the last modification to the file
    fileTime = localtime(&meta_data.st_mtime);
    fileYear = fileTime->tm_year + STARTING_YEAR;

    //Converts month to a string word and writes it
    myWrite(monthToStr(fileTime->tm_mon, tempStr));

    myWrite(" ");

    //Gets day and prints it to terminal
    myitoa(fileTime->tm_mday, tempStr);
    myWrite(tempStr);

    myWrite(" ");

    if (fileYear == currentYear) {
        //Sets initial hour value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';
        tempStr[2] = '\0';

        //Decides how many hour digits to copy based on size of hour time.
        //Then prints hour time and a ':'
        myitoa(fileTime->tm_hour, (fileTime->tm_hour > SINGLE_DIGIT ? tempStr : tempStr + 1));
        myWrite(tempStr);
        myWrite(":");

        //Sets initial minute value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';

        //Decides how many minute digits to copy based on size of minute time.
        //Then prints minute time
        myitoa(fileTime->tm_min, (fileTime->tm_min > SINGLE_DIGIT ? tempStr : tempStr + 1));
        myWrite(tempStr);
    } else {
        myitoa(fileYear, tempStr);
        myWrite(tempStr);
    }

}

char* monthToStr(int month, char* monthStr) {
    myStrCpy(monthStr, MONTH_STRING[month], MONTH_LENGTH + 1);
    return monthStr;
}
