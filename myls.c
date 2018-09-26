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
#define MONTH_LENGTH 3
#define DAY_LENGTH 2
#define HOUR_LENGTH 2
#define MINUTE_LENGTH 2
#define YEAR_LENGTH 4
#define STARTING_YEAR 1900
#define SINGLE_DIGIT 9
#define TIME_LENGTH 5

static const char *MONTH_STRING[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int mystat();
int mywrite();
int myopen();
int myStrLen(char* str);
void myStrCpy(char* dest, const char* src, size_t n);
void writeErrorMsg(char* fileName);
void getFilePerm(struct stat meta_data, char* filePerm);
void getDirChar(struct stat meta_data, char* dir);
nlink_t getLinks(struct stat meta_data);
void myitoa(int num, char* str);
void writeWrapper(char* str);
uid_t getUID(struct stat meta_data);
gid_t getGID(struct stat meta_data);
off_t getSize(struct stat meta_data);
void printAccessTime(struct stat meta_data);
char* monthToStr(int month, char* monthStr);
void printMetaData(struct stat meta_data);

int main(int argc, char** argv)
{
    struct stat meta_data;

    //If file specified, get the file name
    if (argc == 2) {
        int size = myStrLen(argv[1]);
        char fileName[size];
        myStrCpy(fileName, argv[1], size);
        int status = stat(fileName, &meta_data);

        //If stat returned successfully then get convert meta data to string and write,
        //otherwise write error message.
        if (!status) {
            printMetaData(meta_data);
        } else {
            writeErrorMsg(fileName);
        }
    }


    return 0;
}

void printMetaData(struct stat meta_data) {
    char tempStr[MAX_INT_DIGITS];

    getDirChar(meta_data, tempStr);
    writeWrapper(tempStr);
    getFilePerm(meta_data, tempStr);
    writeWrapper(tempStr);

    writeWrapper(" ");
    myitoa(meta_data.st_nlink, tempStr);
    writeWrapper(tempStr);
    writeWrapper(" ");
    myitoa(meta_data.st_uid, tempStr);
    writeWrapper(tempStr);
    writeWrapper(" ");
    myitoa(meta_data.st_gid, tempStr);
    writeWrapper(tempStr);
    writeWrapper(" ");
    myitoa(meta_data.st_size, tempStr);
    writeWrapper(tempStr);
    writeWrapper(" ");
    printAccessTime(meta_data);
    writeWrapper("\n");
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

//Wrapper for the 'write' syste call
//Takes a string as an argument and calculates the length using myStrLen
void writeWrapper(char* str) {
    write(WRITE_SYSCALL, str, myStrLen(str));
}

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
    writeWrapper(monthToStr(fileTime->tm_mon, tempStr));

    writeWrapper(" ");

    //Gets day and prints it to terminal
    myitoa(fileTime->tm_mday, tempStr);
    writeWrapper(tempStr);

    writeWrapper(" ");

    if (fileYear == currentYear) {
        //Sets initial hour value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';
        tempStr[2] = '\0';

        //Decides how many hour digits to copy based on size of hour time.
        //Then prints hour time and a ':'
        myitoa(fileTime->tm_hour, (fileTime->tm_hour > SINGLE_DIGIT ? tempStr : tempStr + 1));
        writeWrapper(tempStr);
        writeWrapper(":");

        //Sets initial minute value to 00
        tempStr[0] = '0';
        tempStr[1] = '0';

        //Decides how many minute digits to copy based on size of minute time.
        //Then prints minute time
        myitoa(fileTime->tm_min, (fileTime->tm_min > SINGLE_DIGIT ? tempStr : tempStr + 1));
        writeWrapper(tempStr);
    } else {
        myitoa(fileYear, tempStr);
        writeWrapper(tempStr);
    }

}

char* monthToStr(int month, char* monthStr) {
    myStrCpy(monthStr, MONTH_STRING[month], MONTH_LENGTH + 1);
    return monthStr;
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
