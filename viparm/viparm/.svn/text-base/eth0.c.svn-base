#include "eth0.h"
#include "vmang.h"
#include "telnet.h"
#include "status.h"

xSemaphoreHandle xEthDeferred;

interface xEth0;
interface xEth1;
interface xEth2;
interface xEth3;
interface xEth4;

/********************************************
 * vNetwork
 * 	This task is the backbone of the
 * network. It starts the tasks that handle
 * each eth device, and handles the deferred
 * interrupts from the MAC controller.
 ********************************************/
void vNetwork( void *pvParameters )
{
	int interface;
	vIPInitializeManager();

	interface = vifup( vstrtoip( "10.0.1.50" ) );
	vifgw( interface, vstrtoip( "10.0.1.1" ) );
	vifnm( interface, hton32( vstrtoip( "255.255.0.0" ) ) );

	// Create the EthDeferred interrupt flag
	vSemaphoreCreateBinary( xEthDeferred );
	// Clear the initially set flag
	xSemaphoreTake( xEthDeferred, 0 );

	// Stack time task
//	xTaskCreate( vIPtime, ( signed portCHAR *)"Time", 32, NULL, tskIDLE_PRIORITY, NULL );
//    xTaskCreate( vEth0, ( signed portCHAR * ) "Eth0", 512, NULL, tskIDLE_PRIORITY+1, NULL );

	// Enable MAC interrupts
	vIPEnableInterrupt();

	while(1)
	{
		// Pause until an interrupt is deferred.
		xSemaphoreTake( xEthDeferred, portMAX_DELAY );
		// Read the packet into a buffer along with information about the packet.
		vGetMACpacket();
		// Set the Eth0 received packet flag.
		xSemaphoreGive( xEth0.Ethrx );
		// Enable MAC interrupts.
		vIPINTenable();
	}
}

/**********************************************************
 * vEth0
 * 		This task is the interface for the Administrator
 * to the device. It *will* handle the telnet service and
 * the ability to shutdown and open addresses and other
 * admin related tasks.
 **********************************************************/
void vEth0( void *pvParameters )
{

	// Create the Eth0 received packet flag
	vSemaphoreCreateBinary( xEth0.Ethrx );
	// Clear the initially set flag
	xSemaphoreTake( xEth0.Ethrx, 0 );

	// Create the Telnet task
    xTaskCreate( vTaskTelnet, ( signed portCHAR * ) "Telnet", 254, NULL, tskIDLE_PRIORITY+1, NULL );

	vTaskDelay( 200 );

//	vARPRequestRoute( eth[0].route );
	while(1)
	{
		// Pause until a packet is received.
		xSemaphoreTake( xEth0.Ethrx, ( 3 / portTICK_RATE_MS ) );
		vMangKernel();
	}
}

/*******************************************
 * Interrupt handler for the MAC interrupts
 *******************************************/
void vEth0int( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken;

	// Set initial status to false
	xHigherPriorityTaskWoken = pdFALSE;

	// Set the deferred interrupt flag
	xSemaphoreGiveFromISR( xEthDeferred, &xHigherPriorityTaskWoken);

	// Disable MAC interrupts
	vIPINTdisable();

	// Clear the Eth0 interrupt flag
	vIPClearInterruptFlag();

	// Switch context
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
