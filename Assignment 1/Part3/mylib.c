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

struct FreeNode *freeHead = NULL;

void pushNode(struct FreeNode *nodePtr) {
	nodePtr->next = freeHead;

	if (freeHead) {
		freeHead->prev = nodePtr;
	}

	freeHead = nodePtr;
}

void deleteNode(struct FreeNode *nodePtr) {
	if (nodePtr == freeHead) {
		freeHead = nodePtr->next;
	}

	if (nodePtr->next) {
		nodePtr->next->prev = nodePtr->prev;
	}

	if (nodePtr->prev) {
		nodePtr->prev->next = nodePtr->next;
	}
}

struct FreeNode *findFirstFit(unsigned long memSize) {
	struct FreeNode *nodePtr = freeHead;
	while (nodePtr) {
		if (nodePtr->size >= memSize) {
			return nodePtr;
		}

		nodePtr = nodePtr->next;
	}

	return NULL;
}

struct FreeNode *findLeftFreeNode(struct AllocMdata *memPtr) {
	struct FreeNode *curPtr = freeHead;
	while (curPtr) {
		if ((char*)curPtr + curPtr->size == (char*)memPtr) {
			return curPtr;
		}

		curPtr = curPtr->next;
	}

	return NULL;
}

struct FreeNode *findRightFreeNode(struct AllocMdata *memPtr) {
	struct FreeNode *curPtr = freeHead;
	while (curPtr) {
		if ((char*)memPtr + memPtr->size == (char*)curPtr) {
			return curPtr;
		}

		curPtr = curPtr->next;
	}

	return NULL;
}

void fillAllocMdata(struct AllocMdata *memPtr, unsigned long memSize) {
	struct AllocMdata mdata = {memSize};
	*memPtr = mdata;
}

void fillFreeMdata(struct FreeNode *nodePtr, unsigned long freeSize) {
	struct FreeNode node = {freeSize, NULL, NULL};
	*nodePtr = node;
}

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

	fillAllocMdata(memPtr, memSize);

	void *freePtr = (char*)memPtr + memSize;
	fillFreeMdata(freePtr, mmapSize - memSize);
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

	struct FreeNode *freePtr = findFirstFit(memSize);
	if (!freePtr) {
		return requestMem(memSize, size);
	}

	deleteNode(freePtr);

	if (freePtr->size - memSize < 24) {
		fillAllocMdata((struct AllocMdata*)freePtr, freePtr->size);
		return (char*)freePtr + 8;
	}

	fillAllocMdata((struct AllocMdata*)freePtr, memSize);

	void *newFreePtr = (char*)freePtr + memSize;
	fillFreeMdata(newFreePtr, freePtr->size - memSize);
	pushNode(newFreePtr);
	
	return (char*)freePtr + 8;
}

int memfree(void *ptr) {
	printf("memfree() called\n");

	if (!ptr) {
		return -1;
	}

	void *mdataPtr = (char*)ptr - 8;
	struct AllocMdata *memPtr = (struct AllocMdata*)mdataPtr;
	struct FreeNode *nodePtr = (struct FreeNode*)mdataPtr;

	struct FreeNode *leftNodePtr = findLeftFreeNode(memPtr);
	struct FreeNode *rightNodePtr = findRightFreeNode(memPtr);

	if (!leftNodePtr && !rightNodePtr) {
		// contiguous memory chunks on the left side and the right side are allocated
		fillFreeMdata(nodePtr, memPtr->size);
		pushNode(nodePtr);

		return 0;
	}

	if (!leftNodePtr && rightNodePtr) {
		// contiguous memory chunk on the right side is free
		deleteNode(rightNodePtr);
		fillFreeMdata(nodePtr, memPtr->size + rightNodePtr->size);
		pushNode(nodePtr);

		return 0;
	}

	if (leftNodePtr && !rightNodePtr) {
		// contiguous memory chunk on the left side is free
		deleteNode(leftNodePtr);
		fillFreeMdata(leftNodePtr, leftNodePtr->size + memPtr->size);
		pushNode(leftNodePtr);

		return 0;
	}

	// contiguous memory chunks on the both left side and the right side are free
	deleteNode(leftNodePtr);
	deleteNode(rightNodePtr);
	fillFreeMdata(leftNodePtr, leftNodePtr->size + memPtr->size + rightNodePtr->size);
	pushNode(leftNodePtr);
	
	return 0;
}
