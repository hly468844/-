#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "bsp_usb.h"
#include "struct_typedef.h"

extern fp32 jiaodu[3];
extern fp32 yaw,pitch;
uint8_t vofa[20];




void my_usb(void const * argument)
{
	vofa[16]=0x00;
	vofa[17]=0x00;
	vofa[18]=0x80;
	vofa[19]=0x7F;
  while(1)
	{
		
		*(float *)&vofa[0]=jiaodu[0];
		*(float *)&vofa[4]=jiaodu[1];
		*(float *)&vofa[8]=jiaodu[2];
		*(float *)&vofa[12]=pitch;
		CDC_Transmit_FS(vofa	,20);
		vTaskDelay(3);
	}
}

