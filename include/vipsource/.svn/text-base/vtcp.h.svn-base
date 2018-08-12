#ifndef _vIPtcp_h
#define _vIPtcp_h

#include "vtypes.h"

// Forward declaare a SockBase
struct SockBase;

// TCP header structure
typedef struct _tagTCPgram
{
    uint16_t	srcePort;
    uint16_t	destPort;
    uint32_t	sequence;
    uint32_t	ACKnumber;
    uint8_t		reserved:4;
    uint8_t		dataOffset:4;
    uint8_t		FIN:1;
    uint8_t		SYN:1;
    uint8_t		RST:1;
    uint8_t		PSH:1;
    uint8_t		ACK:1;
    uint8_t		URG:1;
    uint8_t		ECE:1;
    uint8_t		CWR:1;
    uint16_t	windowSize;
    uint16_t	checksum;
    uint16_t	urgentPointer;
} tagTCPgram;

// Commonly used ports & their names
enum ports
{
    SSH = 22,
    TELNET = 23,
    HTTP = 80,
    HTTPS = 443
};

// Tcp flag masks
enum TcpFlags
{
    f_Fin = 1 << 0,
    f_Syn = 1 << 1,
    f_Rst = 1 << 2,
    f_Psh = 1 << 3,
    f_Ack = 1 << 4,
    f_Urg = 1 << 5,
    f_Ece = 1 << 6,
    f_Cwr = 1 << 7
};

enum TcpEvents
{
    /*!
     * \brief eNewConnection Indicates a SYN flag has been received on a
     * listening socket. This event is only valid for listening sockets.
     */
    eNewConnection = 1 << 0,

    /*!
     * \brief eConnected Indicates a connection has been successfully
     * established. This event is valid for both server & client sockets.
     */
    eConnected = 1 << 1,

    /*!
     * \brief eDisconnected Indicates a connection has been broken. This event
     * is valid for both server and client sockets.
     */
    eDisconnected = 1 << 2,

    /*!
     * \brief eNewData Indicates that new data has arrived on the stream and
     * is ready to be read.
     */
    ePushedData = 1 << 3
};

// The Tcp Socket states
typedef enum
{
    Listen,
    SyncSent,
    SyncReceived,
    Established,
    FinWait1,
    FinWait2,
    CloseWait,
    Closing,
    Closed
} TcpState;

#define TCPSOCK(sock) \
    ( (struct tcpSockExt *)sock->extention )

// TCP socket extention
struct tcpSockExt
{
    // Buffer which contains unread data
    int rxbfd;

    // Buffer which contains unacked (sent) data
    int txbfd;

    // The current state of the socket
    TcpState socketState;

    // Size of the remote window
    uint16_t peerWinSize;

    // This number describes where we are in the sequence of data.
    // This should be incremented when inserting data into the txbfd
    uint32_t localSequence;

    // This number describes where the peer has told us where in the sequence
    // they expect to receive our next byte of data. This should be updated
    // when contigous bytes have been acked in the txbfd
    uint32_t ackedSequence;

    // This number describes where we know the peer is in the sequence.
    uint32_t remoteSequence;

    // This number describes where we have told the peer where in the sequence
    // we expect to receive the next byte of data.
    uint32_t ackToRemSeq;

    int rexmitTimer;
    int rexmitCount;

    // When writing to Tcp, data is written to the overflow buffer first. Once
    // space is available in the txbfd, the data is moved from the overflow
    // buffer into the txbfd and also transmitted to the peer. Once the peer
    // has acknowledged the data, it is then removed from the txbfd.
    char *of;

    // The current size of the overflow buffer
    int ofsize;
};

void vInitializeTCP( void );

void vTcpHandler(void *ploc );

void vTcpAccept( int sfd );

void vTcpReject( int sfd );

int vTcpCopy( int sfd );

bool vTcpSend( int sfd, int flgs,
               const void* optData, int optSize,
               const void* tcpData, int tcpSize );

void vTcpTick( struct SockBase* sock );

uint16_t vTCPchecksum(const void *ploc );

void receiveAck(struct SockBase* sock, const void *ploc );

bool vTcpRefuse( const void* rxploc );

void vTcpDropData( struct SockBase* sock );

void vTcpDisconnect( struct SockBase* sock );

void vTcpFlush( struct SockBase* sock );

int vTcpWrite( struct SockBase* sock, const void *data, int size );

#endif
