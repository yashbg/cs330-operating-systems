#include<ulib.h>

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;
  long user_pages = 0;

  // vm_area will be created without physical pages.
  char * lazy_alloc = mmap(NULL, pages*50, PROT_READ|PROT_WRITE, 0);
  if((long)lazy_alloc < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  //user_pages = get_user_page_stats();
  //printf("user_pages: %d\n", user_pages);
  for(int i = 0; i<25; i++)
  {
    lazy_alloc[(pages * i)] = 'X';
  }
  pmap(0);
  //user_pages = get_user_page_stats();
  //printf("user_pages: %d\n", user_pages);

  for(int i = 25; i<50; i++)
  {
    char* page_read_addr = lazy_alloc+(i*pages);
    char page_read = page_read_addr[0];
  }
  pmap(0);
  user_pages = get_user_page_stats();
  printf("user_pages: %d\n", user_pages);
  if(user_pages != 60)
  {
    printf("Test case failed \n");
    return 1;
  }

  int val1 = munmap((void*)lazy_alloc, pages*50);
  if(val1 < 0)
  {
    printf("Test case failed \n");
    return 1;
  }
  //user_pages = get_user_page_stats();
  //printf("user_pages: %d\n", user_pages);

  printf("Reached end of the program\n");

  return 0;
}
