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
  int total = 126;
  char * mm1 = 0;

  // Creating vm_area;
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
  // Vm_area count should be  126;
   pmap(0);

  // Shrinking in front of  vm_area;
  for(int i =0; i < total; i++)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      util_munmap(vm_area_start, pages*10);
  }
  // Vm_area count should be  126;
  pmap(0);

  // Shrinking at the  end of vm_area;
  for(int i =0; i < total; i++)
  {
      char * vm_area_start = mm1 + (i * (pages*100));
      char* page_90_from_start = vm_area_start + (pages*90);

      util_munmap(page_90_from_start, pages*10);
  }
  // Vm_area count should be  126;
  pmap(1);


  // Deallocating all the vm_area with READ PROT
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100)) + pages*10;
      util_munmap(vm_area_start, pages*80);
  }
  // Vm_area count should be  63;
  pmap(1);

   // Deallocating all the vm_area with READ_WRITE PROT
  for(int i =0; i < total; i = i + 2)
  {
        char* vm_area_start = mm1 + (i * (pages*100)) + pages*10;
        util_munmap(vm_area_start, pages*80);
  }
  // Vm_area count should be  0;
  pmap(0);
  
  return 0;
}
