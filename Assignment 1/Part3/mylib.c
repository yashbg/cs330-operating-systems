#include <stdio.h>

struct Node {
	unsigned long size;
	struct Node *next;
	struct Node *prev;
};

struct Node *freeHead = NULL;

void *memalloc(unsigned long size) {
	printf("memalloc() called\n");
	return NULL;
}

int memfree(void *ptr) {
	printf("memfree() called\n");
	return 0;
}	
