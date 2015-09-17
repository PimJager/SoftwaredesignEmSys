#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
 
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
 
#include  <rtdk.h>
 
#define ITER 10
 
static RT_TASK  t1;
static RT_TASK  t2;

RT_SEM semGlobal;
 
int global = 0;
 
void taskOne(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1000000ULL);
    int i;
    for (i=0; i < ITER; i++)
    {
        rt_sem_p(&semGlobal, TM_INFINITE);
        rt_printf("I am taskOne and global = %d................\n", ++global);
        rt_sem_v(&semGlobal);
        rt_task_wait_period(NULL);
    }
}
 
void taskTwo(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1000000ULL);
    int i;
    for (i=0; i < ITER; i++)
    {
        rt_sem_p(&semGlobal, TM_INFINITE);
        rt_printf("I am taskTwo and global = %d----------------\n", --global);
        rt_sem_v(&semGlobal);
        rt_task_wait_period(NULL);
    }
}
 
int main(int argc, char* argv[])
{
    /* Perform auto-init of rt_print buffers if the task doesn't do so */
    rt_print_auto_init(1);
 
    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT|MCL_FUTURE);

    int res = rt_sem_create(&semGlobal, "binaryCount", 1, S_FIFO);
    if(res < 0) rt_printf("Failed to create semaphore; error: %d: %s", res, strerror(-res));
 
    /* create the two tasks */
    rt_task_create(&t1, "task1", 0, 1, 0);
    rt_task_create(&t2, "task2", 0, 1, 0);
 
    /* start the two tasks */
    rt_task_start(&t1, &taskOne, 0);
    rt_task_start(&t2, &taskTwo, 0);

    pause();
    return 0;
}
