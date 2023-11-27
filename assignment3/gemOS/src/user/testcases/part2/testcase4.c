#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  // vm_area will be created without physical pages.
  char * lazy_alloc = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)lazy_alloc < 0)
  {
    // Testcase failed.
    printf("Test case failed \n");
    return 1;
  }

  // All access should result in page fault.
  for(int i = 0; i<50; i++)
  {
    lazy_alloc[(pages * i)] = 'X';
  }
  // Number of MMAP_Page_Faults should be 50 & 
  // Number of vm_area should 1
  pmap(1);

  //changing the protection should create a new VMA 
  int result  = mprotect((void *)lazy_alloc, pages*25, PROT_READ);
  if(result <0)
  {
     printf("Test case failed \n");
    return 0;
  }
  pmap(1);
  
  //unmapping should delete VMA as well as the TLB
  int val1 = munmap((void*)lazy_alloc, pages*25);
  if(val1 < 0)
  {
    printf("Test case failed \n");
    return 0;
  }
  pmap(1);

  printf("Reached end of the program\n");

  //should fault
  char c = lazy_alloc[0];
  return 0;
}
