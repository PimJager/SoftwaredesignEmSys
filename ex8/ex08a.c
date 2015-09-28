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

#define HIGH 52 /* high priority */
#define MID 51 /* medium priority */
#define LOW 50  /* low priority */

RT_TASK lowP, midP, highP;
RT_SEM start, mysync;

#define BASEPERIOD 0   // baseperiod 0 to get ns

#define EXECTIMELOW   3e7   // execution time of low prio task in ns
#define EXECTIMEMID   1e8   // execution time of mid prio task in ns
#define SPINTIME      1e7   // spin time in ns

#define WAIT          15e6  // time the mid & high priority task sleeps before executing

void prioLow(void *arg)
{
    int err = 0;
    RTIME runtime;

    //rt_sem_v(&start);
    rt_printf("prio low");
    runtime = 0;
    while(runtime < EXECTIMELOW) {
      
      err = rt_sem_p(&mysync,TM_INFINITE);
      if(err < 0) rt_printf("Failed pending semaphore; error: %d: %s", err, strerror(-err)); 
        err = 0;
      rt_printf("Low priority task locks semaphore");

      rt_timer_spin(SPINTIME);  // spin cpu doing nothing

      runtime = runtime + SPINTIME;

      rt_printf("Low priority task unlocks semaphore");
      err = rt_sem_v(&mysync);
      if(err < 0) rt_printf("Failed signaling semaphore; error: %d: %s", err, strerror(-err)); 
        err = 0;
    }
    rt_printf("..........................................Low priority task ends");
}

void prioMid(void *arg){
    RTIME runtime;

    //rt_sem_v(&start);
    rt_task_sleep(WAIT);
  
    runtime = 0;
    while(runtime < EXECTIMEMID) {
      rt_timer_spin(SPINTIME);  // spin cpu doing nothing

      runtime = runtime + SPINTIME;

      rt_printf("Medium task running");
      
    }
    rt_printf("..........................................Medium priority task ends");
}

void prioHigh(void *arg){
    int err = 0;
    //rt_sem_v(&start);
    rt_task_sleep(WAIT);

    int i = 0;
    while(i<3) {
      
      rt_printf("High priority task tries to lock semaphore");
      err = rt_sem_p(&mysync,TM_INFINITE);
      if(err < 0) rt_printf("Failed pending semaphore; error: %d: %s", err, strerror(-err)); 
        err = 0;
      rt_printf("High priority task locks semaphore");

      rt_timer_spin(SPINTIME);  // spin cpu doing nothing

      i++;

      rt_printf("High priority task unlocks semaphore");
      err = rt_sem_v(&mysync);
      if(err < 0) rt_printf("Failed signaling semaphore; error: %d: %s", err, strerror(-err)); 
        err = 0;
    }
    rt_printf("..........................................High priority task ends");
}

//startup code
void startup(){
  int err = 0;
  // semaphore to sync task startup on
  err = rt_sem_create(&mysync,"MySemaphore",1,S_FIFO);
  if(err < 0) rt_printf("Failed to create semaphore; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_sem_create(&start,"Startsync",0,S_FIFO);
  if(err < 0) rt_printf("Failed to create semaphore; error: %d: %s", err, strerror(-err)); 
    err = 0;


  // set timing to ns
  rt_timer_set_mode(BASEPERIOD);

  err = rt_task_create(&lowP, "low", 0, LOW, 0);
  if(err < 0) rt_printf("Failed to create task low; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&lowP, &prioLow, 0);
  if(err < 0) rt_printf("Failed to start task low; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_create(&midP, "mid", 0, MID, 0);
  if(err < 0) rt_printf("Failed to create task medium; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&midP, &prioMid, 0);
  if(err < 0) rt_printf("Failed to start task medium; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_create(&highP, "high", 0, HIGH, 0);
  if(err < 0) rt_printf("Failed to create task high; error: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&highP, &prioHigh, 0);
  if(err < 0) rt_printf("Failed to start task high; error: %d: %s", err, strerror(-err)); 
    err = 0;

  rt_printf("wake up all tasks\n");
  rt_sem_broadcast(&start);
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
