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

int util_mprotect(void * addr, int length, int prot)
{

  int temp = mprotect(addr, length, prot);
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
  int total = 63;
  char * mm1 = 0;
  char * temp = 0;

  // Creating Bulk pages;
  mm1= util_mmap(NULL, (pages*total), PROT_READ|PROT_WRITE, 0);
  for(int i = 0; i < 99; i++)
  {
	  temp = util_mmap(NULL, (pages*total), PROT_READ|PROT_WRITE, 0);
  }

  // vm_area count should be 1
  pmap(0);
  
  // Changing PROTECTION for every 100 alternate pages 
  for(int i =0; i < total; i++)
  {
      int prot = i % 2 == 0? PROT_READ|PROT_WRITE : PROT_READ;
      char* vm_area_start = mm1 + (i * (pages*100));
      util_mprotect(vm_area_start, pages*100, prot);
  }
  // Vm_area count should be  63;
   pmap(1);

  // Changing protection of 10 pages (of VMAs with read protection) from front. So that end of the VMAs with read+write protection will be expanded.
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      //unmapping 10 pages from front
      util_mprotect(vm_area_start, pages*10, PROT_READ|PROT_WRITE);
  }
  // Vm_area count should be  63;
  pmap(0);


  // Changing protection of 10 pages (of VMAs with read protection) from back. 
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100));
      char* vm_area_back_10_pages = vm_area_start + (pages*90);
      util_mprotect(vm_area_back_10_pages, pages*10, PROT_READ|PROT_WRITE);
  }
  // Vm_area count should be  63;
  pmap(1);


  // Changing protection in the center of vm_area, so that vm_Area will be splitted
  for(int i =1; i < total; i = i + 2)
  {
        char* vm_area_start = mm1 + (i * (pages*100)) + pages*20;
        util_mprotect(vm_area_start, pages*10, PROT_READ|PROT_WRITE);
  }
  // Vm_area count should be  125;
  pmap(0);


  // Will merge all the vm_area (PROT_READ) which was splitted earlier.
  for(int i =1; i < total;  i=i+2)
  {
     char* vm_area_start = mm1 + (i * (pages*100)) + pages*20;
     util_mprotect(vm_area_start, pages*10, PROT_READ);
  }
  // Vm_area count should be  63;
  pmap(1);


  // Changing the PROT_READ to PROT_WRITE. So that all the vm_area will be merged to be one.
  for(int i =1; i < total; i = i + 2)
  {
      char* vm_area_start = mm1 + (i * (pages*100 ));
      char * read_area_begining = vm_area_start + (pages*10 );
      util_mprotect(read_area_begining, pages*80, PROT_READ|PROT_WRITE);
  }
  // vm_area count should be 1
   pmap(0);


  util_munmap(mm1, (pages*total *100));

  //Vm_area count should be  0;
  pmap(0);

return 0;
}
