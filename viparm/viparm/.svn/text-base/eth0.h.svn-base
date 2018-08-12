#ifndef _eth0_h
#define _eth0_h

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Virtual IP includes
#include "vconfig.h"
#include "vbase.h"
//#include "apps/telnet.h"

typedef struct
{
	xSemaphoreHandle Ethrx;
	xTaskHandle		 EthTask;
	bool			 Enabled;
} interface;

void vNetwork( void *pvParameters );
void vEth0( void *pvParameters );
void vEth1( void *pvParameters );
void vEth2( void *pvParameters );
void vEth3( void *pvParameters );
void vEth4( void *pvParameters );
void vEth0int( void );

#endif
