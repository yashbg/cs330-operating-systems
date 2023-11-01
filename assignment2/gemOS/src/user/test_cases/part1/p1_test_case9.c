#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd[14];
	for(int i = 0; i<14; i++){
		fd[i] = create_trace_buffer(O_RDWR);
	}
	if(fd[13] > 0){
		printf("1.Test case failed\n");
        	return -1;
	}

        close(fd[0]);

	int fd2 = create_trace_buffer(O_RDWR);
	if(fd2 != 3){
                printf("2.Test case failed\n");
                return -1;
        }
        printf("Test case passed\n");
        return 0;
}
