#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "bsp_usb.h"
#include "struct_typedef.h"

extern fp32 jiaodu[3];
extern fp32 yaw,pitch;
extern fp32 pout_s,pout,yout,yout_s;
extern int16_t i1;
uint8_t vofa[24];
fp32 m;



void my_usb(void const * argument)
{
	
	vofa[20]=0x00;
	vofa[21]=0x00;
	vofa[22]=0x80;
	vofa[23]=0x7F;
  while(1)
	{
		m=(float)i1;
		
		*(float *)&vofa[0]=jiaodu[0];
		*(float *)&vofa[4]=jiaodu[1];
		*(float *)&vofa[8]=jiaodu[2];
		*(float *)&vofa[12]=yaw;
		*(float *)&vofa[16]=pitch;
		CDC_Transmit_FS(vofa	,24);
		vTaskDelay(3);
	}
}

