#include<ulib.h>

void* util_mmap(void * addr, int length, int prot, int flag)
{
  char* temp = mmap(addr, length, prot, 0);
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

  char * mm1 = 0;

  // Creating vm_area;
  int total = 70;
  for(int i =0; i < total; i++)
  {
      int val = i % 2;
      int prot = -1;
      if(val == 0)
        prot = PROT_READ|PROT_WRITE;
      else if(val == 1)
        prot = PROT_READ;
      char* temp = util_mmap(NULL, pages*100, prot, 0);
      if(!mm1)
      {
        mm1 = temp;
      }
  }
  // Vm_area count should be  70;
   pmap(0);

  // Shrinking by unmapping at middle of VMAs;
  char* temp = mm1;
  for(int i =1; i <= total; i++)
  {
        
      char* vm_area_middle = temp+(40 * pages);
      temp += (100*pages);
      util_munmap(vm_area_middle, pages*20);
  }
  // Vm_area count should be  140;
  pmap(0);
  

  //mapping the above gaps with read protect VMAs
  for(int i = 1; i<=total; i++){
  	
	util_mmap(NULL, pages*20, PROT_READ, 0);
  }
  //vm_area count should be 140
  pmap(1);
  return 0;

}
