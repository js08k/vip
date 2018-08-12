/* Scheduler includes. */
#include "telnet.h"
#include "httpd.h"
#include "vconfig.h"
#include "vipv4.h"
#include "vstring.h"
#include "vmang.h"

/* Hardware library includes. */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

const char *rootPage =
        "<html><body><p>Root Page</p><a href=\"/hello\"><p>Hello Link</p></a><a href=\"/javapage\"><p>JavaScript Page</p></a></body></html>";
const char *HelloWorldPage =
        "<html><head><title>Hello World!</title></head><body><p>Hello World!</p><a href=\"/\"><p>Root Link</p></a></body></html>";
//const char *JavaPage =
//        "<html><head><title>JavaScript page</title><script language=javascript type=\"text/javascript\">alert( \"Success!\" )</script></head><body bgcolor=white><noscript><h2>This page requires JavaScript.</h2></noscript></body></html>";
const char *JavaPage =
        "<html><head><title>Functionpassing</title><script language=javascript type=\"text/javascript\">function saySomething(message){alert(message)}</script></head><body><h2>Famous Presidential Quotes</h2><hr><form><input type=button value=\"Lincoln\" onClick=\"saySomething('Four score and seven years ago...')\"><input type=button value=\"Kennedy\" onClick=\"saySomething('Ask not what your country can do for you...')\"><input type=button value=\"Nixon\" onClick=\"saySomething('I am not a crook!')\"></form></body></html>";

int main( void )
{
	prvSetupHardware();

    vIPInitializeManager();

    vifup( vstrtoip( "10.0.1.130" ) );
    vifnm( 0, vstrtoip( "255.255.255.0" ) );
    vifgw( 0, vstrtoip( "10.0.1.1" ) );

    vInitializeTelnet();

    vInitializeHttpd();

    // Add the webpage
    addContent( "/", rootPage, vstrlen( rootPage ) );
    addContent( "/hello", HelloWorldPage, vstrlen( HelloWorldPage ) );
    addContent( "/javapage", JavaPage, vstrlen( JavaPage ) );

    while(1)
    {
        vMangKernel();
    }

    return 0;
}

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }

	// Set the clocking to run from the PLL at 50 MHz
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	// 	Enable Port F for Ethernet LEDs
	//	LED0        Bit 3   Output
	//	LED1        Bit 2   Output
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
	GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
	GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );

    GPIODirModeSet( GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_DIR_MODE_OUT );
    GPIOPadConfigSet( GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );
}
