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

  // changing mprotect at the middle of VMAs;
  char* temp = mm1;
  for(int i =0; i < total; i++)
  {

      char* vm_area_middle = temp+(40 * pages);
      temp += (100*pages);
      if(i%2 == 0){
      	util_mprotect(vm_area_middle, pages*20, PROT_READ);	
      }
      else{
      	util_mprotect(vm_area_middle, pages*20, PROT_READ|PROT_WRITE);
      }

  }
  // Vm_area count should be  210;
  pmap(0);
  //changing the protection flags at every start of the VMA
  for(int i = 0; i<total; i++){
	char* vm_area_start = mm1 + (i* (100*pages));
  	if(i%2 == 0){
		util_mprotect(vm_area_start, pages*40, PROT_READ);
	}
  }
  //vm_area count should be 141
  pmap(1);
  return 0;

}

