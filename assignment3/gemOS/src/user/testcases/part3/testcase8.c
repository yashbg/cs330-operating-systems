#include<ulib.h>

/*cfork, change mmap area prot of child to read-only , parent writes to mmap area, should cause cow */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int pages = 4096;
  int i;
  int * mm1 =(int *) mmap(NULL, pages*2, PROT_READ|PROT_WRITE, 0);
  if(mm1 < 0)
  {
    printf("Testcase failed\n");
    return 1;
  }
  //allocate physical memory
  mm1[1] = 'a';
  mm1[1100] = 'b';

  pid = cfork();
  if(pid){
      sleep(60);
      long cow_f_before = get_cow_fault_stats();
      mm1[1000] = 123;
      mm1[2000] = 456;
      //pmap(1);
      long cow_f_after = get_cow_fault_stats();
      if((cow_f_after-cow_f_before)==2){
          printf("Testcase passed\n");
      }
      else{
          printf("Testcase failed\n");
      }

  }
  else{
      
      int * mm2 =(int *) mmap(NULL, pages*2, PROT_READ|PROT_WRITE,0);
      int result  = mprotect((void *)mm1, pages*2, PROT_READ);
      if(result <0)
      {
           printf("Testcase failed \n");
           return 0;
       }
      //pmap(1);
  }
  
    int val1 =  munmap(mm1, pages*2);
    if(val1 < 0)
    {
        printf("Testcase failed\n");
        return 1;
    }
    
    return 0;
}

