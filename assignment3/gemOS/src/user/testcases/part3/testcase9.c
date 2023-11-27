#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  int i;
  int * mm1 =(int *) mmap(NULL, pages*2, PROT_READ|PROT_WRITE,0);  
  if(mm1 < 0)
  {
    printf("Map failed \n");
    return 1;
  }
  
  mm1[10] = 123;
  mm1[2010] = 456;
  
  int result  = mprotect((void *)mm1, pages*2, PROT_READ);
  if(result <0){
      printf("Tescase failed \n");
      return 0;
  }
  pid = cfork();
  if(pid == 0){
      sleep(60);
      mm1[1] = 1;
  }
  else{
      exit(0);
  }
  int val1 =  munmap(mm1, pages*2);
  if(val1 < 0)
  {
     printf("Testcase failed\n");
     return 1;
  }
  return 0;
}

