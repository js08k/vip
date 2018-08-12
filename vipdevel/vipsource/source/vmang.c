/************************************
 * Virtual IP stack					*
 * File:	vIPmang.c				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 ************************************/
// This is the Virtual IP packet manager
// This handles the IP headers
#include "vconfig.h"
#include "voverlay.h"
#include "vmang.h"
#include "vtypes.h"
#include "vfilter.h"
#include "vheap.h"
#include "vstring.h"
#include "vipv4.h"
#include "varp.h"
#include "vsocket.h"
#include "vbase.h"
#include "vportable.h"

// Packet Manager stack type
struct ptrack
{
    uint8_t     drop:1;
    uint8_t     isrx:1;
    uint8_t     reserved:6;
    uint16_t    ptype;
    void*       ploc;
};

// Packet Manager stack
static struct
{
    struct ptrack *stack;
    int size;
} manager;

bool vIPInitializeManager( void )
{
    // Initialize the Manager stack
    manager.stack = 0;
    manager.size = 0;

    // Initialize the IP stack heap
    initializeucheap();

    // Initialize the base layer of the IP stack
    vInitializeBase();

    // Initialize the arp portion of the ip stack
    vInitializeArp();

    // Initialize ipv4

    // Initialize the sockets
    vInitializeSockets();

    return true;
}

bool vMangKernel( void )
{
    int n;          /* Packet manager count */
    int pAddress;	/* protocol address */
    hwaddr dest;

    vSocketKernel();

    // If no stack is present, return from the manager kernel
    if( manager.stack == 0 )
    {
        return true;
    }

    vEthernetIntDisable();

    // Start the operations of the manager
    // NOTE: The value of manager.size may increase while the loop is
    // iterrating. This occurs if a packet is triggered to be queued for
    // transmission
    for( n = 0; n < manager.size; ++n )
    {
        // See if the structure contains a pending packet
        if( !manager.stack[n].drop  )
        {
            // See if the packet is a received packet
            if( manager.stack[n].isrx )
            {
                // Filter all incoming packets
                if( !vPreFilter( manager.stack[n].ploc ) ||
                        !vPostFilter( manager.stack[n].ploc ) )
                {
                    // Packet failed on either prefilter or postfilter

                    // Drop the packet
                    manager.stack[n].drop = true;

                    // restart at the for-loop
                    continue;
                }

                switch( manager.stack[n].ptype )
                {
                case ARPTYPE:
                    // If the packet is addressed to IP address of pAddress
                    if( addrtointerface( ARP(manager.stack[n].ploc)->TPA, &pAddress ) )
                    {
                        // Handle the ARP message
                        vARPhandler( pAddress, manager.stack[n].ploc );
                    }

                    // Set the manager structure to a completed state, this drops packets which did not match
                    manager.stack[n].drop = true;

                    break;
                case IPv4TYPE:
                    // If the address on this device matches
                    if( addrtointerface( IPv4(manager.stack[n].ploc)->tip,  &pAddress )	)
                    {
                        // Send the packet to the ipv4 handler, ignore any return value
                        vIPhandler( pAddress, manager.stack[n].ploc );

                        // set the manager structure to a completed state
                        manager.stack[n].drop = true;
                    }
                    else
                    {
                        // Unknown interface, drop the packet
                        manager.stack[n].drop = true;
                    }

                    break;
                default:
                    // Unsupported message type
                    manager.stack[n].drop = true;
                    break;
                }
            }
            else
            {
                // Structure contains a blocked transmitting packet
                switch( manager.stack[n].ptype )
                {
                case ARPTYPE:
                    // todo: handle arptype tx packets
                    manager.stack[n].drop = true;
                    break;
                case IPv4TYPE:
                    // Pending packet is an ipv4 transmission packet
                    if( vARPGetMAC( IPv4( manager.stack[n].ploc )->tip, &dest ) )
                    {

                        // Handle the message appropriately
                        vPutMACpacket( dest, IPv4TYPE, (uint8_t *)manager.stack[n].ploc );

                        // Set the flag indicating this structure is free
                        manager.stack[n].drop = true;
                    }
                    break;

                default:

                    manager.stack[n].drop = true;

                    break;
                }
            }
        }
    }

    // Start from the end of the manager & attempt to free all handled packets
    for( n = manager.size; n > 0; )
    {
        // Decrement the counter
        n--;

        // Check if the packet is flagged to be dropped
        if( manager.stack[n].drop )
        {
            // Free the packet's memory at this location
            ucfree( manager.stack[n].ploc );

            // Clear the packet location pointer
            manager.stack[n].ploc = 0;

            // The last packet in the manager is no longer urgent, shrinking
            manager.size--;
        }
        else
        {
            // A pending packet is restricting the shrinking of the manager
            break;
        }
    }

    if( manager.size > 0 )
    {
        // Some packets are still waiting in the manager; the manager can only
        // be shrunk. This operation will never cause an error and so the
        // return does not need to be checked.
        manager.stack = (struct ptrack *)ucrealloc( manager.stack, ( manager.size * sizeof( struct ptrack ) ) );
    }
    else
    {
        /* No more packets are waiting in the manager, it can be deallocated all together */
        ucfree( manager.stack );
        manager.stack = null;
    }

    vEthernetIntEnable();

    /* Exit vMangKernel */
    return true;
}

/*!
 * \brief vMangQueueTX Queues a packet for transmission and the next event loop
 * the packet will be processed. If this function returns true, then the manager
 * will take control of the packet and free the memory associated with the
 * packet. If false is returned, the manager is unable to accept the packet and
 * the memory must be freed by the caller.
 * \param ploc The base location of the packet, including room for the layer 2
 * headers. All layer 3 information and higher must already be set for this
 * packet.
 * \param ptype This is the layer 2 type of the packet.
 * \return Returns true if the packet is now in possession by the manager. If
 * false is returned, then the manager is unable to process the packet.
 */
bool vMangQueueTX( void *ploc, uint16_t ptype )
{
    void *temp;

    if( manager.stack )
    {
        temp = ucrealloc( manager.stack,
                          ( ( manager.size + 1 ) * sizeof( struct ptrack ) ) );
    }
    else
    {
        temp = ucalloc( sizeof( struct ptrack ) );
    }

    if( !temp )
    {
        return false;
    }

    // The new location of the stack is now valid
    manager.stack = (struct ptrack *)temp;

    // Set the packet type
    manager.stack[manager.size].ptype = ptype;

    // set this management structure to be a TX packet
    manager.stack[manager.size].isrx = false;

    // Save the pointer to the packet
    manager.stack[manager.size].ploc = ploc;

    // Appropriately set the flags
    manager.stack[manager.size].drop = false;

    // Increment the manager size.
    ++manager.size;

    // Return a success
    return true;
}

/*******************************************************************
bool vMangQueueRX( ploc, ptype )
    API for adding a reception packet to the manager to be processed.
 *******************************************************************/
bool vMangQueueRX( void *ploc )
{
    void *temp;

    if( manager.stack )
    {
        // Expand the memory of the manager stack.
        temp = ucrealloc( manager.stack,
                          ( ( manager.size + 1 ) * sizeof( struct ptrack ) ) );
    }
    else
    {
        temp = ucalloc( sizeof( struct ptrack ) );
    }

    if( !temp )
    {
        return false;
    }

    // Save the pointer of the new allocated stack
    manager.stack = (struct ptrack *)temp;

    // Set the packet type
    manager.stack[manager.size].ptype = MAC(ploc)->type;

    // Set the packet to show as a RX packet
    manager.stack[manager.size].isrx = true;

    // Save the pointer to the packet
    manager.stack[manager.size].ploc = ploc;

    // Appropriately set the flags
    manager.stack[manager.size++].drop = false;

    // Return a success
    return true;
}
