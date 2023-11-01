#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
	char buff[10];

	for(int i = 0; i< 10; i++){
		buff[i] = 'A' + i;
	}

	int ret = write(fd, buff, 10);
	if(ret != 10)
	{
		printf("1.Test case failed\n");
		return -1;
	}

	ret = read(fd, buff - 8192, 5);
	if(ret != 5){
		printf("2.Test case failed\n");
		return -1;
	}

	ret = read(fd, buff + 81920, 5);
	if(ret != -EBADMEM){
		printf("3.Test case failed\n");
		return -1;
	}

	printf("Test case passed\n");

	return 0;
}
