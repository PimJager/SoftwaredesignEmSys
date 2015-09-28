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
RT_TASK key_isr;

#define LPT1_IRQ 7

void keyhandler(void *arg){
  int n = 0;

  rt_printf("Counting presses:\n");

  while(1){
    int x = rt_intr_wait(&keypress, TM_INFINITE);
    if(x>0){
      rt_printf("BUT interrupt #%d\n", ++n);
      x--;
    }
  }
}

//startup code
void startup(){
  int err = 0;
  // set timing to ns
  rt_timer_set_mode(0);

  err = rt_intr_create(&keypress, NULL, LPT1_IRQ, I_PROPAGATE);
  if(err < 0) rt_printf("Failed creating interrupt: %d: %s", err, strerror(-err)); 
    err = 0;

  err = rt_task_create(&key_isr, NULL, 0, 50, 0);
  if(err < 0) rt_printf("Failed creating task: %d: %s", err, strerror(-err)); 
    err = 0;
  err = rt_task_start(&key_isr, &keyhandler, NULL);
  if(err < 0) rt_printf("Failed starting task: %d: %s", err, strerror(-err)); 
    err = 0;

}

void listenLPT1(){
  //we need to enable interrupts on LPT1
  ioperm(0x37A, 1, 1);
  outb(inb(0x37A)|0x10, 0x37A);
}

void tearDownLPT1(){
  outb(inb(0x37A)&0xEF, 0x37A);
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
  listenLPT1();
  startup();

  pause();

  tearDownLPT1();
}
