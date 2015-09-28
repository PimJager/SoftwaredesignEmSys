/* ex06a.c */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>

#include  <rtdk.h>
#include <sys/io.h>

RT_INTR keypress;
RT_TASK key_isr, dummyT;

#define KEYBOARD_IRQ 1
#define SPINTIME 3e8
#define SLEEPTIME 2e7

void keyhandler(void *arg){
  int n = 0;

  RT_TASK *curtask;
  RT_TASK_INFO curtaskinfo;
  curtask=rt_task_self();
  rt_task_inquire(curtask,&curtaskinfo);
  rt_printf("Handler Task Prio: %d\n", curtaskinfo.cprio);

  while(1){
    int x = rt_intr_wait(&keypress, TM_INFINITE);
    if(x>0){
      rt_task_inquire(curtask,&curtaskinfo);
      rt_printf("KBD interrupt #%d, Prio: %d\n", ++n, curtaskinfo.cprio);
      x--;
    }
  }
}

void dummy(void *arg){
  while(1) {
    rt_printf("Dummy sleep\n");
    rt_task_sleep(SLEEPTIME);
    rt_printf("Dummy spin\n");
    rt_timer_spin(SPINTIME);    
  }
}

//startup code
void startup(){
  int err = 0;
  // set timing to ns
  rt_timer_set_mode(0);

  err = rt_intr_create(&keypress, NULL, KEYBOARD_IRQ, I_PROPAGATE);
  if(err < 0) rt_printf("Failed creating interrupt: %d: %s", err, strerror(-err)); 
    err = 0;

  err = rt_task_create(&key_isr, NULL, 0, 50, 0);
  if(err < 0) rt_printf("Failed creating task: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&key_isr, &keyhandler, NULL);
  if(err < 0) rt_printf("Failed starting task: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_create(&dummyT, NULL, 0, 50, 0);
  if(err < 0) rt_printf("Failed creating task: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&dummyT, &dummy, NULL);
  if(err < 0) rt_printf("Failed starting task: %d: %s", err, strerror(-err)); 
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

  init_xenomai();
  startup();

  pause();
}
