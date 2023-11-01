#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
        char buff[8096];
        char buff2[8096];

	for(int i = 0, j = 0; i< 4096; i++){
                j = i % 26;
                buff[i] = 'A' + j;
        }

	int ret = write(fd, buff, 8096);
	if(ret != 4096){
		printf("1.Test case failed\n");
		return -1;
	}
	
	int ret2 = read(fd, buff2, 8096);
	if(ret2 != 4096){
		printf("2.Test case failed\n");
		return -1;
	}
	
	for(int i = 0; i<4096; i++){
		if(buff[i] != buff2[i]){
                	printf("3.Test case failed\n");
                	return -1;
		}
	}
	
	printf("Test case passed\n");
	return 0;
}
