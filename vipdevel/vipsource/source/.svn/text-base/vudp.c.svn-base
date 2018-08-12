#include "vudp.h"

#include "voverlay.h"
#include "vtypes.h"
#include "vipv4.h"
#include "vheap.h"
#include "vsocket.h"
#include "vbase.h"

/*!
 * \brief vUdpHandler Distributes and handles UDP packets to the cooresponding
 * socket to which the packet is destined.
 * \param ploc This is a pointer to the packet. The address to which the packet
 * was sent should be refrenced from this pointer. This can either be the actual
 * address the device listens on or this can be the broadcast address which this
 * device is listening on.
 */
void vUdpHandler( const void* ploc )
{
    struct SockBase * sock = 0;

    // If the checksum has a value, assume the sender is expecting the packet's
    // checksum to be validated.
    if( UDP(ploc)->checksum )
    {
        // Verify the checksum was correct.
        if( vUdpChecksum(ploc) == 0 )
        {
            // Look up the socket based on the destination address and the
            // destination port. Packet type is always a data gram when within
            // the UDP api.
            sock = getSocket( IPv4(ploc)->tip,
                              hton16( UDP(ploc)->destPort ), dgram );
        }
    }
    else
    {
        sock = getSocket( IPv4(ploc)->tip, hton16( UDP(ploc)->destPort ), dgram );
    }

    // If the socket exists
    if( sock )
    {
        // Request the socket API to call the application's handler function
        sock->handler( sock->sfd, UDP(ploc)->length, UDPpayload(ploc) );
    }
}

void vUdpWrite( int sfd, const void* data, int size )
{
    (void)sfd; (void)data;
    // Allocate memory for building the datagram
    void *ploc = ucalloc( sizeof( tagMACgram ) + sizeof( tagUDPgram ) + size );

    if( ploc )
    {

    }
}

/*!
 * \brief vUdpChecksum
 * \param ploc
 * \return
 */
uint16_t vUdpChecksum( const void* ploc )
{
    // Get a fast variable to hold the checksum result
    register uint32_t sum = 0;

    // Create a readonly 16 bit pointer to the packet location
    const uint16_t *addr = (uint16_t *)UDP(ploc);
    // Create a counter for the length of the UDP summing
    int count = UDP(ploc)->length;

    // Add the UDP pseudo header to the checksum (Identical to TCP)
    sum += ( (uint16_t *)&(IPv4(ploc)->sip) )[0];
    sum += ( (uint16_t *)&(IPv4(ploc)->sip) )[1];
    sum += ( (uint16_t *)&(IPv4(ploc)->tip) )[0];
    sum += ( (uint16_t *)&(IPv4(ploc)->tip) )[1];
    sum += ( ( IPv4(ploc)->Protocol<<8 ) + UDP(ploc)->length );

    // Loop through summing the udp packet
    while( count > 1 )
    {
        sum += *addr++;
        count -= 2;
    }

    // If the packet was an odd number of bytes, this adds the leftover byte
    if( count > 0 )
    {
        // Done this way so "an invalid read of size 1" does not occur.
        sum += *( (unsigned char *)addr );
    }

    while( sum >> 16 )
    {
        // Add the overflow values to the 16bit checksum
        sum = ( sum & 0x0000ffff ) + ( sum >> 16 );
    }

    return ~sum;
}
