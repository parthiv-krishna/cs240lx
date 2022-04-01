#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read in file <name>
// returns:
//  - pointer to the code.  pad code with 0s up to the
//    next multiple of 4.  
//  - bytes of code in <size>
//
// fatal error open/read of <name> fails.
// 
// How: 
//    - use stat to get the size of the file.
//    - round up to a multiple of 4.
//    - allocate a buffer  --- 
//    - zero pads to a // multiple of 4.
//    - read entire file into buffer.  
//    - make sure any padding bytes have zeros.
//    - return it.   
//
// make sure to close the file descriptor (this will
// matter for later labs).
void *read_file(unsigned *size, const char *name) {
    struct stat statbuf;
    if (stat(name, &statbuf) < 0) {
        sys_die(stat, "read_file failed to stat file");
    }

    *size = statbuf.st_size;
    // round up to multiple of 4
    off_t roundup = (*size + 3) & ~0b111;

    void *data = calloc(roundup, 1);
    if (!data) {
        sys_die(calloc, "read_file failed to calloc buffer");
    }

    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        sys_die(open, "read_file failed open file");
    }
    if (read(fd, data, *size) < 0) {
        sys_die(read, "read_file failed to read file");
    }
    
    if (close(fd) < 0) {
        sys_die(close, "read_file failed to close file");
    }
    return data;
}
