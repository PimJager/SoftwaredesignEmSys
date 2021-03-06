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
#define FILEO        "time_diff_virtual.csv"
RT_TASK taskP;

int run = 0;
RTIME times[NUMRUNS]; 
RTIME diffs[NUMRUNS];

void task(void *arg)
{
    int err = 0;
    if(run == 0) err = rt_task_set_periodic(NULL, TM_NOW, PERIOD);
    if(err != 0)  rt_printf("scheduling task filed with err %d: %s\n", err), strerror(-err);
    
    while(run<NUMRUNS){
      times[run] = rt_timer_read();
      run++;
      rt_task_wait_period(NULL);
    }
    rt_printf("DONE, calculating diff\n");

    calculate_diffs();

    write_RTIMES(FILEO, NUMRUNS, diffs);
}

void calculate_diffs(){
  int i = 0;
  while(i<NUMRUNS){
    diffs[i] = times[i+1] - times[i];
    i++;
  }
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
