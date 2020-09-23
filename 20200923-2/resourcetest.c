#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A 	NXT_PORT_A
#define	S1	NXT_PORT_S1

DeclareResource(R1);
DeclareTask(Task1);
DeclareTask(Task2);
DeclareTask(Task3);
DeclareTask(Task4);

/* Global Variables */
int x = 0;
int y = 0;

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void) {
}

TASK(Task4) {
	GetResource(R1);
	
	ActivateTask(Task1);
	ActivateTask(Task2);
	ActivateTask(Task3);
	
	x = x + 1;
	display_goto_xy(x, y);
	display_int(2, 1);
	display_update();
	
	x = x + 1;
	display_goto_xy(x, y);

	systick_wait_ms(500);
	
	display_int(2, 1);
	display_update();
	
	ReleaseResource(R1);
	
	TerminateTask();
}

TASK(Task3) {
	x = x + 1;
	display_goto_xy(x, y);
	display_int(3, 1);
	display_update();
	
	x = x + 1;
	display_goto_xy(x, y);

	systick_wait_ms(500);
	
	display_int(3, 1);
	display_update();
	
	TerminateTask();
}

TASK(Task2) {
	x = x + 1;
	display_goto_xy(x, y);
	display_int(2, 1);
	display_update();
	
	x = x + 1;
	display_goto_xy(x, y);

	systick_wait_ms(500);
	
	display_int(2, 1);
	display_update();
	
	TerminateTask();
}

TASK(Task1) {
	GetResource(R1);
	
	display_goto_xy(x, y);
	display_int(1, 1);
	display_update();
	
	x = x + 1;
	display_goto_xy(x, 0);
	
	systick_wait_ms(500);
	
	display_int(1, 1);
	display_update();
	
	ReleaseResource(R1);
	
	TerminateTask();
}

