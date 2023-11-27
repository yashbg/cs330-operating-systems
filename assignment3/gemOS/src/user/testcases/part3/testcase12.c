#include<ulib.h>

/*mmap and call cfork testcase.*/

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  char x;
  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE,0);
  if(mm1 < 0)
  {
    printf("Map failed \n");
    return 1;
  }
  mm1[0] = 123;
  pid = cfork();
  if(pid){
    sleep(10);
    mm1[0] = 'A';
    printf("Parent mm1[0]:%c\n",mm1[0]);
    long cow_fault = get_cow_fault_stats();
    if(cow_fault == 3)
        printf("Testcase passed\n");
    else
        printf("Testcase failed\n");
  }
  else{
    sleep(60);
  }

  return 0;
}

