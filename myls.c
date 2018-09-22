#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

#include<stdio.h> //TODO - REMOVE THIS IMPORT STATEMENT AND ALL STDLIB FUNCTIONS

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1

int mystat();
int mywire();
int myopen();
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);

int main(int argc, char** argv)
{
    //If file specified, get the file name
    if (argc == 2) {
        int size = myStrLen(argv[1]);
        char fileName[size];
        myStrCpy(fileName, argv[1], size);
    }
    return 0;
}

int myStrLen(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++);
    return i;
}

void myStrCpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}
