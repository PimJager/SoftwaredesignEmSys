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

#define COLUMN 514890 // 8 columns per letter

int X[8] 	= {D0|D7, D1|D6, D2|D5, D3|D4, D3|D4, D2|D5, D1|D6, D0|D7};
int SQ[8]	= {255, D0|D7, D0|D7, D0|D7, D0|D7, D0|D7, D0|D7, 255};

RT_TASK drawT;
RT_INTR clk_i;

void draw(){
	normalize(); //make sure we're going right
	while(1){
		rt_task_sleep(COLUMN*10); //wait to center the X
		drawSymbol(X);
		rt_task_sleep(COLUMN*93); //write on the way back
		drawSymbolRev(X); //not really necessary, as the X is symetric
		normalize();
	}
}

void drawSymbol(int* sym){
	int i = 0;
	while(i<8){
		outb(sym[i], LEDS);
		rt_task_sleep(COLUMN);
		i++;
	}
	outb(OFF, LEDS);
}

void drawSymbolRev(int* sym){
	int i=7;
	while(i>0){
		outb(sym[i], LEDS);
		rt_task_sleep(COLUMN);
		i--;
	}
	outb(OFF, LEDS);
}

//establishes the direction of the pendulum when function ran!
//so the direction at the end of the funtion is the other way!
//0 for first interrupt is going left
//1 for first interrupt is going right 
int direction(){
	rt_intr_wait(&clk_i, TM_INFINITE);
	RTIME fst = rt_timer_read();
	rt_intr_wait(&clk_i, TM_INFINITE);
	RTIME snd = rt_timer_read();
	RTIME diff = snd - fst;
	if(diff < 1e5) {rt_printf(
		"SMALL, diff to small running third: %d\n", diff);
		rt_intr_wait(&clk_i, TM_INFINITE);
		RTIME thrd = rt_timer_read();
		diff = thrd - fst;
	}
	rt_printf("%d\t", diff);
	if(diff < 6e7) return 0;
	else 		   return 1;
}

void normalize(){
	if(direction() == 1) {rt_printf("YES\n"); rt_intr_wait(&clk_i, TM_INFINITE);}
	else rt_printf("NO\n");
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
