/************************************
 *
 * Virtual IP stack                 *
 * File:	vIPipv4.c           *
 * Author:	Joshua Strickland   *
 * Date:	November 2010       *
 * Version:	1.0                 *
 ************************************/
#include "vconfig.h"
#include "voverlay.h"
#include "vipv4.h"
#include "vheap.h"
#include "vmang.h"
#include "vstring.h"
#include "telnet.h"
#include "vicmp.h"
#include "vudp.h"
#include "vtcp.h"
#include "varp.h"
#include "vbase.h"


//static uint32_t *AddressStack;

static struct
{
    struct ipv4_addr *stack;
    int size;
} table = { 0, 0 };

void vIPprint(int sfd, int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    char string[30];
    int n;

    telnetPrint( "Interface\tGateway\t\tBroadcast\tNetmask\n", sfd );
    for( n = 0; n < table.size; n++ )	{
        viptostring( string, table.stack[n].address );
        telnetPrint( string, sfd );
        telnetPrint( "\t", sfd );
        viptostring( string, table.stack[n].gateway );
        telnetPrint( string, sfd );
        telnetPrint( "\t", sfd );
        viptostring( string, table.stack[n].broadcast );
        telnetPrint( string, sfd );
        telnetPrint( "\t", sfd );
        viptostring( string, table.stack[n].netmask );
        telnetPrint( string, sfd );
        telnetPrint( "\n", sfd );
    }
}

/* Add an address to the stack */
int vifup( swaddr address )
{

    void *temp;
    /* If the stack has already been allocated, resize to accommodate the new entry */
    if( table.stack )	{
        temp = ucrealloc( table.stack, sizeof( struct ipv4_addr ) * ( table.size + 1 ) );
    }
    else	/* Otherwise, allocate the stack */
    {
        temp = ucalloc( sizeof( struct ipv4_addr ) );
    }

    /* Verify the allocation was successful */
    if( temp )	{
        /* Save the new pointer to the stack */
        table.stack = (struct ipv4_addr *)temp;
        /* Set the value for the address */
        table.stack[table.size].address = address;
        /* Set the default gw to be the same as the address */
        table.stack[table.size].gateway = address;
        /* Set the default net mask to be 255.255.255.0 */
        table.stack[table.size].netmask =
                swaddr_fromInt( hton32( 0xffffff00 ) );

        // First portion of calculating the broadcast address
        uint32_t broad = ~swaddr_toInt( &table.stack[table.size].netmask );
        // Second step of calculating the broadcast address
        broad |= swaddr_toInt( &table.stack[table.size].address ) &
                swaddr_toInt( &table.stack[table.size].netmask );
        // Set the broadcast address
        table.stack[table.size].broadcast = swaddr_fromInt( broad );

        /* Increment the size of the table after returning the interface index */
        return table.size++;
    }

    /* Return -1 for the interface index - indicating an error */
    return -1;
}

bool vifgw( int interface, swaddr gateway )	{

    /* Verify there is an interface that exists at the requested index */
    if( ( table.size > interface ) && ( interface > -1 ) )
    {
        table.stack[interface].gateway = gateway;

        return true;
    }

    /* Return false, indicates the pointer's data was not modified */
    return false;
}

bool vifnm( int interface, swaddr netmask )	{

    int n;

    uint32_t temp = swaddr_toInt( &netmask );

    /* Verify there is an interface that exists at the requested index */
    if( ( table.size > interface ) && ( interface > -1 ) )
    {
        temp = hton32( temp );

        // Input validation, Force the Subnet to be valid by truncating
        // extraneous values.
        for( n=32; n > 0; --n )
        {
            // Starting from left to right, search for a 0 bit.
            // When found, create a matching netmask.
            if( !( ( 1 << ( n - 1 ) ) & temp ) )
            {
                temp = 0xffffffff << n;
                break;
            }
        }

        temp = hton32( temp );

        table.stack[interface].netmask = swaddr_fromInt( temp );

        // First portion of calculating the broadcast address
        uint32_t broad = ~swaddr_toInt( &table.stack[interface].netmask );
        // Second step of calculating the broadcast address
        broad |= swaddr_toInt( &table.stack[interface].address ) &
                swaddr_toInt( &table.stack[interface].netmask );
        // Set the broadcast address
        table.stack[interface].broadcast = swaddr_fromInt( broad );

        return true;
    }

    /* Return false, indicates the pointer's data was not modified */
    return false;
}

bool vifdown( swaddr address )
{

    int n;

    for( n = 0; n < table.size; n++ )	{
        if( swaddr_cmp( &table.stack[n].address, &address ) )
        {
            table.size--;
            while( n < table.size )	{
                table.stack[n].address = table.stack[n+1].address;
                table.stack[n].gateway = table.stack[n+1].gateway;
                table.stack[n].netmask = table.stack[n+1].netmask;
            }
            table.stack = (struct ipv4_addr *)ucrealloc( table.stack, ( sizeof( struct ipv4_addr ) * table.size ) );

            return true;
        }
    }

    return false;
}

/*
bool addrtointerface( uint32_t, int * )
 */
bool addrtointerface( const swaddr address, int *interface )
{
    int n;
    for( n = 0; n < table.size; ++n )
    {
        if( swaddr_cmp( &table.stack[n].address, &address ) )
        {
            if( interface )
            {
                *interface = n;
            }

            return true;
        }
    }

    return false;
}

bool isValidBroadcast( const swaddr broadcast )
{
    struct ipv4_addr address;
    int i = 0;

    for( i = 0; i < table.size; ++i )
    {
        if( interfacetoaddr( &address, i ) )
        {
            if( swaddr_cmp( &address.broadcast, &broadcast ) )
            {
                return true;
            }
        }
    }

    return false;
}

/*********************************************************************
 * This function gets a copy of the address structure corresponding
 * to the interface index number. This structure will contain the
 * gateway for the interface along with the netmask. If the pointer
 * passed is modified true is returned, otherwise false is returned.
 *********************************************************************/
bool interfacetoaddr( struct ipv4_addr *addr, const int interface )	{

    /* Verify there is an interface that exists at the requested index */
    if( table.size > interface )	{
        /* Check that a valid pointer was passed */
        if( addr )
        {
            /* Copy the data into the location provided */
            vmemcpy( addr, &( table.stack[interface] ), sizeof( struct ipv4_addr ) );

            /* Return true, indicates the pointer data was modified */
            return true;
        }
    }

    /* Return false, indicates the pointer's data was not modified */
    return false;
}

/*********************************************************
 * void vIPhandler( uint8_t vPort, uint8_t* datl )
 * 		handler for for ipv4 packets. Designed to read
 * and check the ipv4 header and perform necessary actions
 * based on information within the header.
 *********************************************************/
bool vIPhandler( uint8_t vPort, void* ploc )
{
    // Verify the IPv4 checksum is correct
    if( vIPchecksum( IPv4(ploc), IPv4(ploc)->HeaderLength * 4 ) == 0 )
    {
        // Calculate the checksums that will involve the IPv4 header
        switch( IPv4(ploc)->Protocol )
        {
        case TCPTYPE:
            TCP(ploc)->checksum = vTCPchecksum( ploc );
            break;
        case UDPTYPE:
            if( UDP(ploc)->checksum )
            {
                UDP(ploc)->checksum = vUdpChecksum( ploc );
            }
            break;
        }

        // Perform the network to hardware byte reordering for the IPv4 header
        IPv4(ploc)->TotalLength = hton16( IPv4(ploc)->TotalLength );
        IPv4(ploc)->Identification = hton16( IPv4(ploc)->Identification );
        IPv4(ploc)->sip = swaddr_hton( IPv4(ploc)->sip );
        IPv4(ploc)->tip = swaddr_hton( IPv4(ploc)->tip );

        switch( IPv4(ploc)->Protocol )
        {
        case ICMPTYPE:	// The message is a ping request, reply to it...

            return vICMPhandler( vPort, IPv4(ploc)->sip,
                                 IPv4(ploc)->TotalLength - IPv4(ploc)->HeaderLength * 4, ploc );
            break;
        case TCPTYPE:
                vTcpHandler( ploc );
            break;
        case UDPTYPE:
                // Call the Udp Handler
                vUdpHandler( ploc );
            break;
        }
    }

    return true;
    // Checksum was not correct, drop the packet
}

/* Passing datn as zero is a flag to this function that the packet is already built */
/* if datn is zero, only SIP, TIP and ploc must be valid */
bool vIPdeal( int interface,
              swaddr TIP,
              uint32_t datn,
              void* ploc,
              uint8_t protocol )
{
    //	uint8_t dest[6];
    hwaddr dest;
    static uint16_t identification;

    if( datn )
    {
        IPv4(ploc)->version = 4;
        IPv4(ploc)->HeaderLength = 5;
        IPv4(ploc)->DSCP = 0;
        IPv4(ploc)->ECN = 0;
        IPv4(ploc)->TotalLength = hton16( datn + sizeof(tagIPgram) );
        IPv4(ploc)->Identification = hton16( identification );
        identification++;
        IPv4(ploc)->Flags = 0;
        IPv4(ploc)->FragmentOffset = 0;
        IPv4(ploc)->TimeToLive = 128;
        IPv4(ploc)->Protocol = protocol;
        IPv4(ploc)->tip = TIP;
        IPv4(ploc)->sip = table.stack[interface].address;

        // TCP checksum must be calculated after the entire IP header is
        // filled out
        if( protocol == TCPTYPE )
        {
            TCP(ploc)->checksum = 0;
            TCP(ploc)->checksum = vTCPchecksum( ploc );
        }

        // IP checksum must be calculated after the entire IP header is
        // filled out
        IPv4(ploc)->Checksum = 0;
        IPv4(ploc)->Checksum =
                vIPchecksum( IPv4(ploc), IPv4(ploc)->HeaderLength * 4 );
    }

    // Create a integer representation of the local subnet
    int32_t source_subnet =
            swaddr_toInt( &table.stack[interface].netmask ) &
            swaddr_toInt( &table.stack[interface].address );
    // Create a integer representation of the target subnet
    int32_t target_subnet =
            swaddr_toInt( &table.stack[interface].netmask ) &
            swaddr_toInt( &TIP );

    // The IP address is in this subnet, send the packet with MAC addressed
    // to the destination
    if( source_subnet == target_subnet )
    {
        if( vARPGetMAC( TIP, &dest ) )
        {
            vPutMACpacket( dest, IPv4TYPE, (uint8_t *)ploc );

            return true;
        }
        else
        {
            vARPrequest( IPv4(ploc)->tip );
            vMangQueueTX( ploc, IPv4TYPE );

            return false;
        }
    }
    /* The IP address is outside this subnet, send the packet with MAC addressed to the gateway */
    else	{
        if( vARPGetMAC( table.stack[interface].gateway, &dest ) )	{
            vPutMACpacket( dest, IPv4TYPE, (uint8_t *)ploc );

            return true;
        }
        else	{
            vARPrequest( table.stack[interface].gateway );
            vMangQueueTX( ploc, IPv4TYPE );

            return false;
        }
    }


    return false;
}

uint16_t vIPchecksum( const void* datl, uint32_t datn )
{
    register uint32_t sum = 0;
    uint16_t *addr = (uint16_t *)datl;
    int count = datn;

    while( count > 1 )
    {
        sum += *addr++;
        count -= 2;
    }

    if( count > 0 )
    {
        // Done this way so "an invalid read of size 1" does not occur.
        sum += *( (unsigned char *)addr );
    }

    while( sum >> 16 )
    {
        sum = ( sum & 0x0000FFFF ) + ( sum >> 16 );
    }

    return ~sum;
}
