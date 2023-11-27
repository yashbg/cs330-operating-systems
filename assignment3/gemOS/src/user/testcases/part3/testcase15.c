#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  int i;
  char * mm1 =(char *) mmap(NULL, pages*2, PROT_READ,0);
  if(mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
 
  pid = cfork();
  if(pid){
      sleep(60);
  }
  else{
      char* mm2 = (char*) mmap(NULL, pages*2, PROT_READ,0);
      pid = cfork();
      if(pid){
      
	      u64 user_pages_before;
	      u64 user_pages_after;
	      int result  = mprotect((void *)mm1, pages*2, PROT_READ|PROT_WRITE);
	      int result1  = mprotect((void *)mm2, pages*2, PROT_READ|PROT_WRITE);
	      if(result <0)
	      {
		   printf("Testcase failed\n");
		   return 0;
	      }
	      user_pages_before =get_user_page_stats();
	      mm1[1000] = 'A';
	      mm2[2000] = 'B';
	      user_pages_after =get_user_page_stats();
	      if((user_pages_after-user_pages_before) >= 2){
		printf("Testcase Passed\n");
	      }
	      else{
		printf("Testcase failed\n");
	      }
      }
      else{
	      sleep(60);
      }
  }
  int val1 =  munmap(mm1, pages*2);
  if(val1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
 
  return 0;
}

