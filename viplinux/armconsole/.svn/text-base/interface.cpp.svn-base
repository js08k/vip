#include "interface.h"

#include <QDebug>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

// Returns a pointer to an MAC header struct
#define MAC(ploc) \
    ( (tagMACgram *)ploc )

// Returns a pointer to the payload of a MAC packet
#define MACpayload(ploc) \
    ( (char *)MAC(ploc) + sizeof( tagMACgram ) )

struct __hwaddr
{
    uint8_t data[6];
};
/* Defining the hardware address type */
typedef struct __hwaddr hwaddr;

typedef struct _tagrxMACpacket
{
    hwaddr dest;
    hwaddr sourc;
    uint16_t type;
} tagMACgram;

uint16_t hton16( uint16_t input )
{
    return ( input << 8 | input >> 8 );
}

// Access to the raw socket properties
struct sockaddr_ll sll;

int network_fd;

Interface::Interface(QObject *parent)
    : QObject(parent)
    , m_timer( this )
{
    connect( &m_timer, SIGNAL( timeout() ), SLOT( loop() ) );

    struct ifreq ifr;

    // Create a raw socket
    network_fd = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );

    // Verify the socket fd was opened
    if ( network_fd == -1 )
    {
        // Unable to open the socket
        qFatal( "Unable to create a raw socket." );
    }

    memset( &sll, 0, sizeof( struct sockaddr_ll ) );
    memset( &ifr, 0, sizeof( struct ifreq ) );

    strncpy( (char *)ifr.ifr_name, "eth0", IFNAMSIZ);

    if( ( ioctl( network_fd, SIOCGIFINDEX, &ifr ) ) == -1 )
    {
        qFatal( "Error getting interface information." );
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

    m_timer.start( 0 );
}

void Interface::loop()
{
    static QByteArray ploc( 1500, (char)0 );

    int size = read( network_fd, ploc.data(), ploc.size() - 1 );

    ploc.data()[size] = 0;

    if( size > 14 )
    {

        if( hton16( MAC(ploc.data())->type ) == 0x1987 )
        {
//            qDebug() << QByteArray( (char *)MACpayload( ploc.data() ),
//                                    size - sizeof( tagMACgram ) );
            qDebug() << (char *)MACpayload( ploc.data() );
        }
    }
}


