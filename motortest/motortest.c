#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A NXT_PORT_A

DeclareCounter(SysTimerCnt);
DeclareTask(Task1);
DeclareTask(Task2);

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

TASK(Task2)
{
	nxt_motor_set_speed(A, 80, 0);
	while(ecrobot_is_ENTER_button_pressed()==TRUE){
		nxt_motor_set_speed(A, -80, 0);
	}
	TerminateTask();
}

TASK(Task1)
{
	display_goto_xy(0, 0);
	display_string("Motor Test");
	display_goto_xy(0, 1);
	display_string("SPIN:");
	display_goto_xy(5, 1);
	display_int(nxt_motor_get_count(A), 3);
	display_update();	
	ActivateTask(Task2); 
	TerminateTask();
}

int sum(int a, int b){
 return a+b;
}

