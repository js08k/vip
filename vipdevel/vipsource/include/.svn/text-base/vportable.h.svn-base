/************************************
 * Virtual IP stack					*
 * File:	vIPportable.h			*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
#ifndef _vIPportable_h
#define _vIPportable_h

#include "vtypes.h"

void vInitializeHardware( void );
void vEthernetIntEnable( void );
void vEthernetIntDisable( void );
void vEthernetIntClear( void );
void vEthernetWrite( const void* base, int length );
int vEthernetRead( void* base, int datl );
int vEthernetPending( void );
void vEthernetClear( void );
void setDebugLed( int state );
void vTimerInterrupt( void );
char vGenerateTrueRandom( void );

void vInitializeMsTime();
int msTime();

void debug( const char *string );

#endif
