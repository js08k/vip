/************************************
 * Virtual IP stack					*
 * File:	vIPipv4.h				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
#ifndef _vIPipv4_h
#define _vIPipv4_h

//#include "vbase.h"
#include "vtypes.h"

struct ipv4_addr
{
	swaddr address;
	swaddr gateway;
	swaddr netmask;
    swaddr broadcast;
};

typedef struct _tagIPgram
{
	uint8_t		HeaderLength:4;
	uint8_t		version:4;
	uint8_t		DSCP:6;
	uint8_t		ECN:2;
	uint16_t	TotalLength:16;
	uint16_t	Identification;
	uint16_t	Flags:3;
	uint16_t	FragmentOffset:13;
    uint8_t		TimeToLive;
	uint8_t		Protocol;
	uint16_t	Checksum;
	swaddr		sip;
	swaddr		tip;
} tagIPgram;

void vIPprint(int sfd, int argc, char *argv[] );
int vifup( swaddr address );
bool vifdown( swaddr address );
bool vifgw( int interface, swaddr gateway );
bool vifnm( int interface, swaddr netmask );
bool addrtointerface(const swaddr address, int *interface );
bool interfacetoaddr( struct ipv4_addr *addr, const int interface );
bool isValidBroadcast( const swaddr broadcast );
bool vIPhandler( uint8_t vPort, void* ploc );
bool vIPdeal( int interface, swaddr TIP, uint32_t datn, void* ploc, uint8_t protocol );
uint16_t vIPchecksum( const void *datl, uint32_t datn );

#endif
