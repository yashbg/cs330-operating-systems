#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

	int fd = create_trace_buffer(O_READ);
	if(fd == 3){
		printf("Test case passed\n");
	}
	else{
		printf("Test case failed\n");
		return -1;
	}
	close(fd);
	return 0;
}
