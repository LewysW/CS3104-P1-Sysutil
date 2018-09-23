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
#define SINGLE_DIGIT 9
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
char getDirChar(struct stat meta_data);
nlink_t getLinks(struct stat meta_data);
char* myitoa(int num, char* str);
void writeWrapper(char* str);
uid_t getUID(struct stat meta_data);
gid_t getGID(struct stat meta_data);
off_t getSize(struct stat meta_data);
time_t getAccessTime(struct stat meta_data);
char* monthToStr(int month, char* monthStr);
char* formatDateTime(char* month, char* day, char* hour, char* min);
char* metaDataToString(struct stat meta_data);

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
            metaDataToString(meta_data);
        } else {
            writeErrorMsg(fileName);
        }
    }


    return 0;
}

char* metaDataToString(struct stat meta_data) {
    char dirChar[2];
    char filePerm[NUM_PERMISSIONS + 1];
    char* links = NULL;
    char* uid = NULL;
    char* gid = NULL;
    char* size = NULL;
    char* accessTime = NULL;

    dirChar[0] = getDirChar(meta_data);
    dirChar[1] = '\0';

    getFilePerm(meta_data, filePerm);

    links = myitoa(getLinks(meta_data), links);

    uid = myitoa(getUID(meta_data), uid);

    gid = myitoa(getUID(meta_data), gid);

    size = myitoa(getSize(meta_data), size);

    accessTime = getAccessTime(meta_data);

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

//TODO - change time printed to correct time - ALWAYS PRINTS CURRENT TIME
time_t getAccessTime(struct stat meta_data) {
    struct tm* timeStruct;
    char month[MONTH_LENGTH + 1];
    char day[DAY_LENGTH + 1];
    char hour[HOUR_LENGTH + 1];
    char min[MINUTE_LENGTH + 1];

    time(&meta_data.st_ctime);

    timeStruct = localtime(&meta_data.st_ctime);

    //Gets month string by index and copies to month variable.
    myStrCpy(month, monthToStr(timeStruct->tm_mon, month), MONTH_LENGTH);

    //Tests whether the day is a single or double digit and then copies the correct number of bytes
    myStrCpy(day, myitoa(timeStruct->tm_mday, day), myitoa(timeStruct->tm_mday, day) > SINGLE_DIGIT ? DAY_LENGTH : DAY_LENGTH - 1);

    //Converts hours to string and copies this value
    myStrCpy(hour, myitoa(timeStruct->tm_hour, hour), HOUR_LENGTH);
    if (myStrLen(hour) == 1) {
        hour[1] = hour[0];
        hour[0] = '0';
        hour[2] = '\0';
    }

    //Converts minutes to string and copies this value
    myStrCpy(min, myitoa(timeStruct->tm_min, min), MINUTE_LENGTH);
    if (myStrLen(min) == 1) {
        min[1] = min[0];
        min[0] = '0';
        min[2] = '\0';
    }

    return (formatDateTime(month, day, hour, min));

}

//Formats the date and time with spaces and a colon.
//Takes the strings month, day, hour and min as parameters.
char* formatDateTime(char* month, char* day, char* hour, char* min) {
    int size = myStrLen(month) + myStrLen(day) + myStrLen(hour) + myStrLen(min) + 3;
    char dateTime[size];
    int index = 0;
    myStrCpy(dateTime, month, myStrLen(month));
    index += myStrLen(month);
    myStrCpy(dateTime + index, " ", sizeof(char));
    index++;
    myStrCpy(dateTime + index, day, myStrLen(day));
    index += myStrLen(day);
    myStrCpy(dateTime + index, " ", sizeof(char));
    index++;
    myStrCpy(dateTime + index, hour, myStrLen(hour));
    index += myStrLen(hour);
    myStrCpy(dateTime + index, ":", sizeof(char));
    index++;
    myStrCpy(dateTime + index, min, myStrLen(min));
    index += myStrLen(min);
    dateTime[index] = '\0';

    return dateTime;
}

char* monthToStr(int month, char* monthStr) {
    myStrCpy(monthStr, MONTH_STRING[month], MONTH_LENGTH + 1);
    return monthStr;
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
