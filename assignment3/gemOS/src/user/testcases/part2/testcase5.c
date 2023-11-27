//stress checking

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
  int total = 120;
  char *mm1 = 0;
  char *temp = 0;

  // Creating Bulk pages;
  mm1= util_mmap(NULL, (pages*total), PROT_READ|PROT_WRITE, 0);
  for(int i = 0; i < 49; i++)
  {
	  temp = util_mmap(NULL, (pages*total), PROT_READ|PROT_WRITE, 0);
  }

  // vm_area count should be 1
  pmap(0);

  // Changing PROTECTION for every 50 alternate pages 
  for(int i =0; i < total; i++)
  {
      int prot = i % 2 == 0? PROT_READ|PROT_WRITE : PROT_READ;
      char* vm_area_start = mm1 + (i * (pages*50));
      util_mprotect(vm_area_start, pages*50, prot);
  }
  // Vm_area count should be  120;
   pmap(0);

   util_mprotect(mm1, pages*50*total, PROT_WRITE|PROT_READ);


  //Writing in every page should result in 6000 page faults
  for(int i = 0; i<total; i++){
	char* vm_area_start = mm1 + (i*(pages*50));  
  	for(int j = 0; j<50; j++){
		char* page_start = vm_area_start + (j*(pages));
		page_start[0] = 'A';
	}
  }
  pmap(0);

  //Validating the written value in every page
  for(int i = 0; i<total; i++){
        char* vm_area_start = mm1 + (i*(pages*50));
        for(int j = 0; j<50; j++){
                char* page_start = vm_area_start + (j*(pages));
                if(page_start[0] != 'A'){
			printf("Testcase Failed \n");
			exit(0);
		}
        }
  }
  pmap(0);
  
  //unmapping every even VMA 
  for(int i = 0; i < total; i=i+2)
  {
	char* even_vma_addr = mm1 + (i* (pages*50));  
	util_munmap(even_vma_addr, pages*50);
  }
  pmap(0);
  //VMA count = 60

  for(int i = 0; i < total; i=i+2)
  {
        char* even_vma_addr = mm1 + (i* (pages*50));
	char * mm2= util_mmap(even_vma_addr, (pages*50), PROT_READ, MAP_FIXED);
  }
  pmap(0);

  for(int i = 0; i < total; i=i+2)
  {
        char* even_vma_addr = mm1 + (i* (pages*50));
	//Result in 60 more faults
  	char page_read = even_vma_addr[0];
  }
  pmap(0);
  printf("Reached end of the program\n");

  return 0;
}
