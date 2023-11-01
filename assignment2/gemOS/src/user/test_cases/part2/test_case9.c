#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int strace_fd = create_trace_buffer(O_RDWR);
	if(strace_fd != 3){
		printf("1.Test case failed\n");
		return -1;
	}

        int rdwr_fd = create_trace_buffer(O_RDWR);
	if(rdwr_fd != 4){
		printf("2.Test case failed\n");
		return -1;
	}

        u64 strace_buff[4096];
        int read_buff[4096];
	int i = 0;
	int address = 0;
	while(i<500){
		start_strace(strace_fd, FULL_TRACING);
        	int read_ret = read(rdwr_fd, read_buff, 10);
		if(read_ret != 0){
			printf("3.Test case failed\n");
			return -1;
		}

		int strace_ret = read_strace(strace_fd, strace_buff, 3);
		if(strace_ret != 64){
                	printf("4.Test case failed\n");
                	return -1;
        	}
		if(strace_buff[0] != 24){
			printf("5. Test case failed\n");
			return -1;
		}
		if(strace_buff[2] != (u64)&read_buff){
                        printf("6. Test case failed\n");
                        return -1;
                }
		end_strace();

		i++;
	}
	if(i != 500){
		printf("7.Test case failed\n");
                return -1;
	}

        close(rdwr_fd);
        close(strace_fd);

        printf("Test case passed\n");
        return 0;
}
