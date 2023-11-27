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
  // VM_AREA READ -> 31
  // VM_AREA WRITE -> 32

  int total = 63;
  for(int i =0; i < total; i++)
  {
      int prot = i % 2 == 0? PROT_WRITE|PROT_READ : PROT_READ;
      char* temp = util_mmap(NULL, pages*100, prot, 0);
      if(!mm1)
      {
        mm1 = temp;
      }
  }
  // Vm_area count should be  63;
   pmap(0);

  // Unmapping 10 pages from front of each vma with read protection(vm_Area count will be same)
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      util_munmap(vm_area_start, pages*10);
  }
  // Vm_area count should be  63;
  pmap(1);

  // Existing vm_area end of vmas with read + write protection are Expanded (vm_Area count will be same)
  for(int i =1; i < total; i=i+2)
  {
      
      char* temp = util_mmap(NULL, (pages*10), PROT_READ|PROT_WRITE, 0);
  }

  // Vm_area count should be  63;
  pmap(0);

  // Unmapping 10 pages from back of each vma with read protection (vm_Area count will be same)
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      char* vm_area_back_10_pages = vm_area_start + (pages*90);
      util_munmap(vm_area_back_10_pages, pages*10);
  }
  // Vm_area count should be  63;
  pmap(1);


  // Existing vm_area start should be updated(Expanded) (vm_Area count will be same)
  // Excluding first vm_area, Because its start cannot be expanded
  for(int i =2; i < total;  i=i+2)
  {
      char* temp = util_mmap(NULL, pages*10, PROT_READ|PROT_WRITE, 0);
  }

  // Vm_area count should be  63;
  pmap(0);


  // Deallocating all the Middle of vm_area with READ PROT. 
  // Count should be double of READ VM_AREA  31 * 2(READ_PROT) + 32(WRITE_PROT);
  for(int i =1; i < total; i = i + 2)
  {
        char* vm_area_start = mm1 + (i * (pages*100)) + pages*20;
        util_munmap(vm_area_start, pages*10);
  }
  // Vm_area count should be  94;
  pmap(1);


    // Will merge all the vm_area (PROT_READ) which was splitted earlier.
  for(int i =1; i < total;  i=i+2)
  {
      char* temp = util_mmap(NULL, pages*10, PROT_READ, 0);
  }

  // Vm_area count should be  63;
  pmap(0);


  // Deallocating VMAs with READ PROT.
  for(int i =1; i < total; i = i + 2)
  {
        char* vm_area_start = mm1 + (i * (pages*100)) + pages*10;
        util_munmap(vm_area_start, pages*80);
  }
  // Vm_area count should be  32;
  pmap(1);


  // All the vm_area with read and write protection will be merged together.
  for(int i =1; i < total;  i=i+2)
  {
      char* temp = util_mmap(NULL, pages*80, PROT_READ|PROT_WRITE, 0);
  }

  //Vm_area count should be  1;
  pmap(1);

  util_munmap(mm1, (pages*total *100));

  //Vm_area count should be  0;
  pmap(0);

return 0;
}
