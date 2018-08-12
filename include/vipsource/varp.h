/************************************
 * Virtual IP stack					*
 * File:	vIParp.h				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
#ifndef _arp_h
#define _arp_h

#include "vtypes.h"

// Overlay for an ARP packet
typedef struct _tagARPgram
{
    // Hardware type
    uint16_t	htype;

    // Protocol type
    uint16_t	ptype;

    // Hardware address length
    uint8_t		hlen;

    // Protocol address length
    uint8_t		plen;

    // Operation
    uint16_t	oper;

    hwaddr		SHA;    /* Source hardware address */
    swaddr		SPA;    /* Source protocol address */
    hwaddr		THA;    /* Target hardware address */
	swaddr		TPA;
} tagARPgram;

// Values for HTYPE
#define ETHERNET 0x0001
#define ETHERNETLEN 0x06

// Values for address length
#define IPV4LEN 0x04
#define MACLEN 0x06

// Opcode
#define REQUEST 0x0001
#define REPLY 0x0002

void vInitializeArp();
bool vARPrequest( const swaddr );
void vARPhandler( int, void * );
void vARPreply(int, const void *);

// This API has not yet been incorporated
bool vARPGetMAC( const swaddr ip_addr, hwaddr *mac_addr );
void vARPwriteMAC(const swaddr ip_addr, const hwaddr mac_addr );

void vARPprint( int sfd, int argc, char *argv[] );

#endif
