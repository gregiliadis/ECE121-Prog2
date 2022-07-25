#ifndef __UTIL_H__
#define __UTIL_H__
#include <unistd.h>
ssize_t myWrite (int fd, const void *buff, size_t count,int line,char *file);
ssize_t myRead (int fd, const void *buff, size_t count,int line, char *file);

#endif
