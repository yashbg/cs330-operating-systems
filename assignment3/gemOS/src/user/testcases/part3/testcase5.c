#include<ulib.h>


int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  int pid = getpid();
  cfork();
  cfork();
  if(pid == getpid())
  {
	  sleep(60);
	  printf("cow faults count: %d\n", get_cow_fault_stats());
  }

  return 0;
}

