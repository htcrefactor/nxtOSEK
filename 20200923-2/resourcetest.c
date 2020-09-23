#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A 	NXT_PORT_A
#define	S1	NXT_PORT_S1

DeclareCounter(SysTimerCnt);
DeclareTask(Task1);
DeclareTask(Task2);
DeclareTask(Task3);
DeclareTask(Task4);

/* Global Variables */
int x = 0;
int y = 0;

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void) {
	StatusType ercd;
	ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
	if(ercd != E_OK) {
    		ShutdownOS(ercd);
  	}
}

TASK(Task4) {
	TerminateTask();
}

TASK(Task3) {
	TerminateTask();
}

TASK(Task2) {
	TerminateTask();
}

TASK(Task1) {
	display_goto_xy(x, y);
	display_int(1, 1);

	display_goto_xy(x + 1, 0);
	
	systick_wait_ms(500);
	
	display_int(1, 1);
	
	TerminateTask();
}

