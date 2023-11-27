#include<ulib.h>

/*cfork, change mmap area prot of child to read-only , parent writes to mmap area, should cause cow */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  int i;
  char c;
  char* mm1 =(char*) mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
  //allocate physical memory
  mm1[1] = 'a';
  mm1[4096] = 'b';
  mprotect(mm1, pages*2, PROT_READ);
  pid = cfork();
  if(pid){
      mprotect(mm1+pages, pages, PROT_READ|PROT_WRITE);
      mm1[4096] = 'd';
  }
  else{
        sleep(60);
	if(mm1[4096] == 'd'){
		printf("Testcase failed\n");
  		exit(0);    
	}	  
    	printf("Testcase passed\n"); 
  }
    return 0;
}

