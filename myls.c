#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

#include <stdio.h> //TODO - REMOVE THIS IMPORT STATEMENT AND ALL STDLIB FUNCTIONS

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1
//Maximum directory name size in linux + length of error message
#define MAX_ERROR_SIZE 4145
#define NUM_PERMISSIONS 9
#define MAX_INT_DIGITS 10
#define ASCII_CONVERSION_INT 48

int mystat();
int mywrite();
int myopen();
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
void writeErrorMsg(char* fileName);
void getFilePerm(struct stat meta_data, char* filePerm);
char getDirChar(struct stat meta_data);
nlink_t getLinks(struct stat meta_data);
char* myitoa(int num, char* str);
void writeWrapper(char* str);
uid_t getUID(struct stat meta_data);
gid_t getGID(struct stat meta_data);
off_t getSize(struct stat meta_data);
time_t getAccessTime(struct stat meta_data);

int main(int argc, char** argv)
{
    struct stat meta_data;

    //If file specified, get the file name
    if (argc == 2) {
        int size = myStrLen(argv[1]);
        char fileName[size];
        myStrCpy(fileName, argv[1], size);
        int status = stat(fileName, &meta_data);

        //If stat returned successfully then get meta data to write,
        //otherwise write error message.
        if (!status) {
            char dirChar[2];
            dirChar[0] = getDirChar(meta_data);
            dirChar[1] = '\0';
            writeWrapper(dirChar);

            char filePerm[NUM_PERMISSIONS + 1];
            getFilePerm(meta_data, filePerm);
            writeWrapper(filePerm);

            char* links = NULL;
            links = myitoa(getLinks(meta_data), links);
            writeWrapper(links);

            char* uid = NULL;
            uid =  myitoa(getUID(meta_data), uid);
            writeWrapper(uid);

            char* gid = NULL;
            gid =  myitoa(getUID(meta_data), gid);
            writeWrapper(gid);

            char* size = NULL;
            size = myitoa(getSize(meta_data), size);
            writeWrapper(size);

            getAccessTime(meta_data);

            writeWrapper(fileName);
        } else {
            writeErrorMsg(fileName);
        }
    }


    return 0;
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
char getDirChar(struct stat meta_data) {
    return S_ISDIR(meta_data.st_mode) ? 'd' : '-';
}

//Gets the permission for user, group and others using the bitmasks provided by stat
//Takes a stat struct as a parameter, and destination to store permissions
//BEGIN CITATION: Learned how to use the masks from this page
//- https://stackoverflow.com/questions/8812959/how-to-read-linux-file-permission-programmatically-in-c-c
void getFilePerm(struct stat meta_data, char* filePerm) {
    mode_t perm = meta_data.st_mode;

    filePerm[0] = (S_IRUSR & perm) ? 'r' : '-';
    filePerm[1] = (S_IRUSR & perm) ? 'w' : '-';
    filePerm[2] = (S_IRUSR & perm) ? 'x' : '-';
    filePerm[3] = (S_IRUSR & perm) ? 'r' : '-';
    filePerm[4] = (S_IRUSR & perm) ? 'w' : '-';
    filePerm[5] = (S_IRUSR & perm) ? 'x' : '-';
    filePerm[6] = (S_IRUSR & perm) ? 'r' : '-';
    filePerm[7] = (S_IRUSR & perm) ? 'w' : '-';
    filePerm[8] = (S_IRUSR & perm) ? 'x' : '-';
    filePerm[9] = '\0';
}
//END CITATION

//Wrapper for the 'write' syste call
//Takes a string as an argument and calculates the length using myStrLen
void writeWrapper(char* str) {
    write(WRITE_SYSCALL, str, myStrLen(str));
}

//Returns the number of hard links a file has.
//Takes the file meta data as a parameter.
nlink_t getLinks(struct stat meta_data) {
    return meta_data.st_nlink;
}

uid_t getUID(struct stat meta_data) {
    return meta_data.st_uid;
}

gid_t getGID(struct stat meta_data) {
    return meta_data.st_gid;
}

off_t getSize(struct stat meta_data) {
    return meta_data.st_size;
}

time_t getAccessTime(struct stat meta_data) {
    time_t rawTime = meta_data.st_atim.tv_sec;
    struct tm* time_struct;
    time_struct = localtime(&rawTime);
}

//Converts an integer to a character array that can be output using write().
//Takes an integer as a parameter.
char* myitoa(int num, char* str) {
    char intStr[MAX_INT_DIGITS];
    int i = 0;

    //Gets digits from least to most significant (reverse order in array)
    while (num) {
        intStr[i++] = num % 10;
        num /= 10;
    }
    intStr[i] = '\0';

    char charStr[i + 1];

    //Converts digits to ASCII and reorders in new array
    for (int j = 0, k = i - 1; j < i; j++, k--) {
        charStr[k] = intStr[j] + ASCII_CONVERSION_INT;
    }

    charStr[i] = '\0';
    str = charStr;
    return str;
}
