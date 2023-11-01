#include<ulib.h>

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

        int fd = create_trace_buffer(O_RDWR);
        char buff[4096];
        char buff2[4096];

        for(int i = 0, j = 0; i< 100; i++){
                j = i % 26;
                buff[i] = 'A' + j;
        }

        int ret = write(fd, buff, 100);
        if(ret != 100){
                printf("1.Test case failed\n");
                return -1;
        }

        int ret2 = read(fd, buff2, 100);
        if(ret2 != 100){
                printf("2.Test case failed\n");
                return -1;
        }
        close(fd);

        int ret3 = write(fd, buff, 100);
        if(ret3 > 0){
                printf("3.Test case failed\n");
                return -1;
        }
        printf("Test case passed\n");
        return 0;
}
