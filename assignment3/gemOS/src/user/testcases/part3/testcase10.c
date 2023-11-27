#include<ulib.h>

/*simple cfork testcase to check separate stack for child and parent */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
  long pid;
  int a[1030];
  a[10] = 100;
  a[1025] = 10250;

  pid = cfork();
  if(pid){
      sleep(60);
      printf("a[10]:%d, a[1025]:%d\n",a[10],a[1025]);
      if(a[10] == 100 && a[1025] == 10250){
          printf("Testcase passed\n");
      }
      else
      {
          printf("Testcase failed\n");
      }
      return 0;
  }
  else{
      a[10] = 10;
      a[1025] = 1025;
  }
  return 0;
}

