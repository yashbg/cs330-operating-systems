#include<ulib.h>

/*cfork, change mmap area prot of child to read-only , parent writes to mmap area, should cause cow */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  int i;
  int val1;
  unsigned long * mm1 =(unsigned long *) mmap(NULL, (pages*4), PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }

  //physical mem allocation
  mm1[0] = 1;
  mm1[512] = 2;
  mm1[1024] = 3;
  mm1[1536] = 4;

  pid = cfork();
  if(pid){
      sleep(60);
      mm1[10] = 100;
      mm1[800] = 123;
      mm1[1500] = 456;
      if(mm1[10] == 100 && mm1[800] == 123 && mm1[1500] == 456)
          printf("Testcase passed\n");
      else
          printf("Testcase failed\n");
     
  }
  else{
      
      int result  = mprotect((void *)mm1, pages*4, PROT_READ);
      if(result <0)
      {
           printf("Testcase failed1 \n");
           return 0;
      }
      
      val1 =  munmap((void *)mm1, pages);
      if(val1 < 0)
      {
          printf("Testcase failed\n");
          return 1;
      }

      sleep(120);
      printf("Child exiting\n");

      exit(0);
  }
  
    val1 =  munmap(mm1, pages*4);
    if(val1 < 0)
    {
        printf("Testcase failed\n");
        return 1;
    }

    return 0;
}

