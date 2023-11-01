#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
        char buff[2] = {'A', '1'};
        char buff2[1];

	for(int i = 0; i<1000; i++){
		int ret = write(fd, buff+(i%2), 1);
		if(ret != 1){
			printf("1.Test case failed\n");
			return -1;
		}

		int ret2 = read(fd, buff2, 1);
		if(ret2 != 1){
			printf("2.Test case failed\n");
			return -1;
		}

		int ret3 = read(fd, buff2, 1);
		if(ret3 != 0){
			printf("3.Test case failed\n");
			return -1;
		}

		if(buff[i%2] != buff2[0]){
			printf("4.Test case failed\n");
                        return -1;
		}
	}
	printf("Test case passed\n");
        return 0;
}
