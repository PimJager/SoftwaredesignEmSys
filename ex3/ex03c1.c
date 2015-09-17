//Using semaphore and a broadcast signal start tasks in priority order

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
 
#include <native/task.h>
#include <native/timer.h>
 
#include  <rtdk.h>
RT_TASK task1, task2, task3, task4, task5;
 
RT_SEM semGlobal;

void task(void *arg)
{
  rt_sem_p(&semGlobal, TM_INFINITE);

  int a = * (int *) arg;

  RT_TASK *curtask;
  RT_TASK_INFO curtaskinfo;
  curtask=rt_task_self();
  rt_task_inquire(curtask,&curtaskinfo);
  rt_printf("Task name: %s arg: %d \n", curtaskinfo.name, a);
  
}
 
int main(int argc, char* argv[])
{
 
  // Perform auto-init of rt_print buffers if the task doesn't do so
  rt_print_auto_init(1);
 
  // Lock memory : avoid memory swapping for this program
  mlockall(MCL_CURRENT|MCL_FUTURE);
 
  rt_printf("starting tasks\n");
 
  int res = rt_sem_create(&semGlobal, "a", 0, S_PRIO);
  if(res < 0) rt_printf("Failed to create semaphore; error: %d: %s", res, strerror(-res));

  rt_task_create(&task1, "t1", 0, 10, 0);
  rt_task_create(&task2, "t2", 0, 20, 0);
  rt_task_create(&task3, "t3", 0, 30, 0);
  rt_task_create(&task4, "t4", 0, 40, 0);
  rt_task_create(&task5, "t5", 0, 50, 0);
 

  int a1 = 1;
  int a2 = 2;
  int a3 = 3;
  int a4 = 4;
  int a5 = 5;
  
  rt_task_start(&task1, &task, &a1);
  rt_task_start(&task2, &task, &a2);
  rt_task_start(&task3, &task, &a3);
  rt_task_start(&task4, &task, &a4);
  rt_task_start(&task5, &task, &a5);

  rt_sem_broadcast(&semGlobal);
}

