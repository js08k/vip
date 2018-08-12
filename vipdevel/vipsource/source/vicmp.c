/************************************
 * Virtual IP stack					*
 * File:	vIPicmp.c				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
//! vIP stack includes
#include "vconfig.h"
#include "voverlay.h"
#include "vicmp.h"
#include "vheap.h"
#include "vstring.h"
#include "vipv4.h"
#include "vbase.h"


// Enum defining the possible types for the icmp type
enum icmptype
{
    EchoReply = 0,
    DestUnreachable = 3,
    SourceQuench = 4,
    RedirectMessage = 5,
    AlternateHost = 6,
    EchoRequest = 8,
    RouterAdvertise = 9,
    RouterSolicitation = 10,
    TimeExceeded = 11,
    TimeStamp = 13,
    TimeStampReply = 14,
    InfoRequest = 15,
    InfoReply = 16,
    AddressMaskRequest = 17,
    AddressMaskReply = 18,
    TraceRoute = 30
};


// ICMP reply declaration
void vICMPreply(int, swaddr, int, const void * );

/*
 * The return value indicates whether to free the heap for ploc
 * 	TRUE == Free the heap
 * 	FALSE == Don't free the heap
 */
bool vICMPhandler( int position, swaddr sip, uint32_t datn, const void *ploc )
{
    // Only handle the ICMP packet if the checksum is correct
    if( vIPchecksum( ICMP(ploc), datn ) == 0 )
    {
        /* Currently only requests for ping reply are handled */
        switch( ICMP(ploc)->type )
        {

        /* Case: Request for a ping reply */
        case EchoRequest:
            /*
               Since the original packet send by the requesting device held in memory is a already built
               ICMP packet, there is only a need to modify a few portions using the same memory allocated
               to receive the packet.
            */
            /* Set ICMP type to be an 0x00 ( an echo reply ) */
            vICMPreply( position, sip, datn, ploc );

            /* The memory is deallocated when this function returns true */
            break;
        }
    }

    /* The memory for the packet is no longer needed and can be deallocated */
    return true;
}

/*!
 * \brief vICMPreply: Replies to a ping request.
 * \param ifint The integer cooresponding to the interface which received the
 * ping request.
 * \param sip The ip address of the source of the ping request.
 * \param datasize The size of the payload of the ip packet containing the
 * ping request. (Same as the size of the ping request packet)
 * \param rploc The memory address containing the ping request packet (this
 * address will include the MAC & ip headers).
 */
void vICMPreply( int ifint, swaddr sip, int datasize, const void *rploc )
{

    void *tploc;
    struct ipv4_addr myaddr;

    if( !interfacetoaddr( &myaddr, ifint ) )
        return;

    int totalsize = IPv4(rploc)->TotalLength + sizeof( tagMACgram );
    tploc = ucalloc( totalsize );

    if( tploc )
    {
        // Copy the received packet to the transmit packet
        vmemcpy( tploc, rploc, totalsize );

        // Set ICMP type to be an EchoReply
        ICMP(tploc)->type = EchoReply;

        // Set ICMP code to 0x00 (no idea)
        ICMP(tploc)->code = 0x00;

        // Clear the checksum so the calculation will be correct
        ICMP(tploc)->checksum = 0;

        // Calculate the new checksum for the ICMP packet.
        ICMP(tploc)->checksum = vIPchecksum( ICMP(tploc), datasize );

        // Send the packet to the IP builder, this function will finish
        // the packet and ship it
        if( vIPdeal( ifint, sip, datasize, tploc, ICMPTYPE ) )
        {
            ucfree( tploc );
        }
    }
}
