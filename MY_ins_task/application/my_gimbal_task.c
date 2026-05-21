#include "CAN_receive.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "pid.h"
#include "math.h"
extern fp32 jiaodu[3];
extern fp32 INS_angle[3];
extern motor_measure_t motor_chassis[7];
fp32 yaw,pitch;
pid_type_def ymotor,pmotor,ymotor_s,pmotor_s;
fp32 ykp=1.8,yki=0.05,ykd=0.5,pkp=1.2,pki=0.00,pkd=0.8,yout,pout,pkuixishu=-0,pkui;
fp32 ykp_s=80,yki_s=0.11,ykd_s=0.8,pkp_s=80,pki_s=3.5,pkd_s=1.2,yout_s,pout_s;
fp32 t=0;


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
uint8_t ev,count;


int16_t i1,i2;








void my_gimbal_task(void const * argument)
{
	fp32 YPID[3]={ykp,yki,ykd};
	fp32 PPID[3]={pkp,pki,pkd};
	fp32 YPID_S[3]={ykp_s,yki_s,ykd_s};
	fp32 PPID_S[3]={pkp_s,pki_s,pkd_s};
	PID_init(&ymotor,PID_POSITION,YPID,200,50);
	PID_init(&pmotor,PID_POSITION,PPID,200,50);
	PID_init(&ymotor_s,PID_POSITION,YPID_S,6000,2000);
	PID_init(&pmotor_s,PID_POSITION,PPID_S,6000,3000);
	
  while(1)
	{
		
		
		t=t+0.01;
		yaw=30*sin(t);
		pitch=yaw;
		state_ran(ev);
//		CAN_cmd_chassis(i1,0,0,0);
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
		yout_s=PID_calc(&ymotor_s,motor_chassis[4].speed_rpm, yout);
		pout_s=PID_calc(&pmotor_s,motor_chassis[0].speed_rpm, pout);
		
	
		pkui=pkuixishu*cos(INS_angle[1]);         //丑陋前馈
		if(pkui>0)pkui=-pkui;
	
		if(pitch>50) pout=-10000;      //超角度回力
		if(pitch<-50) pout=10000;	
		if(yaw>45) yout=-5000;      //超角度回力
		if(yaw<-45) yout=5000;	
		
		
	
		CAN_cmd_chassis((int16_t)pout_s,0,0,0);
		vTaskDelay(2);
	  CAN_cmd_gimbal((int16_t)yout_s,0,0,0);
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