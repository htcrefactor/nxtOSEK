#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A NXT_PORT_A

DeclareCounter(SysTimerCnt);
DeclareTask(Task1);
DeclareTask(Task2);
DeclareTask(Task3);

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
	StatusType ercd;
	ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */
	if(ercd != E_OK)
  	{
    		ShutdownOS(ercd);
  	}
}

int thcount = 0;

TASK(Task2) {
	nxt_motor_set_speed(A, 80, 1);
	TerminateTask();
}

TASK(Task3) {
	if(ecrobot_get_touch_sensor(NXT_PORT_S1) == TRUE) {
		thcount++;
	}
	
	while(ecrobot_get_touch_sensor(NXT_PORT_S1) == TRUE) {
		nxt_motor_set_speed(A, -80, 1);
	}

	TerminateTask();
}

TASK(Task1) {
	display_goto_xy(0, 0);
	display_string("Touch Count!!!");
	display_goto_xy(0, 1);
	display_string("Count:");
	display_goto_xy(6, 1);
	
	if(ecrobot_get_touch_sensor(NXT_PORT_S1) == TRUE) {
		thcount++;	
	}
	display_int(thcount, 3);
	display_update();
	ActivateTask(Task2);
	TerminateTask();
}
