/* ex07a.c */

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

RT_TASK demo_task[NTASKS];
RT_SEM mysync;

// set new baseperiod for the clock :
// - clock only increments counter when a base period is passed
// - the unit of the clock counter is called a jiffie, which in fact
//   represents that a baseperiod is passed
//   e.g. If 10 baseperiods are passed, the clock gives an increase
//   in 10 jiffies
#define BASEPERIOD 1e6   // baseperiod in ns.

// when base period is set, all times in the api are expressed in jiffies
#define EXECTIME   200   // execution time in jiffies
#define SPINTIME    10   // spin time in jiffies
#define SLICE       20   // slice size for RBR scheduling

void demo(void *arg)
{
    RTIME starttime, runtime;
    int num=*(int *)arg;
    RT_TASK *curtask;
    RT_TASK_INFO curtaskinfo;

    int res1 = rt_task_set_mode(0, T_RRB, NULL);
    if(res1!=0) rt_printf("Error setting RRB scheduling: %d: %s\n", res1, strerror(-res1));
    int res2 = rt_task_slice(NULL, SLICE);
    if(res2!=0) rt_printf("Error setting slice for task %d: %d: %s\n", arg, res2, strerror(-res2));

    rt_printf("Task  : %d\n",num);

    rt_sem_p(&mysync,TM_INFINITE);

    // let the task run RUNTIME(=200) jiffies in steps of SPINTIME(=20) jiffies
    runtime = 0;
    while(runtime < EXECTIME) {
      rt_timer_spin(SPINTIME*BASEPERIOD);  // spin cpu doing nothing
      // note: rt_timer_spin function does not accept jiffies only nanoseconds
      //       deviates from timing conventions throughout the Xenomai API

      runtime = runtime + SPINTIME;

      rt_printf("Running Task  : %d  at time : %d\n",num,runtime);
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

  // change to period mode because round robin does not work
  // in one shot mode
  rt_timer_set_mode(BASEPERIOD);// set tick period
  for(i=0; i < NTASKS; i++) {
    rt_printf("start task  : %d\n",i);
    sprintf(str,"task%d",i);
    rt_task_create(&demo_task[i], str, 0, 50, 0);
    rt_task_start(&demo_task[i], &demo, &i);
  }
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