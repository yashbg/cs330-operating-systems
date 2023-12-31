#include<ulib.h>

static int func1(int a)
{
   if(a == 1)
   {
	return 0;
   }
   func1(a-1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////// Do not modify this test case. Otherwise, backtrace addresses will get affected ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
    int ret = 0;
    int ftrace_fd = 0;
    u64 ftrace_buff[512];

    ftrace_fd = create_trace_buffer(O_RDWR);
    if(ftrace_fd != 3)
    {
	printf("0. Test case failed\n");
	return -1;
    }
    
    ret = ftrace((unsigned long)&func1, ADD_FTRACE, 1, ftrace_fd);
    if(ret != 0)
    { 
	printf("1. Test case failed\n");
	return -1;
    }

    ret = ftrace((unsigned long)&func1, ENABLE_FTRACE, 0, ftrace_fd);
    if(ret != 0)
    { 
	printf("2. Test case failed\n");
	return -1;
    }

    ret = ftrace((unsigned long)&func1, ENABLE_BACKTRACE, 0, ftrace_fd);
    if(ret != 0)
    { 
	printf("3. Test case failed\n");
	return -1;
    }

    func1(3);

    int read_ret = read_ftrace(ftrace_fd, ftrace_buff, 4);
    /*for(int i = 0; i<(read_ret/8); i++){
	printf("ftrace_buff[%d] : %x\n", i*8, ftrace_buff[i]);
    }
    */

    if(read_ret != 120)
    { 
	printf("4. Test case failed\n");
	return -1;
    }

    if((u64*)(((u64*)ftrace_buff)[0]) != (u64*)(&func1))
    { 
	printf("5. Test case failed\n");
	return -1;
    }

    if(((u64*)ftrace_buff)[1] != 0x3)
    { 
	printf("6. Test case failed\n");
	return -1;
    }

    if((u64*)(((u64*)ftrace_buff)[3]) != (u64*)(0x100000F51))
    { 
	printf("7. Test case failed\n");
	return -1;
    }

    if((u64*)(((u64*)ftrace_buff)[4]) != (u64*)(&func1))
    { 
	printf("8. Test case failed\n");
	return -1;
    }

    if(((u64*)ftrace_buff)[5] != 0x2)
    { 
	printf("9. Test case failed\n");
	return -1;
    }

    if((u64*)(((u64*)ftrace_buff)[7]) != (u64*)(0x100000E15))
    { 
	printf("10. Test case failed\n");
	return -1;
    }
    
    ret = ftrace((unsigned long)&func1, DISABLE_BACKTRACE, 0, ftrace_fd);
    if(ret != 0)
    {
        printf("11. Test case failed\n");
        return -1;
    }

    ret = ftrace((unsigned long)&func1, DISABLE_FTRACE, 0, ftrace_fd);
    if(ret != 0)
    {
        printf("12. Test case failed\n");
        return -1;
    }

    ret = ftrace((unsigned long)&func1, REMOVE_FTRACE, 0, ftrace_fd);
    if(ret != 0)
    {
        printf("13. Test case failed\n");
        return -1;
    }
    printf("Test case passed\n");

	
    return 0;
}
  
