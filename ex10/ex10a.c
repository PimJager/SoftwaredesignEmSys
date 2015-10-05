#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>

#include  <rtdk.h>
#include <sys/io.h>

#define NUMRUNS     10000
#define BASEPERIOD  0   // baseperiod 0 to get ns
#define PERIOD      1e6   // execution time of low prio task in ns

RT_TASK taskP;

int runs = 0;
RTIME diffs[NUMRUNS]; 



void task(void *arg)
{
    diffs[runs] = rt_timer_read();
    rt_printf("Run %d, runtime %d\n", runs, diffs[runs]);
    int err = 0;
    unsigned long long period = 1000000000LLU;
    if(runs == 0) err = rt_task_set_periodic(NULL, TM_NOW, period);
    if(err != 0)  rt_printf("scheduling task filed with err %d: %s\n", err), strerror(-err);
    runs++;
}

//startup code
void startup(){
  int err = 0;

  // set timing to ns
  rt_timer_set_mode(BASEPERIOD);

  err = rt_task_create(&taskP, "high", 0, 50, 0);
  if(err < 0) rt_printf("Failed to create task ; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&taskP, &task, 0);
  if(err < 0) rt_printf("Failed to start task; error: %d: %s", err, strerror(-err)); 
    err = 0;
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
