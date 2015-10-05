#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>

#include  <rtdk.h>
#include <sys/io.h>

#define NUMRUNS     10000
#define BASEPERIOD  0   // baseperiod 0 to get ns
#define PERIOD      1e6   // execution time of low prio task in us
#define FILEO        "time_diff_linux_d.csv"

#define LPT1_IRQ 7

RT_INTR keypress;

RT_TASK taskP;

int run = 0;
RTIME diffs[NUMRUNS];

void task(void *arg)
{
    int err = 0;
    if(run == 0) err = rt_task_set_periodic(NULL, TM_NOW, PERIOD);
    if(err != 0)  rt_printf("scheduling task filed with err %d: %s\n", err), strerror(-err);

    outb(inb(0x378) | 0x01, 0x378); //set D0 HIGH
    
    while(run<NUMRUNS){
      RTIME s = rt_timer_read();
      //set D0 LOW and HIGH again
      outb(inb(0x378) & 0xfe, 0x378);
      outb(inb(0x378) | 0x01, 0x378);
      //wait for respons:
      rt_intr_wait(&keypress, TM_INFINITE);
      diff[run] = rt_timer_read() - s;
      run++;
      rt_task_wait_period(NULL);
    }
    rt_printf("Done listening, saving to file\n");
    write_RTIMES(FILEO, NUMRUNS, diff);
    rt_printf("Done\n");
}


void write_RTIMES(char * filename, unsigned int number_of_values,
                  RTIME *time_values){
         unsigned int n=0;
         FILE *file;
         file = fopen(filename,"w");
         while (n<number_of_values) {
              fprintf(file,"%u;%llu\n",n,time_values[n]);  
              n++;
         }
         fclose(file);
 }

 void setupLPT1(){
  //we need to enable interrupts on LPT1
  ioperm(0x37A, 1, 1);
  outb(inb(0x37A)|0x10, 0x37A);
}

void tearDownLPT1(){
  outb(inb(0x37A)&0xEF, 0x37A);
}

//startup code
void startup(){
  int err = 0;

  // set timing to ns
  rt_timer_set_mode(BASEPERIOD);

  err = rt_intr_create(&keypress, NULL, LPT1_IRQ, I_PROPAGATE);
  if(err < 0) rt_printf("Failed creating interrupt: %d: %s", err, strerror(-err)); 
    err = 0;
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

  setupLPT1();

  //startup code
  startup();

  // wait for CTRL-c is typed to end the program
  pause();

  tearDownLPT1();
}
