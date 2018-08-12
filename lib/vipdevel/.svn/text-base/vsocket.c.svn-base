#include "vconfig.h"
#include "vsocket.h"
#include "vheap.h"
#include "vtcp.h"
#include "vbase.h"
#include "vrandn.h"
#include "vipv4.h"
#include "voverlay.h"
#include "vstring.h"
#include "vportable.h"
#include "telnet.h"

static struct
{
    // Last refrenced socket
    struct SockBase *cache;

    // Socket stack
    struct SockBase *stack;

    // Current size of the stack
    int size;
} table = { 0, 0, 0 };

bool resizeSocketTable( int size )
{
    if( table.size == size )
    {
        // Size remained unchanged, just return.
        return false;
    }

    void *temp;

    // The cache must be cleared since memory may changed.
    table.cache = 0;

    if( !size )
    {
        // Set the size of the stack to be zero.
        table.size = 0;

        // free the memory the stack is using, null value sockets.stack
        // should be accounted for when socket.size == size was performed.
        ucfree( table.stack );

        // Clear the value
        table.stack = 0;

        return true;
    }
    else
    {
        // This will reallocate or allocate based on the value of the stack
        temp = ucrealloc( table.stack, sizeof( struct SockBase ) * size );

        if( temp )
        {
            // Set the location of the stack to the memory passed by realloc
            table.stack = (struct SockBase *)temp;

            // Set the size of the stack to be the size requested to realloc
            table.size = size;

            return true;
        }
    }

    return false;
}

/*********************************************************************
                        **Local API only**

struct socket *sfdtosocket( uint16_t sfd )
        Returns a pointer to the socket when passed the socket file
    descriptor.

    NOTE:
         If sfd is a bad file descriptor this function will return
    null, this must be checked before attempting to use the returned
    value.
 *********************************************************************/
struct SockBase *sfdtosocket( int fd )
{
    // Check if the cache containd the correct socket
    if( table.cache && table.cache->sfd == fd )
    {
        // The socket table's cache was accurate, return the cache
        return table.cache;
    }
    else if( table.stack )
    {
        int n;

        // Search the socket stack for the given file descriptor
        for( n = 0; n < table.size; ++n )
        {
            // Check if the fd exists in the socket stack
            if( table.stack[n].sfd == fd )
            {
                // Set the socket table's cache
                table.cache = &( table.stack[n] );

                // Found the socket structure, returning the pointer
                return &( table.stack[n] );
            }
        }
    }

    // the socket was not able to be found, bad socket file descriptor
    return 0;
}

void vSocketKernel( void )
{
    if( !table.stack )
    {
        // Nothing to do.
        return;
    }

        int i;

        for( i = 0; i < table.size; ++i )
        {
            // Grab a pointer to the socket
            struct SockBase *sock = &table.stack[i];

            if( sock->type == dgram )
            {
                // Currently do nothing w/ dgram sockets
                continue;
            }
            else if( sock->type == stream )
            {
                vTcpTick( sock );
            }
        }
}


// listening sockets
void vInitializeSockets( void )
{
    // Initialize the sockets table
    table.cache = 0;
    table.stack = 0;
    table.size = 0;
}

void vSockPrint( int sfd, int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    int i;
    char string[30];

    telnetPrint( "Sockets", sfd );
    for( i = 0; i < table.size; ++i )
    {
        struct SockBase* sock = &table.stack[i];
        telnetPrint( "\n", sfd );
        vitoa( string, sock->sfd );
        telnetPrint( string, sfd );
        telnetPrint( "\t", sfd );
        telnetPrint( sock->type == stream ? "stream\t" : "dgram\t", sfd );
        viptostring( string, sock->localAddress );
        telnetPrint( string, sfd );
        telnetPrint( ":", sfd );
        vitoa( string, sock->localPort );
        telnetPrint( string, sfd );
        telnetPrint( "\t", sfd );
        viptostring( string, sock->remoteAddress );
        telnetPrint( string, sfd );
        telnetPrint( ":", sfd );
        vitoa( string, sock->remotePort );
        telnetPrint( string, sfd );

        if( sock->type == stream && sock->extention )
        {
            telnetPrint( TCPSOCK(sock)->socketState == Listen ? " Listening" : " Not Listening", sfd );
        }
    }

    telnetPrint( "\n", sfd );
}

int vSocket( SockType type, void (* handle)( int, int, const void* ), int window )
{
    /// \todo lock the socket table mutex

    // Do not allow allocation of window larget then 1K
    if( window > 1024 )
    {
        // Window is too large
        return 0;
    }

    // Request a new file descriptor to use as the socket file descriptor
    int sfd = vfdGet();

    // Check if an error occured when creating the socket file descriptor.
    if( sfd == 0 )
    {
        // Since a file descriptor can not be 0, this indicates an error.
        return 0;
    }

    // Attempt to resize the stack, creating a new socket at the end.
    if( !resizeSocketTable( table.size + 1 ) )
    {
        // Delete the allocated socket file descriptor.
        vfdDel( sfd );

        // Since a file descriptor can not be 0, this indicates an error.
        return 0;
    }

    // Get a pointer to the new socket
    struct SockBase *sock = &table.stack[ table.size - 1 ];

    // Update the socket table's cache to the new socket.
    table.cache = sock;

    // Set the socket file descriptor
    sock->sfd = sfd;

    if( type == dgram )
    {
        // Set the type of the socket
        sock->type = dgram;

        // Zero initialize the socket's local and remote ports
        sock->localPort = 0;
        sock->remotePort = 0;

        // Zero initialize the local & remote addresses
        swaddr_cpy( &sock->localAddress, 0 );
        swaddr_cpy( &sock->remoteAddress, 0 );

        // Set the socket's handler (callback) function
        sock->handler = handle;

        // The extention is not needed for UDP sockets.
        sock->extention = 0;

        /// \todo: Unlock the socket table mutex

        return sock->sfd;
    }

    // Set the type of the socket
    sock->type = stream;

    // Allocate the tcp extention of the sockets
    sock->extention = ucalloc( sizeof( struct tcpSockExt ) );

    // todo: Check the return from the allocation for the TCP extention.
    vmemcpy( sock->extention, 0, sizeof( struct tcpSockExt ) );

    // request a tx & rx buffer for the socket
    TCPSOCK(sock)->rxbfd = vOpenBuf( window );
    TCPSOCK(sock)->txbfd = vOpenBuf( window );

    // verify the buffers and the sfd were created successfully
    if( TCPSOCK(sock)->rxbfd > 0 &&
            TCPSOCK(sock)->txbfd > 0 )
    {
        // Zero initialize the socket's local and remote ports
        sock->localPort = 0;
        sock->remotePort = 0;

        // Zero initialize the local & remote addresses
        swaddr_cpy( &sock->localAddress, 0 );
        swaddr_cpy( &sock->remoteAddress, 0 );

        // Set the pointer to the socket handler
        sock->handler = handle;

        // Set the timeout event to be inactive
        TCPSOCK(sock)->rexmitCount = 0;
        TCPSOCK(sock)->rexmitTimer = msTime();

        // Set the overflow pointer to zero
        TCPSOCK(sock)->of = 0;
        TCPSOCK(sock)->ofsize = 0;

        return sock->sfd;
    }
    else
    {
        // Check if the receive buffer was created
        if( TCPSOCK(sock)->rxbfd > 0 )
        {
            // Close the receive buffer
            vCloseBuf( TCPSOCK(sock)->rxbfd );
        }

        // Check if the transmit buffer was creaed
        if( TCPSOCK(sock)->txbfd > 0 )
        {
            // Close the transmit buffer
            vCloseBuf( TCPSOCK(sock)->txbfd );
        }

        sock->sfd = 0;

        // Shrink the table back down
        resizeSocketTable( table.size - 1 );

        // Delete the allocated socket file descriptor.
        vfdDel( sfd );

        // Since a file descriptor can not be 0, this indicates an error.
        return 0;
    }
}

int vSocketCopy(int sfd_orig )
{
    // Request a new file descriptor to use as the socket file descriptor
    int sfd = vfdGet();

    // Check if an error occured when creating the socket file descriptor.
    if( sfd == 0 )
    {
        // Since a file descriptor can not be 0, this indicates an error.
        return 0;
    }

    // Attempt to resize the stack, creating a new socket at the end.
    if( !resizeSocketTable( table.size + 1 ) )
    {
        // Delete the allocated socket file descriptor.
        vfdDel( sfd );

        // Since a file descriptor can not be 0, this indicates an error.
        return 0;
    }

    // Get a copy of the socket from the sfd
    struct SockBase* orig = sfdtosocket( sfd_orig );

    // Get a pointer to the new socket
    struct SockBase *sock = &table.stack[ table.size - 1 ];

    // Update the socket table's cache to the new socket.
    table.cache = sock;

    // Set the socket file descriptor
    sock->sfd = sfd;

    // Set the type of the socket
    sock->type = orig->type;

    // Zero initialize the socket's local and remote ports
    sock->localPort = orig->localPort;
    sock->remotePort = orig->remotePort;

    // Zero initialize the local & remote addresses
    swaddr_cpy( &sock->localAddress, &orig->localAddress );
    swaddr_cpy( &sock->remoteAddress, &orig->remoteAddress );

    // Clear out the origional's attachment to the peer
    swaddr_cpy( &orig->remoteAddress, 0 );
    orig->remotePort = 0;

    // Set the socket's handler (callback) function
    sock->handler = orig->handler;

    // The extention is not needed for UDP sockets.
    sock->extention = 0;

    return sock->sfd;
}

bool vCloseSocket( int sfd )
{
    // todo: Delete this socket gracefully, this is half-assed right now.
    struct SockBase* sock = sfdtosocket( sfd );

    if( sock->extention )
    {
        if( sock->type == stream )
        {
            vCloseBuf( TCPSOCK(sock)->txbfd );
            vCloseBuf( TCPSOCK(sock)->rxbfd );

            if( TCPSOCK(sock)->of )
            {
                TCPSOCK(sock)->of = 0;
            }
        }

        ucfree( sock->extention );

        sock->extention = 0;
    }

    table.cache = 0;

    int i;
    for( i = 0; i < table.size; ++i )
    {
        sock = &table.stack[i];

        if( sock->sfd == sfd )
        {
            vfdDel( sock->sfd );
            break;
        }
    }

    int j;
    for( j = i; j < table.size - 1; ++j )
    {
        vmemcpy( &table.stack[j], &table.stack[j+1], sizeof( struct SockBase ) );
    }

    resizeSocketTable( table.size - 1 );

    return false;
}

/*
 * Associate a local IP address with an existing socket
 * 	The address must exist on the device for this to succeed
 */
bool vBind( int sfd, swaddr address, uint16_t port )
{
    struct SockBase *sock = sfdtosocket( sfd );

    if( !sock )
    {
        return false;
    }

    if( sock->type == dgram )
    {
        // Check if an interface owns this ip OR if it is a valid broadcast
        // address.
        if( addrtointerface( address, 0 ) || isValidBroadcast( address ) )
        {
            // Set the local port
            sock->localPort = port;

            // Set the local software address
            swaddr_cpy( &sock->localAddress, &address );

            // Return success
            return true;
        }
        else
        {
            // Return a failure
            return false;
        }
    }
    else
    {
        // Verify the address passed exists within the interfaces
        if( addrtointerface( address, 0 ) )
        {
            // Set the local software address
            swaddr_cpy( &sock->localAddress, &address );

            // Set the local port
            sock->localPort = port;

            // Set the state to listen
            TCPSOCK(sock)->socketState = Listen;

            // Return a success
            return true;
        }
        return false;
    }
}

bool vConnect( int sfd, swaddr addr, uint16_t port )
{
    (void)sfd;
    (void)addr;
    (void)port;
    // todo: Implement this function
    return false;
}

/* This is intended to resemble the unix version of accept,
 * with one key difference being: a new socket is not created
 * 	to hold the connection while the old continues to listen.
 * The old socket is used. This will cause a DoS whenever a device
 * connects to this socket, unless a new socket is created in it's place
 */
bool vAccept( uint16_t sfd )
{
    struct SockBase *sock = sfdtosocket( sfd );

    /* If the fd was a valid socket, this evaluates true */
    if( sock )
    {
        /* If the socket has received a request to connect */
        if( TCPSOCK(sock)->socketState == SyncReceived )
        {
            ++TCPSOCK(sock)->localSequence;
            /* A connection request has been received */
            vTcpSend( sock->sfd, ( f_Ack | f_Ack ), 0, 0, 0, 0 );
            /* Transition the state machine */
            TCPSOCK(sock)->socketState = SyncSent;
        }
        else
            return false;
    }

    return false;
}

void vDisconnect( int sfd )
{
    struct SockBase *sock = sfdtosocket( sfd );

    vTcpDisconnect( sock );
}

/***************************************************
 * bool vTCPconnected( tcpSocket* socket )
 *		If a client is connected to the server port
 *	then this function returns true, otherwise this
 *	function will return false.
 ***************************************************/
bool vSockConnected(int sfd )
{
    // Get a socket pointer translated from socket fd
    const struct SockBase *sock = sfdtosocket( sfd );

    /* sock is a valid socket and the event occurred */
    if( sock )
    {
        if ( TCPSOCK(sock)->socketState == Established )
        {
            /* return true */
            return true;
        }
    }

    /* either sfd was invalid or there was no event */
    return false;
}

void vSockFlush( int sfd )
{
    /* pointer to the socket */
    struct SockBase* sock = sfdtosocket( sfd );

    vTcpFlush( sock );
}

int vSockWrite( int sfd, const void *data, int size )
{
    // pointer to the socket
    struct SockBase* sock;

    // get socket pointer from the sfd
    sock = sfdtosocket( sfd );

    // Call the tcp write function
    return vTcpWrite( sock, data, size );
}

uint16_t vSockRead(int sfd, void *data, int size )	{
    /* pointer to the socket */
    struct SockBase *sock = sfdtosocket( sfd );
    /* sock is a valid socket and the event occurred */
    if( sock )
    {
        return vReadBuf( TCPSOCK(sock)->rxbfd, data, size );
    }
    /* either sfd was invalid */
    return 0;
}

uint16_t vSockPeek( int sfd, void *data, int size )
{
    /* pointer to the socket */
    struct SockBase *sock = sfdtosocket( sfd );
    /* sock is a valid socket and the event occurred */
    if( sock )
    {
        return vPeekBuf( TCPSOCK(sock)->rxbfd, data, size );
    }
    /* either sfd was invalid */
    return 0;
}

uint16_t vSockPend(int sfd )	{
    /* pointer to the socket */
    struct SockBase *sock = sfdtosocket( sfd );
    /* sock is a valid socket and the event occurred */
    if( sock )
    {
        return vBufGetPending( TCPSOCK(sock)->rxbfd );
    }
    /* either sfd was invalid */
    return null;
}


uint16_t vSockRoom(int sfd )	{
    /* pointer to the socket */
    struct SockBase *sock = sfdtosocket( sfd );

    if( !sock )
        return 0;

    return vBufGetFree( TCPSOCK(sock)->txbfd );
}

struct SockBase* matchSocket( SockType type, const void* ploc)
{
    if( type == stream )
    {
        int i;
        // Attempt to find a socket that matches a peer w/ existance
        for( i = 0; i < table.size; ++i )
        {
            struct SockBase* sock = &table.stack[i];

            // The socket will contain information about the remote peer
            if( sock &&
                    sock->type == type &&
                    sock->localPort == TCP(ploc)->destPort &&
                    sock->remotePort == TCP(ploc)->srcePort &&
                    swaddr_cmp( &sock->localAddress, &IPv4(ploc)->tip ) &&
                    swaddr_cmp( &sock->remoteAddress, &IPv4(ploc)->sip ) )
            {
                return sock;
            }
        }

        // The socket will contain information about the remote peer
        for( i = 0; i < table.size; ++i )
        {
            struct SockBase* sock = &table.stack[i];

            // The socket will contain information about the remote peer
            if( sock &&
                    sock->type == type &&
                    sock->localPort == TCP(ploc)->destPort &&
                    swaddr_cmp( &sock->localAddress, &IPv4(ploc)->tip ) )
            {
                return sock;
            }
        }
    }

    return 0;
}

struct SockBase * getSocket( const swaddr addr, uint16_t port, SockType type )
{
    if( table.cache &&
            table.cache->type == type &&
            table.cache->localPort == port &&
            swaddr_cmp( &addr, &table.cache->localAddress ) )
    {
        return table.cache;
    }

    int i;

    for( i = 0; i < table.size; ++i )
    {
        struct SockBase *sock = &table.stack[i];

        // Currently force all
        if( sock->type == type &&
                sock->localPort == port &&
                swaddr_cmp( &addr, &sock->localAddress ) )
        {
            // Update the socket table cache
            table.cache = sock;

            // Return the socket sfd
            return sock;
        }
    }

    return 0;
}

void callback( int sfd, int size, const void * buffer )
{
    const struct SockBase *sock = sfdtosocket( sfd );

    if( sock->handler )
    {
        sock->handler( sfd, size, buffer );
    }
}

