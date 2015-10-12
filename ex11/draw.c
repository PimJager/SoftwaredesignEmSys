#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>

#include  <rtdk.h>
#include <sys/io.h>

#define CLK_INTR 7

#define OFF   0 //0x0000 0000
#define D0    1 //0x0000 0001
#define D1    2 //0x0000 0010
#define D2    4 //0x0000 0100
#define D3    8 //0x0000 1000
#define D4   16 //0x0001 0000
#define D5   32 //0x0010 0000
#define D6   64 //0x0100 0000
#define D7  128 //0x1000 0000

#define LEDS 0x378

RT_TASK drawT;
RT_INTR clk_i;

#define RESULTFILE "mess.csv"
RTIME m[1000];

void draw(){
	while(1){
		normalize();
    	outb(D0 | D1 | D2 | D7, LEDS);
    	rt_intr_wait(&clk_i, TM_INFINITE);
    	outb(OFF, LEDS);
    	rt_intr_wait(&clk_i, TM_INFINITE);
	}
}

//establishes the direction of the pendulum
//0 for first interrupt is going left
//1 for first interrupt is going right
int direction(){
	RTIME fst = rt_timer_read();
	rt_intr_wait(&clk_i, TM_INFINITE);
	RTIME snd = rt_timer_read();
	RTIME diff = snd - fst;
	if(diff < 6.6e7) return 0;
	else 			 return 1;
}

void normalize(){
	if(direction() == 0) rt_intr_wait(&clk_i, TM_INFINITE);
	return;
}

void setupCLK(){
  	ioperm(0x37A, 1, 1);
  	ioperm(LEDS, 1, 1);
  	outb(inb(0x37A)|0x10, 0x37A);
}

void init_xenomai(){
  	mlockall(MCL_CURRENT|MCL_FUTURE);
  	rt_print_auto_init(1);
}

void startup(){
	rt_timer_set_mode(0);
	int err = 0;
	err = rt_intr_create(&clk_i, NULL, CLK_INTR, I_PROPAGATE);
		if(err < 0) rt_printf("Failed creating interrupt: %d: %s", err, strerror(-err)); 
		 err = 0;
	err = rt_task_create(&drawT, "draw", 0, 50, 0);
		if(err < 0) rt_printf("Failed to create task ; error: %d: %s", err, strerror(-err)); 
		err = 0;
	err = rt_task_start(&drawT, &draw, 0);
		if(err < 0) rt_printf("Failed to start task; error: %d: %s", err, strerror(-err)); 
		err = 0;
}

int main(){
	init_xenomai();
	setupCLK();
	startup();
	pause();
}
