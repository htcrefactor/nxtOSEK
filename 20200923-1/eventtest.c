#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A 	NXT_PORT_A
#define	S1	NXT_PORT_S1

DeclareCounter(SysTimerCnt);
DeclareTask(Task1);
DeclareTask(Task2);
DeclareEvent(event1);
DeclareEvent(event2);

/* Global Variables */
int count_t1 = 0;
int count_t2 = 0;

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void) {
	StatusType ercd;
	ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
	if(ercd != E_OK) {
    		ShutdownOS(ercd);
  	}
}

TASK(Task2) {
	while(1) {
		count_t2 = count_t2 + 1;
	
		WaitEvent(event1);
		ClearEvent(event1);
		nxt_motor_set_speed(A, 80, 1);
			
		WaitEvent(event2);
		ClearEvent(event2);
		nxt_motor_set_speed(A, 0, 1);
	}
	
	TerminateTask();
}

TASK(Task1) {
	count_t1 = count_t1 + 1;
	
	display_goto_xy(0, 1);
	display_string("Task1 Count:");
	display_goto_xy(12, 1);
	display_int(count_t1, 3);
	
	display_goto_xy(0, 0);
	display_string("Task2 Count:");
	display_goto_xy(12, 0);
	display_int(count_t2, 3);

	display_update();
	
	if(ecrobot_get_touch_sensor(S1) == TRUE) {
		SetEvent(Task2, event1);
	} else {
		SetEvent(Task2, event2);
	}
	
	TerminateTask();
}

