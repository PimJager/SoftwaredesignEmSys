/* ex06a.c */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>

#include  <rtdk.h>
#include <sys/io.h>

#define NTASKS 3

#define HIGH 52 /* high priority */
#define MID 51 /* medium priority */
#define LOW 50  /* low priority */

RT_TASK demo_task[NTASKS];
RT_SEM mysync;

#define BASEPERIOD 0   // baseperiod 0 to get ns

#define EXECTIME   2e8   // execution time in ns
#define SPINTIME   1e7   // spin time in ns

void demo(void *arg)
{
    RTIME starttime, runtime;
    int num=*(int *)arg;
    RT_TASK *curtask;
    RT_TASK_INFO curtaskinfo;

    rt_printf("Task  : %d\n",num);

    rt_sem_p(&mysync,TM_INFINITE);

    runtime = 0;
    while(runtime < EXECTIME) {
      rt_timer_spin(SPINTIME);  // spin cpu doing nothing

      runtime = runtime + SPINTIME;

      rt_printf("Running Task  : %d  at ms : %d\n",num,runtime/1000000);
    }
    rt_printf("End Task  : %d\n",num);
}

//startup code
void startup()
{
  int i;
  char  str[10] ;

  // semaphore to sync task startup on
  rt_sem_create(&mysync,"MySemaphore",0,S_FIFO);

  // set timing to ns
  rt_timer_set_mode(BASEPERIOD);

  for(i=0; i < NTASKS; i++) {
    rt_printf("start task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_create(&demo_task[i], str, 0, 50, 0);
    rt_task_start(&demo_task[i], &demo, &i);
  }
  // assign priorities to tasks
  // (or in creation use 50+i)
  rt_task_set_priority(&demo_task[0],LOW);
  rt_task_set_priority(&demo_task[1],MID);
  rt_task_set_priority(&demo_task[2],MID);

  rt_printf("wake up all tasks\n");
  rt_sem_broadcast(&mysync);
}

void init_xenomai() {
  /* Avoids memory swapping for this program */
  mlockall(MCL_CURRENT|MCL_FUTURE);

  /* Perform auto-init of rt_print buffers if the task doesn't do so */
  rt_print_auto_init(1);
}

int main(int argc, char* argv[])
{
  printf("\nType CTRL-C to end this program\n\n" );

  // code to set things to run xenomai
  init_xenomai();

  //startup code
  startup();

  // wait for CTRL-c is typed to end the program
  pause();
}