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

int mystat();
int mywrite();
int myopen();
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
void writeErrorMsg(char* fileName);
void getFilePerm(struct stat meta_data, char* filePerm);
void writeFilePerm(char* filePerm);
int isDir(struct stat meta_data);
void writeIsDir(int isDir);

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
            int dir = isDir(meta_data);
            writeIsDir(file);
            char filePerm[NUM_PERMISSIONS];
            getFilePerm(meta_data, filePerm);
            writeFilePerm(filePerm);

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

//Returns whether a file is a directory or not.
//Takes a stat struct as a parameter
int isDir(struct stat meta_data) {
    return S_ISDIR(meta_data.st_mode);
}

//Writes a 'd' if a file is a directory or a '-' if it is not.
void writeIsDir(int isDir) {
    char id[1];
    id[0] = (isDir) ? 'd' : '-';
    write(WRITE_SYSCALL, id, sizeof(char)); //TODO replace with my write wrapper
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
}
//END CITATION

//Writes the file permissions for the user, group, and others.
//Takes a list of file permission characters as input.
void writeFilePerm(char* filePerm) {
    for (int i = 0; i < NUM_PERMISSIONS; i++) write(WRITE_SYSCALL, filePerm + i, sizeof(char));
}
