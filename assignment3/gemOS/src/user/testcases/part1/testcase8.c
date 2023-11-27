#include<ulib.h>

void* util_mmap(void * addr, int length, int prot, int flag)
{
  char* temp = mmap(addr, length, prot, flag);
  if((long)temp <= 0)
  {
    printf("Testcase Failed \n");
    exit(0);
  }
  return temp;
}

int util_munmap(void * addr, int length)
{
   int temp = munmap(addr, length);
   if(temp < 0)
   {
     printf("Testcase Failed \n");
     exit(0);
   }
   return temp;
}
int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pages = 4096;

  // Doing MMAP
  char * mm1= util_mmap(NULL, (pages*100), PROT_READ|PROT_WRITE, 0);
  if((long)mm1 <= 0)
  {
    printf("Testcase Failed \n");
    exit(0);
  }

  // vm_area count should be 1
  pmap(1);
  
  // Doing mmap for the same address using MAP_FIXED 
  // MMAP should return -1
  char* mm2 = mmap(mm1, pages*100, PROT_READ|PROT_WRITE, MAP_FIXED);
  if((long)mm2 != -1){
  	printf("Testcase Failed \n");
	printf("mm2: %x\n", mm2);
	exit(0);
  }
  // Allocating at hint address which is already occupied
  mm2 = util_mmap(mm1, (pages*100), PROT_READ|PROT_WRITE, 0);
  //mm2 should be the next address after mm1's VMA
  if(mm2 != mm1+(pages*100)){
  	printf("Testcase Failed \n");
        exit(0);
  }

  // vm_area count should be 1
  pmap(1);

  return 0;
}
