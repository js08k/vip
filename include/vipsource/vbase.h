#ifndef _vbase_h
#define _vbase_h

#include "vtypes.h"

// Global variables owned by base.
extern const hwaddr vMac_broadcast;
extern const hwaddr vMac_localaddr;

// Header structure of a Hardware packet
typedef struct _tagrxMACpacket
{
    hwaddr dest;
    hwaddr sourc;
    uint16_t type;
} tagMACgram;

void vInitializeBase( void );

void vPutMACpacket(const hwaddr dest, uint16_t type, void *ploc );

void vGetMACpacket( void );

uint16_t hton16( uint16_t input );

uint32_t hton32( uint32_t input );

#endif
