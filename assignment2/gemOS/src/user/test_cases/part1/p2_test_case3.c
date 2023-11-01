#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);

	int ret = write(fd, &main, 10);
	if(ret != 10){
		printf("1.Test case failed\n");
		return -1;
	}

	ret = write(fd, &main + 0xA000, 10);
	if(ret != -EBADMEM){
		printf("2.Test case failed\n");
		return -1;
	}

	printf("Test case passed\n");

	return 0;

}
