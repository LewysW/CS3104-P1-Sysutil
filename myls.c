#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1

int main(int argc, char** argv)
{
    char* text = "Hello world!\n";    // String to print
    size_t len = 14;     // Length of our string, which we need to pass to write syscall
    long handle = 1;     // 1 for stdout, 2 for stderr, file handle from open() for files
    long ret = -1;       // Return value received from the system call

    ret = write(handle, text, len);
    return ret;
}
