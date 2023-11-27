#include<ulib.h>

/*parent mmap and call cfork,
then parent and child writes.
first write causes copy-on-write and increase user_region_pages stats
but second write only changes PTE entry */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  long cow_fault;
  char * mm1 = mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Testcase failed \n");
    return 1;
  }
  mm1[0] = 'A';
  mm1[1] = 'A';
  pid = cfork();
  if(pid){
      sleep(60);
      long user_pages_before = get_user_page_stats();
      mm1[1] = 'B';
      long user_pages_after = get_user_page_stats();
      if(user_pages_before == user_pages_after)
          printf("user pages testcase passed\n");
      else
          printf("user pages testcase failed\n");
  }
  else{
      printf("Child mm1[0]:%c\n",mm1[0]);
      mm1[1] = 'B';
      printf("Child mm1[1]:%c\n",mm1[1]);

      exit(0);
  }
  int val1 = munmap(mm1, pages*2);
  if(val1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
  
  return 0;
}

