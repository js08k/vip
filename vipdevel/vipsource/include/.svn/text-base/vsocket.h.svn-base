#ifndef _vipsocket_h
#define _vipsocket_h

#include "vtypes.h"

typedef enum
{
    stream,
    dgram
} SockType;

/*!
 * \brief SockBase Used to build the base of a UDP socket or a
 * TCP socket.
 */
struct SockBase
{
    // Socket file descriptor
    int sfd;

    // Socket type
    SockType type;

    // Local Port
    uint16_t localPort;

    // Remote Port
    uint16_t remotePort;

    // Local Address
    swaddr localAddress;

    // Remote address
    swaddr remoteAddress;

    /*!
     * \brief handler This is the socket's way of notifying the application
     * that data has arrived. This function will be called by the socket api.
     * \param sfd This is the socket file descriptor which is calling this
     * handler.
     * \param This is the number of bytes that are currently pending reading.
     * If this is called by TCP socket, this is the number of bytes pending in
     * the TCP receive buffer. If this is called by a UDP socket, this is the
     * number of bytes pending in the buffer pointer.
     * \param buffer When the socket is a UDP socket (dgram), this pointer will
     * contain a pointer to the data read from the dgram. The deallocation of
     * this pointer will occur automaticallyl after this handler has returned.
     * \note The buffer pointer will be set to 0 when this is called by a TCP
     * socket and should be ignored.
     */
    void (* handler)( int, int, const void * );

    void* extention;
};

struct SockBase* matchSocket( SockType type, const void* ploc );

void vInitializeSockets( void );

void vSocketKernel( void );

// TCP socket API
bool vSockConnected( int sfd );

// TCP socket data API
void vSockFlush( int sfd );

uint16_t vSockRoom( int sfd );

uint16_t vSockPend( int sfd );

int vSockWrite( int sfd, const void *data, int size );

uint16_t vSockRead( int sfd, void *data, int size );

uint16_t vSockPeek( int sfd, void *data, int size );

uint16_t Getmaxsfd( void );

struct SockBase *sfdtosocket( int sfd );

int vSocket( SockType, void (*handle)( int, int, const void* ), int window );
int vSocketCopy( int sfd_orig );

bool vCloseSocket( int sfd );

bool vBind(int sfd, swaddr address, uint16_t port );

void vDisconnect(int sfd );

void vSockPrint(int sfd, int argc, char *argv[] );

struct SockBase * getSocket( const swaddr, uint16_t, SockType );

void callback( int sfd, int size, const void * buffer );

#endif
