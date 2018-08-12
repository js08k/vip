#include "vipv4.h"
#include "vfilter.h"
#include "varp.h"
#include "vtypes.h"
#include "voverlay.h"
#include "vbase.h"
#include "vstring.h"
#include "telnet.h"
#include "vconfig.h"

int PreFilterDropped = 0;
int PreFilterAllowed = 0;
int PostFilterDropped = 0;
int PostFilterAllowed = 0;
int PostFilterAllowARP = 0;
int PostFilterAllowIP = 0;

const hwaddr macbroadcast = { BROADCASTMAC };
const hwaddr macaddr = { DEFAULTMAC };

/*
 * This is a firewall function, if the packet submitted passes
 * the rules true is returned. If the packet does not pass the rules
 * the packet is dropped
 */
bool vPostFilter( void *ploc )
{
	PostFilterAllowed++;
    if( MAC(ploc)->type == ARPTYPE )
    {
		PostFilterAllowARP++;
    }
    else if( MAC(ploc)->type == IPv4TYPE )
    {
		PostFilterAllowIP++;
    }

    // Allow all packets to pass
	return true;
}

/*
bool vPreFilter( void *ploc )
	This filter is expected to be executed prior to placing the
packet into the manager. Packets that match a certain description
can be known to always be dropped later, this is a way to pick
those packets out early and drop them early. Doing this allows for
a small amount of processing power to be used prior to the manager
as opposed to a substantial amount more during execution of the
manager.

This filter currently only allows ARP messages to be received when
the packet is sent as a broadcast packet. All packets sent to this
device's MAC address are allowed to pass.
 */
bool vPreFilter( void *ploc )
{
	/* Target packets sent to the broadcast MAC address */

    if( hwaddr_cmp( &MAC(ploc)->dest, &macbroadcast ) )
    {
		/* Target ARP packets */
        if( MAC(ploc)->type != ARPTYPE )
        {
			/* Filter tracking enabled, track the count for dropped packets */
			PreFilterDropped++;
			/* Drop the packet */
			return false;
		}
	}
    else if( !hwaddr_cmp( &MAC(ploc)->dest, &macaddr ) )
    {
            /* Filter tracking enabled, track the count for dropped packets */
            PreFilterDropped++;
            return false;
    }

	/* Filter tracking enabled, track the count for allowed packets */
	PreFilterAllowed++;
	/* Allow the packet */
	return true;
}

void vFilter(int sfd, int argc, char *argv[] )
{
    (void)argc;
    (void)argv;

	char string[16];
    telnetPrint( "\t\tDrop\tAllow\nPreManager:\t", sfd );
	vitoa( string, PreFilterDropped );
    telnetPrint( string, sfd );
    telnetPrint( "\t", sfd );
	vitoa( string, PreFilterAllowed );
    telnetPrint( string, sfd );
    telnetPrint( "\n", sfd );
    telnetPrint( "PostManager:\t", sfd );
	vitoa( string, PostFilterDropped );
    telnetPrint( string, sfd );
    telnetPrint( "\t", sfd );
	vitoa( string, PostFilterAllowed );
    telnetPrint( string, sfd );
    telnetPrint( "\t( ", sfd );
	vitoa( string, PostFilterAllowARP );
    telnetPrint( string, sfd );
    telnetPrint( " ARP allowed, ", sfd );
	vitoa( string, PostFilterAllowIP );
    telnetPrint( string, sfd );
    telnetPrint( " IP allowed)\n", sfd );
}
