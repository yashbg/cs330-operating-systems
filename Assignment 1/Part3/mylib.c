#include <stdio.h>
#include <sys/mman.h>
#include "mylib.h"

struct FreeNode {
	unsigned long size;
	struct FreeNode *next;
	struct FreeNode *prev;
};

void pushNode(struct FreeNode *nodePtr);

struct FreeNode *freeHead = NULL;

void *memalloc(unsigned long size) {
	printf("memalloc() called\n");
	if (!size) {
		return NULL;
	}

	if (!freeHead) {
		// request memory from OS
		unsigned long mmapSize = size;
		if (size % MMAP_MIN_SZ) {
			mmapSize = ((size / MMAP_MIN_SZ) + 1) * MMAP_MIN_SZ;
		}

		void *memPtr = mmap(NULL, mmapSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (memPtr == MAP_FAILED) {
			return NULL;
		}
	}

	return NULL;
}

int memfree(void *ptr) {
	printf("memfree() called\n");
	return 0;
}	
