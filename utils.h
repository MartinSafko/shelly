#include <stdlib.h>
#include <fcntl.h>
#include <err.h>

static inline void* safe_malloc(size_t size)
{
    void* ret = malloc(size);
    if (ret == NULL)
        errx(1, "Out of memory");
    
    return ret;
}

static inline int safe_open(const char* pathname, int flags, mode_t mode)
{
    int fd = open(pathname, flags, mode);
    if (fd == -1)
        err(1, "Failed to open file, %s", pathname);

    return fd;
}

