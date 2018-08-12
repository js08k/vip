/************************************
 * Virtual IP stack					*
 * File:	vIPconfig.h				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
#ifndef _vIPconfig_h
#define _vIPconfig_h
#include "vtypes.h"

//#define BIG_ENDIAN
#define LITTLE_ENDIAN

/*******************************
 *  Virtual IP define settings
 *******************************/
#define RXPAYLOAD		1500	//Incoming MAC pay-load max is 1500 bytes.
//#define ARPLIFE			400		//Number of seconds an ARP can live.

// Device MAC address
// #define DEFAULTMAC	 { 0x14, 0xfe, 0xb5, 0xb6, 0xfb, 0xe5 } /* eth0 */
//#define DEFAULTMAC	 { 0xbc, 0x77, 0x37, 0xda, 0x86, 0xbc }	/* wlan0 */
#define DEFAULTMAC		{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } /* arm */

// Networking layer specific:
// Default Gateway
#define DEFAULTGW	 ( 10  | 0 <<8 | 1 <<16 | 1 <<24 )

// Default Netmask
#define NETMASK	 ( 255 | 255 <<8 | 255 <<16 | 0 <<24 )

// Broadcasting MAC address
#define BROADCASTMAC { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
//#define BROADCASTMAC DEFAULTMAC /*{ 0xbc, 0x77, 0x37, 0xda, 0x86, 0xbc } */

#endif
