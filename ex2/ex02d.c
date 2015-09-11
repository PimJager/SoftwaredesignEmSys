#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
 
#include <native/task.h>
#include <native/timer.h>
 
#include  <rtdk.h>
RT_TASK task1;
RT_TASK task2;
RT_TASK task3;
RT_TASK task4;
RT_TASK task5;
 
void task(void *arg)
{
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
 
  /*
   * Arguments: &task,
   *            name,
   *            stack size (0=default),
   *            priority,
   *            mode (FPU, start suspended, ...)
   */
  rt_task_create(&task1, "t1", 0, 50, 0);
  rt_task_create(&task2, "t2", 0, 50, 0);
  rt_task_create(&task3, "t3", 0, 50, 0);
  rt_task_create(&task4, "t4", 0, 50, 0);
  rt_task_create(&task5, "t5", 0, 50, 0);
 

  int a1 = 1;
  int a2 = 2;
  int a3 = 3;
  int a4 = 4;
  int a5 = 5;
  /*
   * Arguments: &task,
   *            task function,
   *            function argument
   */
  rt_task_start(&task1, &task, &a1);
  rt_task_start(&task2, &task, &a2);
  rt_task_start(&task3, &task, &a3);
  rt_task_start(&task4, &task, &a4);
  rt_task_start(&task5, &task, &a5);

  // Loop three tasks
  //task, timeout, period
  int err;
  err = rt_task_set_periodic(&task2, TM_NOW, 100000);
  if(err == 0){
    rt_printf("task started suvvesfully\n");
  } else {
    rt_printf("starting task filed with errno %d\n", err);
  } 

  rt_printf("end program by CTRL-C\n");
  pause();
}

