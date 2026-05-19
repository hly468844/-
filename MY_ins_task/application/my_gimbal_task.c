#include "CAN_receive.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "pid.h"
#include "math.h"
extern fp32 jiaodu[3];
extern	 fp32 INS_angle[3];
fp32 yaw,pitch;
pid_type_def ymotor,pmotor;
fp32 ykp=20,yki,ykd,pkp=35,pki=0.5,pkd=2,yout,pout,pkuixishu=-9000,pkui;
fp32 t;


typedef enum
{
	STATE_IDLE=0,
	STATE_ENABLE,
	STATE_FOLLOW,
	STATE_AIMING
}state;
typedef enum
{
	EVENT_DISABLE=0,
  EVENT_ENABLE,
	EVENT_FOLLOW,
	EVENT_AIMING
}Event;


void state_ran(Event ev);
void follow_run(void);
void aiming_run(void);


uint8_t current_state=STATE_IDLE;
uint8_t ev;


int16_t i1,i2;








void my_gimbal_task(void const * argument)
{
	fp32 YPID[3]={ykp,yki,ykd};
	fp32 PPID[3]={pkp,pki,pkd};
	PID_init(&ymotor,PID_POSITION,YPID,6000,500);
	PID_init(&pmotor,PID_POSITION,PPID,6000,3000);
	
  while(1)
	{
		t=t+0.0005;
		pitch=30*sin(t);
		state_ran(ev);
		vTaskDelay(2);
		
	}
}



//	CAN_cmd_chassis(i1,0,0,0);
//		vTaskDelay(2);
//	CAN_cmd_gimbal(i2,0,0,0);
//		vTaskDelay(2);











void follow_run(void)
{
		yout=PID_calc(&ymotor,jiaodu[0], yaw);
		pout=PID_calc(&pmotor,jiaodu[1], pitch);
	
		pkui=pkuixishu*cos(INS_angle[1]);         //丑陋前馈
		if(pkui>0)pkui=-pkui;
	
		if(pitch>70) pout=-10000;      //超角度回力
		if(pitch<-70) pout=10000;	
		if(yaw<-90||yaw>90) yout=0;
		
		
	
		CAN_cmd_chassis((int16_t)pout,0,0,0);
		vTaskDelay(2);
	//CAN_cmd_gimbal((int16_t)yout,0,0,0);
		vTaskDelay(2);
}


void aiming_run(void)
{
		

}




void state_ran(Event ev)
{
	switch(current_state){
		case STATE_IDLE:
			HAL_GPIO_WritePin(GPIOH, GPIO_PIN_10, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOH, GPIO_PIN_11, GPIO_PIN_RESET);
			if(ev==EVENT_ENABLE)
			{
				current_state=STATE_ENABLE;
			}
		break;
		
		case STATE_ENABLE:
			HAL_GPIO_WritePin(GPIOH, GPIO_PIN_10, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOH, GPIO_PIN_11, GPIO_PIN_SET);
			if(ev==EVENT_FOLLOW)
			{
				current_state=STATE_FOLLOW;
				follow_run();
			}
			else if(ev==EVENT_AIMING)
			{
				current_state=STATE_AIMING;
				aiming_run();
			}
		break;
		
		case STATE_FOLLOW:
			if(ev==EVENT_FOLLOW)
			{
				follow_run();
			}
			else if(ev==EVENT_AIMING)
			{
				current_state=STATE_AIMING;
				aiming_run();
			}
			else if(ev==EVENT_DISABLE)
			{
				current_state=STATE_IDLE;
			}
		break;
		
		case STATE_AIMING:
			if(ev==EVENT_FOLLOW)
			{
				current_state=STATE_FOLLOW;
				follow_run();
			}
			else if(ev==EVENT_AIMING)
			{
				aiming_run();
			}
			else if(ev==EVENT_DISABLE)
			{
				current_state=STATE_IDLE;
			}
		break;
	}
}