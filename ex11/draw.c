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

#define D0   1 //0x0000 0001
#define D1   2 //0x0000 0010
#define D2   4 //0x0000 0100
#define D3   8 //0x0000 1000
#define D4  16 //0x0001 0000
#define D5  32 //0x0010 0000
#define D6  64 //0x0100 0000
#define D7 128 //0x1000 0000

#define LEDS 0x378

RT_TASK drawT;
RT_INTR clk_i;

void draw(){
	int c = 0;
	while(1){
	    int x = rt_intr_wait(&keypress, TM_INFINITE);
	    if(x>0){
	    	RTTIME t = rt_timer_read();
	    	rt_printf("INTR %d RECEVIED on %d \n", c, t);
	    	x--;
	    	c++;
	    }
	}
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
	rt_set_timer_mode(0);
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