#include<ulib.h>

/*Check one physical page irrespective of length*/

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  int x;
  char * mm1 = mmap(NULL, 2, PROT_READ|PROT_WRITE, 0);
  if((long)mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
  mm1[0] = 'B';
  if(mm1[0] != 'B')
  {
    printf("Testcase failed\n");
    return 1;
  }
  // vm_area count should be 1 and page fault should be 1;
  pmap(0);

  mm1[4095] = 'D';
  if(mm1[4095] != 'D')
  {
      printf("Testcase failed\n");
      return 1;

  }

  printf("Reached end of the program\n");

  //Should result in seg fault;
  mm1[4096] = 'D';
  return 0;
}
