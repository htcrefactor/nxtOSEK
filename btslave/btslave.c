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

int speed = 60;
int motorDisplacement = 0;
int target =0 ;
int flag = 0;
signed int degree;
int right_left;
int systemFreeze = 0;
int sonar_data[10] ={0,};
int count =0;
/* below macro enables run-time Bluetooth connection */

#define RUNTIME_CONNECTION



/* LEJOS OSEK hooks */

void ecrobot_device_initialize()

{
  ecrobot_init_bt_slave("ma-slave");
  ecrobot_init_sonar_sensor(NXT_PORT_S1);
  for(int i = 0; i <10;i++){
   sonar_data[i++]=  ecrobot_get_sonar_sensor(NXT_PORT_S1);
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

  if(ercd != E_OK)

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
/*
display_clear(0);
      display_goto_xy(0, 0);
      display_string("3 :");
      display_int(bt_receive_buf[3],0);
      display_goto_xy(0, 1);
      display_string("4 :");
      display_int(bt_receive_buf[4],0);
      display_goto_xy(0, 2);
      display_string("5 :");
      display_int(bt_receive_buf[5],0);
      display_goto_xy(0, 3);
      display_string("6 :");
      display_int(bt_receive_buf[6],0);
      display_string("7 :");
        display_int(bt_receive_buf[7],0);
      display_goto_xy(0, 5);
      display_string("speed :");
      display_int(speed,0);
      display_update();
      display_goto_xy(0, 6);
      display_string("degree:");
      display_int(degree, 4);
  */

  speed = 60;

///////////////////////////////////

  if (bt_receive_buf[5] == 1){
     speed = 75;
  }
      if (bt_receive_buf[5] == 2){
      speed = 65;
  }

 ///////////////////////////////////

    if (bt_receive_buf[3] == 1 && bt_receive_buf[7] == 0)
  {
      nxt_motor_set_speed(NXT_PORT_A, -speed, 0);
      nxt_motor_set_speed(NXT_PORT_C, -speed, 0);  
     if(flag == 1){
       count++;
    
      if(bt_receive_buf[5] == 2){
        if(count >= 0 && count < 23){
     nxt_motor_set_speed(NXT_PORT_A, 100, 1);
     nxt_motor_set_speed(NXT_PORT_C, 100, 1);
        
         }
       else {
        nxt_motor_set_speed(NXT_PORT_A, 0, 1);
      nxt_motor_set_speed(NXT_PORT_C, 0, 1);
           }      
         }
     if(bt_receive_buf[5] == 1){
     if(count >= 0 && count < 45){
     nxt_motor_set_speed(NXT_PORT_A, 100, 1);
     nxt_motor_set_speed(NXT_PORT_C, 100, 1);
         }
         else {
        nxt_motor_set_speed(NXT_PORT_A, 0, 1);
      nxt_motor_set_speed(NXT_PORT_C, 0, 1);
         }
       }
       TerminateTask();
     }
   }
  else if(bt_receive_buf[3] == 2 && bt_receive_buf[7] == 0)
  {
     nxt_motor_set_speed(NXT_PORT_A, speed, 0);
     nxt_motor_set_speed(NXT_PORT_C, speed, 0);  
     count = 0;
  }

/////////////////////////////////////

if(bt_receive_buf[4] == 3) {
  nxt_motor_set_speed(NXT_PORT_B, -60, 0);
}

if(bt_receive_buf[4] == 4) {
  nxt_motor_set_speed(NXT_PORT_B, 60, 0);
}

if(bt_receive_buf[4] == 0) {
  nxt_motor_set_speed(NXT_PORT_B, 0, 0);
  nxt_motor_set_count(NXT_PORT_B, 0);
}

  ////////////////////////////////////
   if(bt_receive_buf[6] == 1 && bt_receive_buf[7] != 0)
  { 
    speed = 0;
    nxt_motor_set_speed(NXT_PORT_A,speed, 1);
    nxt_motor_set_speed(NXT_PORT_C,speed, 1);
  }

  else if(bt_receive_buf[6] == 2 && bt_receive_buf[7] != 0)
  {
    while(speed >= 0){
    for(int i=0; i < 500; i++);
    speed -= 1;
    nxt_motor_set_speed(NXT_PORT_A,speed, 0);
    nxt_motor_set_speed(NXT_PORT_C,speed, 0);
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

  TouchSensorStatus_old = TouchSensorStatus;



  TerminateTask();

}
/* SonarTask executed every 500ms */
TASK(SonarTask)
{ 
  static S32 distance =0;
   static S32 sonar_sort[10];
   static int i = 0;
   S32 tmp = 0;
   
   for(int i = 0; i<10;i++){
   sonar_data[i] = sonar_data[i+1];
   sonar_sort[i] = sonar_data[i];
  }
    sonar_data[9] = ecrobot_get_sonar_sensor(NXT_PORT_S1);
   sonar_sort[9] = sonar_sort[9]; 
      for(int j=i;sonar_sort[j] < sonar_sort[j-1];j--) {
         tmp = sonar_sort[j];
         sonar_sort[j] = sonar_sort[j-1];
         sonar_sort[j-1] = tmp;
      }
      distance = sonar_sort[6];
      i = 0;
   
  if(distance > 23){ 
   flag = 1;
}
  
  if(distance < 23) flag = 0;
  
  display_clear(0);
  display_goto_xy(0, 1);
  display_string("distance: ");
  display_int(distance,0);
  display_update();
   
  TerminateTask();
}



/* EventHandler executed by OSEK Events */

TASK(EventHandler)

{

  static U8 bt_send_buf[32]; 



  while(1)

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

  

  while(1)

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
