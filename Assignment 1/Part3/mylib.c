#include <stdio.h>
#include <sys/mman.h>
#include "mylib.h"

struct AllocMdata {
	unsigned long size;
};

struct FreeNode {
	unsigned long size;
	struct FreeNode *next;
	struct FreeNode *prev;
};

void pushNode(struct FreeNode *nodePtr);

struct FreeNode *freeHead = NULL;

struct FreeNode *findFreeNode(unsigned long size);

void *requestMem(unsigned long memSize, unsigned long size) {
	// request memory from OS
	unsigned long mmapSize = memSize;
	if (size % MMAP_MIN_SZ) {
		mmapSize = ((size / MMAP_MIN_SZ) + 1) * MMAP_MIN_SZ;
	}

	void *memPtr = mmap(NULL, mmapSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (memPtr == MAP_FAILED) {
		return NULL;
	}

	// fill metadata of allocated memory
	struct AllocMdata mdata = {size};
	*(struct AllocMdata*)memPtr = mdata;

	// fill metadata of free memory and push it into free list
	void *freePtr = (char*)memPtr + memSize;
	struct FreeNode node = {mmapSize - memSize, NULL, NULL};
	*(struct FreeNode*)freePtr = node;
	pushNode(freePtr);

	return (char*)memPtr + 8;
}

void *memalloc(unsigned long size) {
	printf("memalloc() called\n");
	
	if (!size) {
		return NULL;
	}

	unsigned long memSize = 8 + size;
	if (memSize % 8) {
		// eight-bytes alignment padding
		unsigned long ebaPadding = 8 - memSize % 8;
		memSize += ebaPadding;
	}
	if (memSize < 24) {
		// memSize should be atleast 24
		memSize = 24;
	}

	if (!freeHead) {
		return requestMem(memSize, size);
	}

	struct FreeNode *freePtr = findFreeNode(size);
	if (!freePtr) {
		return requestMem(memSize, size);
	}
	
	return NULL;
}

int memfree(void *ptr) {
	printf("memfree() called\n");
	return 0;
}	
