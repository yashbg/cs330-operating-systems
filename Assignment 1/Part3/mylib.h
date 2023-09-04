#ifndef MYLIB_H
#define MYLIB_H

#define MMAP_MIN_SZ 4 * 1024 * 1024

void *memalloc(unsigned long size);
int memfree(void *ptr);

#endif 
