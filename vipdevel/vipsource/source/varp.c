#include "voverlay.h"
#include "vtypes.h"

#include "varp.h"
#include "vbase.h"
#include "vheap.h"
#include "vstring.h"
#include "telnet.h"
#include "vipv4.h"

/*!
 *  \note ARP entries are only entered when they are requested, this is to
 *  make keep the implementation slightly more secure. An ARP reply
 *  will only be added if the requested flag has been set.
 */

// C++ equivalent: Arp::entry (protected)
struct vArp_entry
{
    hwaddr	hwaddress;
    swaddr	swaddress;
    int		requested:1;
    int		entrySet:1;
    struct vArp_entry *next;
};

struct _arp_table_
{
    struct vArp_entry* stack;
    int size;
} table;

void vInitializeArp()
{
    // Set the stack to null (unallocated)
    table.stack = 0;

    // initialize the current size of the stack
    table.size = 0;
}

/********************************************
 * void vARPrequest( int, uint32_t )		*
 * 		used to send a ARP request. vPort	*
 * is the port who is sending the request	*
 * and TPA is the address containing the	*
 * the IP of the target of the ARP being	*
 * sent.									*
 * Passing zero in for TPA will cause the	*
 * IP address of the port sending the		*
 * request to be sent for the target. This	*
 * will cause a gratuitous ARP message.		*
 ********************************************/
bool vARPrequest( const swaddr tpa )
{
    void *ploc, *temp;
    int n;
    struct ipv4_addr myaddr;

    // IP stack requires an address in order to function on the network,
    // return false
    if( !interfacetoaddr( &myaddr, 0 ) )
    {
        return false;
    }

    if( table.stack )
    {
        // Look through the stack for an entry that already exists
        for( n=0; n < table.size; ++n )	{
            if( swaddr_cmp( &table.stack[n].swaddress, &tpa ) )
            {
                // Break from the loop, a matching entry was found
                break;
            }
        }

        // If n == table.size, an entry was not found in the stack
        if( n == table.size )
        {
            // Extend the size of the table for a new entry
            temp = ucrealloc( table.stack,
                              sizeof( struct vArp_entry )*( table.size + 1 ) );
        }
        else
        {
            // Set temp to table.stack so future code does not mess up the
            // stack
            temp = table.stack;
        }
    }
    else
    {
        // Set the index to zero to indicate the entry location to use
        n = 0;

        // Allocate initial memory for the stack
        temp = ucalloc( sizeof( struct vArp_entry ) );
    }

    // If temp does not have a valid value, return a failure (it should unless
    // a allocation failed
    if( !temp )
    {
        return false;
    }

    // Set the new pointer to the stack
    table.stack = (struct vArp_entry *)temp;

    /* If true, there was an old entry with the requested IP */
    if( n != table.size )
    {
        /* TODO: change to base this request drop off of time elapsed */
        return true;
    }

    // fill the entry with the IP being targeted.
    swaddr_cpy( &table.stack[n].swaddress, &tpa );

    // flag entry as having been requested
    table.stack[n].requested = true;
    table.stack[n].entrySet = false;

    // Request a space to build a packet
    ploc = ucalloc( sizeof( tagARPgram ) + sizeof( tagMACgram ) );

    // Check if space was allocated
    if( !ploc )
    {

        // There was not enough space to allocate for this ARP request
        return false;
    }
    else
    {
        // Set the ARP parameters.
        ARP(ploc)->htype = ETHERNET;
        ARP(ploc)->hlen = ETHERNETLEN;

        // Currently only supporting ipv4
        // todo: Look into expanding support for more than ipv4
        ARP(ploc)->ptype = IPv4TYPE;
        ARP(ploc)->plen = IPV4LEN;

        // Set the operation to be a request
        ARP(ploc)->oper = REQUEST;

        // call the apropriate network to hardware translation
        ARP(ploc)->htype = hton16( ARP(ploc)->htype );
        ARP(ploc)->ptype = hton16( ARP(ploc)->ptype );
        ARP(ploc)->oper = hton16( ARP(ploc)->oper );

        // Add this device's MAC address in the source field
        hwaddr_cpy( &ARP(ploc)->SHA, &vMac_localaddr );

        // Add 0's for the MAC address of the target field (request)
        hwaddr_cpy( &ARP(ploc)->THA, 0 );

        // Setup SPA for a regular ARP request
        swaddr_cpy( &ARP(ploc)->SPA, &myaddr.address );

        // Setup the protocol addresses of the ARP message
        swaddr_cpy( &ARP(ploc)->TPA, &tpa );

        // Request the packet to be sent, with broadcast MAC
        vPutMACpacket( vMac_broadcast, ARPTYPE, ploc );

        // Free the memory used to build the packet
        ucfree( ploc );

        //
        table.size++;

        return true;
    }
}

void vARPhandler( int interface, void* ploc )
{
    ARP(ploc)->htype = hton16( ARP(ploc)->htype );
    ARP(ploc)->ptype = hton16( ARP(ploc)->ptype );
    ARP(ploc)->oper = hton16( ARP(ploc)->oper );

    // todo: Only supporting ipv4 arp requests. This needs to be expanded
    // eventually
    if( ARP(ploc)->htype == ETHERNET &&
            ARP(ploc)->ptype == IPv4TYPE &&
            ARP(ploc)->hlen == MACLEN &&
            ARP(ploc)->plen == IPV4LEN )
    {
        // Verifying ARP hardware/protocol lengths
        if( ARP(ploc)->oper == REQUEST )
        {
            // If ARP type is a request...
            vARPreply( interface, ploc );
        }
        else if( ARP(ploc)->oper == REPLY )
        {
            // If ARP type is a reply...
            vARPwriteMAC( ARP(ploc)->SPA, ARP(ploc)->SHA );
        }
    }
}

/******************************************************************
 * This can be implemented like a ping reply. Since it is a reply
 * the original packet can be used to build the replying packet.
 ******************************************************************/
void vARPreply( int position, const void* rploc )
{
    void* tploc = ucalloc( sizeof( tagMACgram ) + sizeof( tagARPgram ) );

    // If this allocation failed, just let the packet drop. The requester will
    // resend the request.
    if( tploc )
    {
        struct ipv4_addr myaddr;

        vmemcpy( tploc, rploc, sizeof( tagMACgram ) + sizeof( tagARPgram ) );

        // Get the basic address of the IP stack
        // Todo: This likley can be passed to me instead of looking it up again.
        interfacetoaddr( &myaddr, position );

        // Set the ARP parameters.
        ARP(tploc)->htype = ETHERNET;
        ARP(tploc)->hlen = ETHERNETLEN;

        // Currently only supporting ipv4
        // todo: Look into expanding support for more than ipv4
        ARP(tploc)->ptype = IPv4TYPE;
        ARP(tploc)->plen = IPV4LEN;

        // Set the operation to be a request
        ARP(tploc)->oper = REPLY;

        // call the apropriate network to hardware translation
        ARP(tploc)->htype = hton16( ARP(tploc)->htype );
        ARP(tploc)->ptype = hton16( ARP(tploc)->ptype );
        ARP(tploc)->oper = hton16( ARP(tploc)->oper );

        // Set the THA field based on the request's SHA
        hwaddr_cpy( &ARP(tploc)->THA, &ARP(rploc)->SHA );

        // Set the SHA field from this device's MAC
        hwaddr_cpy( &ARP(tploc)->SHA, &vMac_localaddr );

        // Set the TPA field based on the request's TPA
        swaddr_cpy( &ARP(tploc)->TPA, &ARP(rploc)->SPA );

        // Set the SPA field based on the vAddress who is sending this reply
        swaddr_cpy( &ARP(tploc)->SPA, &( myaddr.address ) );

        // Pass the packet to teh MAC API, target MAC address is the MAC
        // address from the requester
        vPutMACpacket( ARP(tploc)->THA, ARPTYPE, tploc );

        // Since the packet was not passed to the manager, this function is
        // responsable for deallocation.
        ucfree( tploc );
    }
}

bool vARPGetMAC( const swaddr ip_addr, hwaddr * mac_addr )
{
    int n;

    for( n=0; n < table.size; ++n )
    {
        // Check if the IP matches this entry being inspected
        if( swaddr_cmp( &table.stack[n].swaddress, &ip_addr ) )
        {
            // Check if the entry has been set
            if( table.stack[n].entrySet )
            {
                // Copy the MAC entry
                hwaddr_cpy( mac_addr, &table.stack[n].hwaddress );

                // Return true, entry was found
                return true;
            }
            else
            {
                // Entry was found, but is not yet set
                return false;
            }
        }
    }

    // Todo: Queue a transmission to request the arp entry.

    // Entry was not found
    return false;
}

/********************************************************
 * void vARPwriteMAC( uint8_t* ip_addr, uint8_t* mac_addr )
 *
 ********************************************************/
void vARPwriteMAC( const swaddr ip_addr, const hwaddr mac_addr )
{

    int n;

    for( n=0; n < table.size; ++n )
    {
        /* Check if the IP matches this entry being inspected */
        if( swaddr_cmp( &table.stack[n].swaddress, &ip_addr ) )
        {
            /* Only copy the MAC address if it was requested */
            if( table.stack[n].requested )
            {
                /* Copy the MAC address from the packet */
                hwaddr_cpy( &table.stack[n].hwaddress, &mac_addr );
            }
            /* Set the entrySet flag */
            table.stack[n].entrySet = true;
            /* Clear the requested flag */
            table.stack[n].requested = false;
            /* Leave the loop */
            break;
        }
    }

}

void vARPprint( int sfd, int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    char string[30];
    char mac[3];
    int n, m;
    for( n = 0; n < table.size; ++n )
    {
        viptostring( string, table.stack[n].swaddress );
        vstrcat( string, " is at " );
        telnetPrint( string, sfd );
        if( table.stack[n].entrySet )	{
            chartohex( table.stack[n].hwaddress.data[0], string );
            for( m = 1; m < 6; m++ )	{
                vstrcat( string, ":" );
                chartohex( table.stack[n].hwaddress.data[m], mac );
                vstrcat( string, mac );
            }
            telnetPrint( string, sfd );
        }
        else	{
            telnetPrint( "??:??:??:??:??:??", sfd );
        }

        telnetPrint( "\n", sfd );
    }
}
