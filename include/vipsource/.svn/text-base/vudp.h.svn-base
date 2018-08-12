#ifndef _vudp_h
#define _vudp_h

#include "vtypes.h"

// UDP header structure
typedef struct _tagUDPgram
{
    uint16_t    srcePort;
    uint16_t    destPort;
    uint16_t    length;
    uint16_t    checksum;
} tagUDPgram;

void vUdpHandler( const void* ploc );
uint16_t vUdpChecksum( const void* );



#endif
