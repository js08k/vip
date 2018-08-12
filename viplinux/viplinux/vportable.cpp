/************************************
 * Virtual IP stack					*
 * File:	vIPportable.c			*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 * 									*
 * Translation in order to run		*
 * on linux simulated network 		*
 * through the backplane			*
 * two stars in a comment "**"		*
 * indicate a linux specific 		*
 * implementation.					*
 ************************************/
#include "main.h"
#include <QDebug>
#include <QTime>
#include "vportable.h"
#include "vconfig.h"
#include "vbase.h"
#include "vmang.h"
#include "vheap.h"
#include "vtypes.h"
#include "vipv4.h"
#include "vstring.h"

// pcap includes
#include <pcap/pcap.h>

/* **LINUX specific includes 		*/
#include <sys/socket.h>
//#include <linux/if_packet.h>
//#include <linux/if_ether.h>
#include <linux/if_arp.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
//#include <unistd.h>
#include <fcntl.h>


/* **define an Ethernet frame size */
//#ifdef ETH_FRAME_LEN
//#undef ETH_FRAME_LEN
//#endif

//#define ETH_FRAME_LEN 128

// Access to the raw socket properties
struct sockaddr_ll sll;

// File descriptor of the socket
int network_fd;
pcap* m_pcap = 0;

// Buffer for reading from the network
QByteArray networkBuffer( 1500, (char)0 );

QTime msTimer;

void vInitializeMsTime()
{
    msTimer.start();
}

int msTime()
{
    return msTimer.elapsed();
}

void vInitializeHardware( void )
{
    struct ifreq ifr;

    /* **Create a raw socket			*/
    network_fd = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );

    /* **Verify the socket fd was opened */
    if ( network_fd == -1 )
    {
        // Unable to open the socket
        qFatal( "Unable to create a raw socket." );
    }

    memset( &sll, 0, sizeof( struct sockaddr_ll ) );
    memset( &ifr, 0, sizeof( struct ifreq ) );

    strncpy( (char *)ifr.ifr_name, "br0", IFNAMSIZ);

    if( ( ioctl( network_fd, SIOCGIFINDEX, &ifr ) ) == -1 )
    {
        qFatal( "Error getting interface information." );
    }

    if( ( fcntl( network_fd, F_SETFL, O_NONBLOCK ) == -1 ) )
    {
        qWarning() << "Unable to set reading as nonblocking.";
        exit( -1 );
    }

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;

    if( bind( network_fd, (struct sockaddr *)&sll, sizeof( sockaddr_ll ) ) < 0 )
    {
        qDebug() << "Could not bind to specifically" << QString( ifr.ifr_name );
        exit( -1 );
    }

    qDebug() << "Socket was created with fd of"
             << network_fd;

    vInitializeMsTime();
}

void vInitializeEthernetInt( void )
{

}

void vEthernetIntEnable(void)
{

}

void vEthernetIntDisable(void)
{

}

void vEthernetIntClear( void )
{

}

void vEthernetWrite( const void *base, int datl )
{
    int send_result = 0;
    const unsigned char *dest_mac = (unsigned char *)base;

    // Copy destination MAC
    vmemcpy( sll.sll_addr, dest_mac, 6 );

    // Send the packet
    send_result = sendto( network_fd, base, datl, 0,
                          ( struct sockaddr * )&sll, sizeof( sll ) );

    /*	send_result = write( network_fd, base, datl ); */
    if ( send_result == -1 )
    {
        fprintf( stderr, "Packet was not sent. %d %s\n", errno, strerror( errno ) );
    }
}

int vEthernetRead( void* base, int datl )
{
    sint32_t data_size;

    if( networkBuffer.size() >= int( datl ) )
    {
        data_size = datl;
    }
    else
    {
        data_size = networkBuffer.size();
    }

    vmemcpy( base, networkBuffer.data(), data_size );

    networkBuffer.remove( 0, data_size );

    return data_size;
}

void vEthernetClear()
{
    networkBuffer.clear();
}

void setDebugLed( int state )
{
    static bool ledstatus = false;

    if( ledstatus && state )
    {
        qDebug() << "DebugLed: ON";
    }
    else if( !ledstatus && !state )
    {
        qDebug() << "DebugLed: OFF";
    }
}

/*!
 * \brief debug
 * \param string
 */
void debug( const void *string )
{
    qDebug() << QString( (char *)string );
}
