#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  char * mm2 = mmap(NULL, pages*3, PROT_READ, 0);
  if((long)mm2 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);
  
  int val1 = munmap((void*)mm2, pages);
  if(val1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  int val2 = mprotect((void*)mm1, pages*5, PROT_READ);
  if(val2 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);
  
  int val3 = munmap((void*)mm1+pages, pages*3);
  if(val3 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(1);

  return 0;
}
  
