#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  int x;
  char * mm1 = mmap(NULL, pages, PROT_READ, 0);
  if((long)mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
  // vm_area count should be 1 and page fault should be 0;
  pmap(0);

  int result  = mprotect((void *)mm1, pages, PROT_READ|PROT_WRITE);
  if(result <0)
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }
  pmap(0);

  mm1[0] = 'B';
  if(mm1[0] != 'B')
  {
    printf("Testcase failed\n");
    return 1;
  }
  // vm_area count should be 1 and page fault should be 1;
  pmap(0);

  result  = mprotect((void *)mm1, pages, PROT_READ);
  if(result <0)
  {
    // Testcase failed
     printf("Test case failed \n");
    return 0;
  }

  printf("Reached end of the program\n");

  //should fault
  mm1[0] = 'X';

  return 0;
}
