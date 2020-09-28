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
DeclareEvent(ForwardBackwardVelocityEvent);

DeclareTask(BreakTask);
DeclareEvent(BreakEvent);

DeclareTask(CurveTask);
DeclareEvent(CurveEvent);

DeclareResource(R1);

int speed = 0;                // 뒷바퀴 속도
int speed_B = 0;              // 앞바퀴 제어 조절하기 위한 변수
float pre;                    // 앞바퀴 속도  
int target = 0;
int flag_stop = 0;            // 멈춰야 하는지, 아닌지
int sonar_data[10] = {0, };   // 시간 순 배열
int count = 0;                // 경사, 낭떠러지를 만나고 일정 시간 동안 뒷바퀴를 역방향으로 회전시켜주기 위해 횟 수를 카운트하는 변수
int flag_sonar = 0;           // B버튼눌렀는지, 안 눌렀는지 확인하기 위한 변수

/* below macro enables run-time Bluetooth connection */
#define RUNTIME_CONNECTION

/* LEJOS OSEK hooks */
void ecrobot_device_initialize()
{
  nxt_motor_set_count(NXT_PORT_B, 0);
  ecrobot_init_bt_slave("ma-slave");
  ecrobot_init_sonar_sensor(NXT_PORT_S1);
  for (int i = 0; i < 10; i++)   // 시작 시 배열을 모두 채우고 시작
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

  U8 TouchSensorStatus;

  ecrobot_read_bt_packet(bt_receive_buf, 32);

  /* read packet data from the master device */

  // sonar sensor와 바닥까지의 거리를 확인하기 위해...
  display_clear(0);
  display_goto_xy(0, 3);
  display_string("distance:");
  display_goto_xy(0, 4);
  display_int(sonar_data[9], 0);
  display_update();

  ///////////////////////////////////

  if (bt_receive_buf[5] == 1)  //급속 모드
  {
    speed = 90;
  }
  if (bt_receive_buf[5] == 2)  // 저속 모드
  {
    speed = 75;
  }

  ///////////////////////////////////

  if (bt_receive_buf[3] == 1 && bt_receive_buf[7] != 1)  // 전진
  {
    speed = -speed;
    SetEvent(ForwardBackwardVelocityTask, ForwardBackwardVelocityEvent);
  }
  else if (bt_receive_buf[3] == 2 && bt_receive_buf[7] != 1)  //후진
  {
    count = 0;
    SetEvent(ForwardBackwardVelocityTask, ForwardBackwardVelocityEvent);
  }

  /////////////////////////////////////

  if (bt_receive_buf[4] == 3)  //좌회전
  {
    speed_B = -100;
  }
  else if (bt_receive_buf[4] == 4)  //우회전
  {
    speed_B = 100;
  }
  else
    speed_B = 0;

  pre = (50 * speed_B) / 100 - nxt_motor_get_count(NXT_PORT_B);

  SetEvent(CurveTask, CurveEvent);

  ////////////////////////////////////
  if (bt_receive_buf[6] == 1 && bt_receive_buf[7] == 1)  //  즉시 브레이크에서 A를 누를 때
  {
    speed = 0;
    SetEvent(BreakTask, BreakEvent);
  }

  else if (bt_receive_buf[6] == 2 && bt_receive_buf[7] == 1)  //천천히 브레이크에서 A를 누를 때
  {
    while (speed >= 0)
    {
      for (int i = 0; i < 500; i++);
      speed -= 1;
      SetEvent(BreakTask, BreakEvent);
    }
  }

  if (bt_receive_buf[7] == 2)  // 버튼 누르고 있을 시
  {
    flag_sonar = 1;
  }

  if (bt_receive_buf[7] == 0)  // 버튼 안 누르고 있을 때
  {
    flag_sonar = 0;
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

  TouchSensorStatus_old = TouchSensorStatus;

  TerminateTask();
}
/* SonarTask executed every 500ms */
TASK(SonarTask)
{
  if (flag_sonar == 0)  TerminateTask(); // B버튼을 안 누르면 SonarTask를 실행할 필요없음
  GetResource(R1);
  static S32 sonar_sort[10];

  S32 tmp = 0; // 

  for (int i = 0; i < 10; i++)   // 하나씩 앞으로 민다.
  {
    sonar_data[i] = sonar_data[i + 1];
    sonar_sort[i] = sonar_data[i];
  }
  sonar_data[9] = ecrobot_get_sonar_sensor(NXT_PORT_S1);  // 새로운 값 맨 끝에 넣기
  sonar_sort[9] = sonar_data[9];
  for (int i = 0; i < 9; i++)  // 버블 정렬
  {
    for (int j = 0; j < 9 - i; j++)
    {
      if (sonar_sort[j] > sonar_sort[j + 1])
      {
        tmp = sonar_sort[j];
        sonar_sort[j] = sonar_sort[j + 1];
        sonar_sort[j + 1] = tmp;
      }
    }
  }

  if (sonar_sort[6] > 20)  // 낭떠러지 (추락 방지 시스템)
    flag_stop = 1;
  if (sonar_sort[6] < 20)  // 낭떠러지 아닐 경우
    flag_stop = 0;
  if (sonar_sort[6] < 6)  // 올라갈 수 있는 최대 경사 한계 변환값 (차체 충돌 방지 시스템)
    flag_stop = 1;

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

/* Idle Task */

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

TASK(ForwardBackwardVelocityTask)  // 전진, 후진 event
{
  while (1)
  {
    WaitEvent(ForwardBackwardVelocityEvent);
    ClearEvent(ForwardBackwardVelocityEvent);

    nxt_motor_set_speed(NXT_PORT_A, speed, 0);
    nxt_motor_set_speed(NXT_PORT_C, speed, 0);
    if (flag_stop == 1)    // 낭떠러지일 경우 역방향을 50*5ms만큼 주고 멈춘다.  // 저속 모드
    {
      count++;
      if (speed == -75)
      {
        if (count >= 0 && count < 50)
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
      else if (speed == -90)  // 급속 모드
      {
        if (count >= 0 && count < 200)
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

TASK(BreakTask)  //  브레이크 event
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

TASK(CurveTask)  // 조향 event
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
