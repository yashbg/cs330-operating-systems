#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  // vm_area will be created without physical pages.
  char * lazy_alloc = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)lazy_alloc < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  pmap(0);
  //printf("%d\n",get_user_page_stats());

  for(int i = 0; i<50; i++)
  {
	  char* page_read = lazy_alloc + (i*pages);
	  char temp = page_read[0];
  }
  pmap(0);
  //printf("%d\n",get_user_page_stats());

  int result  = mprotect((void *)lazy_alloc, pages*25, PROT_READ);
  if(result <0)
  {
     printf("Test case failed \n");
    return 0;
  }
  pmap(0);
  //printf("%d\n",get_user_page_stats());

  result  = munmap((void *)lazy_alloc, pages*25);
  if(result <0)
  {
     printf("Test case failed \n");
    return 0;
  }
  pmap(0);
  //printf("%d\n",get_user_page_stats());

  printf("Reached end of the program\n");
  return 0;
}


