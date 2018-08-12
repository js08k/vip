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

// Uncomment this block if the target is a big endian device
//#ifndef BIG_ENDIAN
//#undef LITTLE_ENDIAN
//#define BIG_ENDIAN
//#endif

// Uncomment this block if the target is a little endian device
#ifndef LITTLE_ENDIAN
#undef BIG_ENDIAN
#define LITTLE_ENDIAN
#endif

/*******************************
 *  Virtual IP define settings
 *******************************/
#define RXPAYLOAD		1500	//Incoming MAC pay-load max is 1500 bytes.
//#define ARPLIFE			400		//Number of seconds an ARP can live.

/* Device MAC address */
//#define DEFAULTMAC  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
//#define DEFAULTMAC  { 0x14, 0xfe, 0xb5, 0xb6, 0xfb, 0xe5 } /* eth0 */
//#define DEFAULTMAC  { 0xbc, 0x77, 0x37, 0xda, 0x86, 0xbc }	/* wlan0 */
#define DEFAULTMAC  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } /* arm */
//#define DEFAULTMAC  { 0x52, 0x54, 0x00, 0xf2, 0xd3, 0x24 }

// Broadcasting MAC address
#define BROADCASTMAC { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
//#define BROADCASTMAC { 0x14, 0xfe, 0xb5, 0xb6, 0xfb, 0xe5 }

#endif
