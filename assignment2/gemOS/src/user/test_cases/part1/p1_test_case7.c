#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
	if(fd != 3){
		printf("1. Test case failed\n");
		return -1;
	}

        int fd2 = create_trace_buffer(O_RDWR);
	if(fd2 != 4){
                printf("2. Test case failed\n");
		return -1;
        }

        char buff[4096];
        char buff2[4096];
        char buff3[4096];
        char buff4[4096];

        for(int i = 0; i< 4096; i++){
                buff[i] = 'A';
        }

	for(int i = 0; i< 4096; i++){
                buff2[i] = 'B';
        }

        int ret = write(fd, buff, 4096);
        if(ret != 4096){
                printf("3.Test case failed\n");
                return -1;
        }

        int ret2 = read(fd, buff3, 4096);
        if(ret2 != 4096){
                printf("4.Test case failed\n");
                return -1;
        }
        int ret3 = write(fd2, buff2, 4096);
        if(ret3 != 4096){
                printf("5.Test case failed\n");
                return -1;
        }

        int ret4 = read(fd2, buff4, 4096);
        if(ret4 != 4096){
                printf("6.Test case failed\n");
                return -1;
        }
        int ret5 = ustrncmp(buff, buff3, 4096);
        if(ret5 != 0){
                printf("7.Test case failed\n");
                return -1;
	}
	int ret6 = ustrncmp(buff2, buff4, 4096);
        if(ret6 != 0){
                printf("8.Test case failed\n");
                return 0;
        }
        close(fd);
        close(fd2);
        printf("Test case passed\n");
        return 0;
}
