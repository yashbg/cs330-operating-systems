#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
        char buff[100];
        char buff2[100];

        for(int i = 0, j = 0; i< 100; i++){
		j = i % 26;
                buff[i] = 'A' + j;
        }

        int ret = write(fd, buff, 100);
//	printf("ret value from write: %d\n", ret);
        if(ret != 100){
                printf("1.Test case failed\n");
                return -1;
        }

        int ret2 = read(fd, buff2, 100);
        if(ret2 != 100){
                printf("2.Test case failed\n");
                return -1;
        }

	int ret3 = write(fd, buff, 0);
        if(ret3 != 0){
                printf("1.Test case failed\n");
                return -1;
        }

        int ret4 = read(fd, buff2, 100);
        if(ret4 != 0){
                printf("2.Test case failed\n");
                return -1;
        }
        
	int ret5 = ustrncmp(buff, buff2, 100);
        if(ret5 != 0){
                printf("3.Test case failed\n");
                return -1;       
        }
        
	close(fd);
        printf("Test case passed\n");
        return 0;
}
