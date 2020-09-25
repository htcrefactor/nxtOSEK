#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define A NXT_PORT_A

/* nxtOSEK hook to be invoked from an ISR in category 2 */
DeclareCounter(SysTimerCnt);
DeclareTask(Task1);
DeclareTask(Task2);
DeclareTask(Task3);

int m_speed = 100;
int thcount = 0;

void user_1ms_isr_type2(void){ 
	StatusType ercd;
	ercd = SignalCounter(SysTimerCnt);
	if (ercd != E_OK) {
		ShutdownOS(ercd);
	}
}

TASK(Task3) {
	if(m_speed != 0)
	{
		m_speed -= 5;
		nxt_motor_set_speed(A, m_speed,1);
	}
	TerminateTask();
}

TASK(Task2) {
	if(ecrobot_is_ENTER_button_pressed() == TRUE)
	{
		thcount += 1;
		ActivateTask(Task3);
	}else
	{
		m_speed = 100;
	}
	TerminateTask();
}



TASK(Task1)
{
	display_goto_xy(0,0);
	display_string("Thouch count!!");
	display_goto_xy(0,1);
	display_string("Count: ");
	display_goto_xy(6,1);
	display_int(thcount,3);
	display_update();
	nxt_motor_set_speed(A,m_speed,1);
	TerminateTask();
}
