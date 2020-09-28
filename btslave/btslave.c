/* btslave.c */

#include "kernel.h"

#include "kernel_id.h"

#include "ecrobot_interface.h"

/* OSEK declarations */
DeclareCounter(SysTimerCnt);
DeclareTask(EventDispatcher);
DeclareTask(EventHandler);
DeclareTask(IdleTask);
DeclareEvent(TouchSensorOnEvent);
DeclareEvent(TouchSensorOffEvent);
DeclareTask(SonarTask);
DeclareTask(ForwardBackwardVelocityTask);
DeclareTask(BreakTask);
DeclareTask(CurveTask);

DeclareEvent(ForwardBackwardVelocityEvent);
DeclareEvent(BreakEvent);
DeclareEvent(CurveEvent);

DeclareResource(R1);

int speed = 0;
int speed2 = 0;
float pre;
int motorDisplacement = 0;
int target = 0;
int flag = 0;
signed int degree;
int right_left;
int sonar_data[10] = {0,};
int count = 0;
int b_mode_flag = 0;

/* below macro enables run-time Bluetooth connection */

#define RUNTIME_CONNECTION

/* LEJOS OSEK hooks */

void ecrobot_device_initialize()

{
    ecrobot_init_bt_slave("ma-slave");
    ecrobot_init_sonar_sensor(NXT_PORT_S1);
    for (int i = 0; i < 10; i++)
    {
        sonar_data[i] = ecrobot_get_sonar_sensor(NXT_PORT_S1);
    }
}

void ecrobot_device_terminate()

{

    ecrobot_term_bt_connection();
}

/* LEJOS OSEK hook to be invoked from an ISR in category 2 */

void user_1ms_isr_type2(void)

{

    StatusType ercd;

    ercd = SignalCounter(SysTimerCnt); /* Increment OSEK Alarm Counter */

    if (ercd != E_OK)

    {

        ShutdownOS(ercd);
    }
}

/* EventDispatcher executed every 5ms */

TASK(EventDispatcher)
{

    static U8 bt_receive_buf[32];

    static U8 TouchSensorStatus_old = 0;
    //motorDisplacement = nxt_motor_get_count(NXT_PORT_B);

    U8 TouchSensorStatus;

    ecrobot_read_bt_packet(bt_receive_buf, 32);

    /* read packet data from the master device */
    degree = nxt_motor_get_count(NXT_PORT_B);

    display_clear(0);

    display_update();

    display_goto_xy(0, 2);
    display_string("distance :");
    display_int(sonar_data[9], 0);

    display_goto_xy(0, 3);
    display_string("6 :");
    display_goto_xy(0, 4);
    display_int(bt_receive_buf[6], 0);
    display_string("7 :");
    display_int(bt_receive_buf[7], 0);
    display_goto_xy(0, 5);
    display_string("speed :");
    display_int(speed, 0);
    display_update();

    ///////////////////////////////////

    if (bt_receive_buf[5] == 1)
    {
        speed = 90;
    }
    if (bt_receive_buf[5] == 2)
    {
        speed = 75;
    }

    ///////////////////////////////////

    if (bt_receive_buf[3] == 1 && bt_receive_buf[7] == 0)
    {
        speed = -speed;
        SetEvent(ForwardBackwardVelocityTask, ForwardBackwardVelocityEvent);
    }
    else if (bt_receive_buf[3] == 2 && bt_receive_buf[7] == 0)
    {
        count = 0;
        SetEvent(ForwardBackwardVelocityTask, ForwardBackwardVelocityEvent);
    }

    /////////////////////////////////////

    if (bt_receive_buf[4] == 3)
    {
        speed2 = -100;
    }
    else if (bt_receive_buf[4] == 4)
    {
        speed2 = 100;
    }
    else
        speed2 = 0;

    pre = (50 * speed2) / 100 - nxt_motor_get_count(NXT_PORT_B);

    SetEvent(CurveTask, CurveEvent);

    ////////////////////////////////////
    if (bt_receive_buf[6] == 1 && bt_receive_buf[7] != 0)
    {
        speed = 0;
        SetEvent(BreakTask, BreakEvent);
    }

    else if (bt_receive_buf[6] == 2 && bt_receive_buf[7] != 0)
    {
        while (speed >= 0)
        {
            for (int i = 0; i < 500; i++)
                ;
            speed -= 1;
            SetEvent(BreakTask, BreakEvent);
        }
    }

    /////////////////////////////////////

    TouchSensorStatus = ecrobot_get_touch_sensor(NXT_PORT_S4);

    if (TouchSensorStatus == 1 && TouchSensorStatus_old == 0)

    {

        /* Send a Touch Sensor ON Event to the Handler */

        SetEvent(EventHandler, TouchSensorOnEvent);
    }

    else if (TouchSensorStatus == 0 && TouchSensorStatus_old == 1)

    {

        /* Send a Touch Sensor OFF Event to the Handler */

        SetEvent(EventHandler, TouchSensorOffEvent);
    }

    if (bt_receive_buf[7] == 2)
    {
        ActivateTask(SonarTask);
    }

    TouchSensorStatus_old = TouchSensorStatus;

    TerminateTask();
}
/* SonarTask executed every 500ms */
TASK(SonarTask)
{

    static S32 sonar_sort[10];
    GetResource(R1);

    S32 tmp = 0;

    for (int i = 0; i < 10; i++)
    {
        sonar_data[i] = sonar_data[i + 1];
        sonar_sort[i] = sonar_data[i];
    }
    sonar_data[9] = ecrobot_get_sonar_sensor(NXT_PORT_S1);
    sonar_sort[9] = sonar_data[9];
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0 ; j < 9 - i ; j++)
        {
            if(sonar_sort[j] > sonar_sort[j+1])
            {
                tmp = sonar_sort[j];
                sonar_sort[j] = sonar_sort[j+1];
                sonar_sort[j+1] = tmp;
            }
        }
    }
    if (sonar_sort[6] > 20)
        flag = 1;
    else if (sonar_sort[6] < 20)
        flag = 0;
    ReleaseResource(R1);

    TerminateTask();
}

/* EventHandler executed by OSEK Events */

TASK(EventHandler)

{

    static U8 bt_send_buf[32];

    while (1)

    {

        WaitEvent(TouchSensorOnEvent); /* Task is in waiting status until the Event comes */

        ClearEvent(TouchSensorOnEvent);

        /* send packet data to the master device */

        bt_send_buf[0] = 1;

        ecrobot_send_bt_packet(bt_send_buf, 32);

        WaitEvent(TouchSensorOffEvent); /* Task is in waiting status until the Event comes */

        ClearEvent(TouchSensorOffEvent);

        /* send packet data to the master device */

        bt_send_buf[0] = 0;

        ecrobot_send_bt_packet(bt_send_buf, 32);
    }

    TerminateTask();
}

/* IdleTask */

TASK(IdleTask)

{

    static SINT bt_status = BT_NO_INIT;

    while (1)

    {

        ecrobot_init_bt_slave("ma-slave");

        if (ecrobot_get_bt_status() == BT_STREAM && bt_status != BT_STREAM)

        {

            display_clear(0);

            display_goto_xy(0, 0);

            display_string("[BT]");

            display_update();
        }

        bt_status = ecrobot_get_bt_status();
    }
}

//////////////////////////////

/////////////////////////////////

TASK(ForwardBackwardVelocityTask)
{
    while (1)
    {
        WaitEvent(ForwardBackwardVelocityEvent);
        ClearEvent(ForwardBackwardVelocityEvent);
        nxt_motor_set_speed(NXT_PORT_A, speed, 0);
        nxt_motor_set_speed(NXT_PORT_C, speed, 0);

        if (flag == 1)
        {
            count++;
            if (speed == -75)
            {
                if (count >= 0 && count < 60)
                {
                    ecrobot_set_motor_speed(NXT_PORT_A, 100);
                    ecrobot_set_motor_speed(NXT_PORT_C, 100);
                }
                else
                {
                    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
                    nxt_motor_set_speed(NXT_PORT_C, 0, 1);
                }
            }
            else if (speed == -90)
            {
                if (count >= 0 && count < 100)
                {
                    ecrobot_set_motor_speed(NXT_PORT_A, 100);
                    ecrobot_set_motor_speed(NXT_PORT_C, 100);
                }
                else
                {
                    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
                    nxt_motor_set_speed(NXT_PORT_C, 0, 1);
                }
            }
        }
    }
}

TASK(BreakTask)
{
    while (1)
    {
        WaitEvent(BreakEvent);
        ClearEvent(BreakEvent);

        if (speed == 0)
        {
            nxt_motor_set_speed(NXT_PORT_A, speed, 1);
            nxt_motor_set_speed(NXT_PORT_C, speed, 1);
        }
        else
        {
            nxt_motor_set_speed(NXT_PORT_A, speed, 0);
            nxt_motor_set_speed(NXT_PORT_C, speed, 0);
        }
    }
}

TASK(CurveTask)
{
    while (1)
    {
        WaitEvent(CurveEvent);
        ClearEvent(CurveEvent);

        if (pre > 0)
        {
            nxt_motor_set_speed(NXT_PORT_B, (pre / 2 + 30), 1);
        }
        else if (pre < 0)
        {
            nxt_motor_set_speed(NXT_PORT_B, (pre / 2 - 30), 1);
        }

        else
        {
            nxt_motor_set_speed(NXT_PORT_B, 0, 0);
        }
    }
}
