#include "vtcp.h"

#include "vconfig.h"
#include "vsocket.h"
#include "vheap.h"
#include "vbase.h"
#include "vrandn.h"
#include "vipv4.h"
#include "voverlay.h"
#include "vstring.h"
#include "vportable.h"

void vTcpHandler( void *ploc )
{
    if( TCP(ploc)->checksum )
    {
        return;
    }

    // Perform the network to hardware byte reordering for the Tcp header
    TCP(ploc)->srcePort = hton16( TCP(ploc)->srcePort );
    TCP(ploc)->destPort = hton16( TCP(ploc)->destPort );
    TCP(ploc)->sequence = hton32( TCP(ploc)->sequence );
    TCP(ploc)->ACKnumber = hton32( TCP(ploc)->ACKnumber );
    TCP(ploc)->windowSize = hton16( TCP(ploc)->windowSize );
    TCP(ploc)->urgentPointer = hton16( TCP(ploc)->urgentPointer );

    // Look up the socket to which this packet is addressed.
    struct SockBase* sock = matchSocket( stream, ploc );

    // Check if a socket was found.
    if( !sock )
    {
        // Refuse the connection, since no corresponding socket was found.
        vTcpRefuse( ploc );

        // Drop the packet
        return;
    }

    // Do not perform a port and address comparison if the socket is in
    // the "Listen" state
    if( TCPSOCK(sock)->socketState != Listen )
    {
        if( TCP(ploc)->SYN ||
                TCP(ploc)->srcePort != sock->remotePort ||
                !swaddr_cmp( &IPv4(ploc)->sip, &sock->remoteAddress ) )
        {
            // Refuse the connection; the socket is not listening
            vTcpRefuse( ploc );

            // Incorrect source address, drop the packet.
            return;
        }
    }

    // Switch behaviour based on the state of the socket
    switch( TCPSOCK(sock)->socketState )
    {
    case Closed:
        TCPSOCK(sock)->socketState = Listen;
    case Listen:
        // Check if receiving a request to connect.
        if( TCP(ploc)->SYN )
        {
            // Copy in the sequence information from the client
            TCPSOCK(sock)->remoteSequence = TCP(ploc)->sequence;
            TCPSOCK(sock)->ackToRemSeq = TCP(ploc)->sequence;

            // Recored the peer's port
            sock->remotePort = TCP(ploc)->srcePort;

            // Record the peer's IP, sip has already been byte reordered.
            sock->remoteAddress = IPv4(ploc)->sip;

            // Record the peer's current window size
            TCPSOCK(sock)->peerWinSize = TCP(ploc)->windowSize;

            // Generate a pseudo random 32bit number
            TCPSOCK(sock)->localSequence = vrand();
            TCPSOCK(sock)->ackedSequence = TCPSOCK(sock)->localSequence;

            if( sock->handler )
            {
                // Call the socket handler to accept or reject the connection.
                sock->handler( sock->sfd, eNewConnection, 0 );
            }
            else
            {
                // By the protocol, a SYN packet counts as one byte in the
                // sequence
                ++TCPSOCK(sock)->localSequence;

                // Send a SYN & ACK to accept the packet on behalf of the
                // server application, since no handler was given.
                vTcpSend( sock->sfd, f_Syn | f_Ack, 0, 0, 0, 0 );

                // Save the new event time.
                TCPSOCK(sock)->rexmitTimer = msTime();

                // Increment the rexmit count
                TCPSOCK(sock)->rexmitCount = 0;

                // Set the socket's state
                TCPSOCK(sock)->socketState = SyncReceived;
            }
        }
        break;
    case SyncSent:
        // todo: Implement this state.
        // Client only state.
        break;
    case SyncReceived:
        if( TCP(ploc)->ACK )
        {
            if( TCP(ploc)->ACKnumber == TCPSOCK(sock)->localSequence )
            {
                // Update the most recently recevied ack
                TCPSOCK(sock)->ackedSequence = TCP(ploc)->ACKnumber;

                // Propigate the sockets state to an established state.
                TCPSOCK(sock)->socketState = Established;

                if( sock->handler )
                {
                    // Call the socket handler, indicating the event is an
                    // established connection.
                    sock->handler( sock->sfd, eConnected, 0 );
                }
            }
        }
        break;
    case Established:
        // Verify this packet acknowledges the correct sequence we have sent as
        // well as contains the correct sequence we expected to receive.
        if( TCP(ploc)->ACKnumber == TCPSOCK(sock)->localSequence &&
                TCP(ploc)->sequence == TCPSOCK(sock)->ackToRemSeq )
        {
            // Remove data that was sent by this ipstack and has been
            // acknowledged by the peer that we are communicating with.
            if( TCP(ploc)->ACK && vBufGetPending( TCPSOCK(sock)->txbfd ) )
            {
                receiveAck( sock, ploc );
            }

            // Length of the received packet
            // Calculate the data length of the TCP packet
            // note: IPv4(ploc)->TotalLength was byte reordered within ipv4
            // when the packet was received.
            int length = IPv4(ploc)->TotalLength -
                    ( IPv4(ploc)->HeaderLength + TCP(ploc)->dataOffset ) * 4;

            if( length )
            {
                // Save the data to the buffer. Update the peer's sequence
                // to what we have received and could write to the buffer.
                // This data will be acked during the socket's periodic
                // kernel
                TCPSOCK(sock)->remoteSequence +=
                        vWriteBuf( TCPSOCK(sock)->rxbfd,
                                   TCPpayload(ploc), length );

                // ack the packet here
                vTcpSend( sock->sfd, f_Ack, 0, 0, 0, 0 );

                if( TCP(ploc)->PSH && sock->handler )
                {
                    sock->handler( sock->sfd, ePushedData, 0 );
                }
            }

            // Check if the client is initiating a close
            if( TCP(ploc)->FIN )
            {
                // Remote sent a FIN packet, this is equevalent to 1 byte in
                // the sequence
                ++TCPSOCK(sock)->remoteSequence;

                // We are closing the connection, drop any data we were sending.
                vTcpDropData( sock );

                // Send an acknowledge of the request to close.
                vTcpSend( sock->sfd, f_Ack, 0, 0, 0, 0 );

                // By the protocol, a FIN packet counts as one byte in the
                // sequence.
                ++TCPSOCK(sock)->localSequence;

                // Send a request to close
                vTcpSend( sock->sfd, f_Fin | f_Ack, 0, 0, 0, 0 );

                // Save the new event time.
                TCPSOCK(sock)->rexmitTimer = msTime();

                // Increment the rexmit count
                TCPSOCK(sock)->rexmitCount = 0;

                // Set the state to closing to wait for an acknowledgement from
                // the peer
                TCPSOCK(sock)->socketState = CloseWait;
            }
        }
        else if( TCP(ploc)->sequence + 1 == TCPSOCK(sock)->ackToRemSeq )
        {
            vTcpSend( sock->sfd, f_Ack, 0, 0, 0, 0 );
        }

        // End case "Established"
        break;

    case FinWait1:
        // This state indicates this tcp/ip stack has sent a FIN+ACK and it is
        // closing the connection.

        // Check if the peer is acking the FIN+ACK this tcp/ip stack sent.
        if( TCP(ploc)->ACK )
        {

            vTcpDropData( sock );

            // Verify the ack sequence is correct
            if( TCP(ploc)->ACKnumber == TCPSOCK(sock)->localSequence )
            {
                // Update the most recently recevied ack
                TCPSOCK(sock)->ackedSequence = TCP(ploc)->ACKnumber;

                // Check if this packet is also a FIN+ACK packet
                if( TCP(ploc)->FIN )
                {
                    // By the protocol, a SYN packet counts as one byte in the
                    // sequence
                    ++TCPSOCK(sock)->remoteSequence;

                    // sending an ACK of the FIN+ACK packet
                    vTcpSend( sock->sfd, f_Ack, 0, 0, 0, 0 );

                    // Save the new event time.
                    TCPSOCK(sock)->rexmitTimer = msTime();

                    // Increment the rexmit count
                    TCPSOCK(sock)->rexmitCount = 0;

                    // The packet was a FIN+ACK, going immediatly to the
                    // closed state (not implementing TimeWait case)
                    TCPSOCK(sock)->socketState = Closed;

                    if( sock->handler )
                    {
                        sock->handler( sock->sfd, eDisconnected, 0 );
                    }
                }
                else
                {
                    // Going to the FinWait2 state to wait for the peer's
                    // FIN packet.
                    TCPSOCK(sock)->socketState = FinWait2;
                }
            }
        }
        // End case "FinWait1"
        break;
    case FinWait2:
        // This state indicates this tcp/ip stack has sent a FIN+ACK and it is
        // closing the connection. It also has received an ACK to the
        // transmitted FIN+ACK sent by this tcp/ip stack.

        // Verify this is a FIN packet
        if( TCP(ploc)->FIN )
        {
            // Verify the correct acknumber was sent
            if( TCP(ploc)->ACKnumber == TCPSOCK(sock)->localSequence )
            {
                // Update the most recently recevied ack
                TCPSOCK(sock)->ackedSequence = TCP(ploc)->ACKnumber;

                // sending an ACK of the FIN+ACK packet
                vTcpSend( sock->sfd, f_Ack, 0, 0, 0, 0 );

                // Save the new event time.
                TCPSOCK(sock)->rexmitTimer = msTime();

                // Increment the rexmit count
                TCPSOCK(sock)->rexmitCount = 0;

                // The packet was a FIN+ACK, going immediatly to the
                // closed state (not implementing TimeWait case)
                TCPSOCK(sock)->socketState = Closed;

                if( sock->handler )
                {
                    sock->handler( sock->sfd, eDisconnected, 0 );
                }
            }
        }


        // End case "FinWait2"
        break;
    case CloseWait:
        // Allow this case to drop through to case "Closing"
    case Closing:
        if( TCP(ploc)->ACK )
        {
            // Check if the correct ack number was received.
            if( TCP(ploc)->ACKnumber == TCPSOCK(sock)->localSequence )
            {
                TCPSOCK(sock)->ackedSequence = TCP(ploc)->ACKnumber;

                // Clear information about the remote port and address.
                sock->remotePort = 0;
                swaddr_cpy( &sock->remoteAddress, 0 );

                // Clear out any sequence information
                TCPSOCK(sock)->ackedSequence = 0;
                TCPSOCK(sock)->localSequence = 0;

                // Set the state to closing to wait for an acknowledgement from
                // the peer
                TCPSOCK(sock)->socketState = Closed;

                if( sock->handler )
                {
                    sock->handler( sock->sfd, eDisconnected, 0 );
                }
            }
        }
        break;
    }
}

/*****************************************************************************
 * void vTCPsend( vPort, tcpSckt, flgs, optData, optSize, tcpData, tcpSize )
 * 		This function builds the TCP packet based on the parameters passed,
 * 	vPort is the IP address from which the packet will be sent, tcpSckt is the
 *  source socket of the TCP packet, flgs is the flags of the packet that will
 *  be set ( these are 'ored' together ), optData is the location of data
 *  comprising the options portion of a TCP packet and optSize the size of the
 *  options, tcpData is the location of the TCP data and tcpSize is the size
 *  of the data being pointed to by tcpData.
 *****************************************************************************/
bool vTcpSend( int sfd, int flgs,
               const void *optData, int optSize,
               const void *tcpData, int tcpSize )
{
    void* ploc;
    uint32_t loadSize;
    struct SockBase *sock = sfdtosocket( sfd );
    int interface;

    if( !sock )
    {
        return false;
    }

    // Setup the size of of the tcp packet
    loadSize = optSize + tcpSize;

    // Allocate memory for the packet that is to be sent
    ploc = ucalloc( sizeof(tagMACgram) + sizeof(tagIPgram) +
                    sizeof(tagTCPgram) + loadSize );

    // If allocation was successful
    if( ploc )
    {
        vmemcpy( ploc, 0, sizeof(tagMACgram) + sizeof(tagIPgram) +
                 sizeof(tagTCPgram) + loadSize );

        // Initialize specific memory locations to zero: ipv4 checksum &
        // headerLength & tcp dataOffset & checksum
        // IPv4 Header length must be zero before using "TCP(ploc)"
        IPv4(ploc)->HeaderLength = 5;
        IPv4(ploc)->Checksum = 0;
        TCP(ploc)->dataOffset = 5;
        TCP(ploc)->checksum = 0;

        /* Set the destination port to the Client's port */
        TCP(ploc)->destPort = sock->remotePort;
        /* Set the source port to the Server's port */
        TCP(ploc)->srcePort = sock->localPort;
        /* Return the acknowledgment number */
        TCPSOCK(sock)->ackToRemSeq = TCPSOCK(sock)->remoteSequence + 1;
        TCP(ploc)->ACKnumber = TCPSOCK(sock)->ackToRemSeq;

        // Set the sequence number according to what is being sent.
        if( tcpSize )
        {
            // Subtract the size of the tcpSize from the sequence.
            TCP(ploc)->sequence = TCPSOCK(sock)->localSequence - tcpSize;
        }
        else
        {
            // Subtract one from the sequence if the Fin or Syn flag is set,
            // otherwise do not subtract one.
            TCP(ploc)->sequence = TCPSOCK(sock)->localSequence -
                    ( flgs & ( f_Fin | f_Syn ) ? 1 : 0 );
        }

        // Always clear out the reserved portion
        TCP(ploc)->reserved = 0;

        // If the signal was passed, then that signal will be set in the packet
        TCP(ploc)->FIN = flgs & f_Fin ? true : false;
        TCP(ploc)->SYN = flgs & f_Syn ? true : false;
        TCP(ploc)->RST = flgs & f_Rst ? true : false;
        TCP(ploc)->PSH = flgs & f_Psh ? true : false;
        TCP(ploc)->ACK = flgs & f_Ack ? true : false;
        TCP(ploc)->URG = flgs & f_Urg ? true : false;
        TCP(ploc)->ECE = flgs & f_Ece ? true : false;
        TCP(ploc)->CWR = flgs & f_Cwr ? true : false;

        // Send the size of the window
        TCP(ploc)->windowSize = vBufGetFree( TCPSOCK(sock)->rxbfd );

        /* If options are needed, add them to the packet */
        if( optSize )
        {
            TCP(ploc)->dataOffset = 5 + ( optSize / 4 );

            if( optSize % 4 )
                TCP(ploc)->dataOffset++;

            // Copy the TCP options
            vmemcpy( TCPoptions(ploc), optData, optSize );
        }
        else
        {
            // Offset of the data no options required
            TCP(ploc)->dataOffset = 5;
        }

        /* If data is being sent, add it to the packet */
        if( tcpSize )
        {
            // Copy the TCP data
            vmemcpy( TCPpayload(ploc), tcpData, tcpSize );
        }

        if( addrtointerface( sock->localAddress, &interface ) )
        {
            TCP(ploc)->srcePort = hton16( TCP(ploc)->srcePort );
            TCP(ploc)->destPort = hton16( TCP(ploc)->destPort );
            TCP(ploc)->sequence = hton32( TCP(ploc)->sequence );
            TCP(ploc)->ACKnumber = hton32( TCP(ploc)->ACKnumber );
            TCP(ploc)->windowSize = hton16( TCP(ploc)->windowSize );
            TCP(ploc)->urgentPointer = hton16( TCP(ploc)->urgentPointer );

            // Send the packet to the IP portion
            if( vIPdeal( interface, sock->remoteAddress,
                         sizeof(tagTCPgram) + loadSize, ploc, TCPTYPE ) )
            {
                ucfree( ploc );
                ploc = null;
            }

            return true;
        }

        ucfree( ploc );
        ploc = 0;
    }

    return false;
}

/*!
 * \brief vTCPrefuse Replies to a tcp sync packet with the correct information
 * to say "Connection refused"
 * \param rxploc
 * \return
 */
bool vTcpRefuse( const void *rxploc )
{
    int interface;

    // Look up the interface associated with the target address
    if( addrtointerface( IPv4(rxploc)->tip, &interface ) )
    {
        // Allocate memory to contain the "connection refused" packet
        void* txploc = ucalloc( sizeof( tagMACgram ) +
                                sizeof( tagIPgram ) +
                                sizeof( tagTCPgram ) );

        // Validate the allocation was successful
        if( txploc )
        {
            // Initialize the specific fields within the new packet.
            IPv4(txploc)->HeaderLength = 5;
            IPv4(txploc)->Checksum = 0;
            TCP(txploc)->dataOffset = 5;
            TCP(txploc)->checksum = 5;
            TCP(txploc)->reserved = 0;

            // Copy the Information from the rxpacket in order to reply back
            // with the correct information for a "connection refused".
            TCP(txploc)->destPort = TCP(rxploc)->srcePort;
            TCP(txploc)->srcePort = TCP(rxploc)->destPort;
            TCP(txploc)->ACKnumber = TCP(rxploc)->sequence + 1;

            // Generate a random sequence to be sent with the connection
            // refused.
            TCP(txploc)->sequence = vrand();

            // Set the appropriate flags for a "connection refused"
            TCP(txploc)->RST = true;
            TCP(txploc)->ACK = true;
            TCP(txploc)->FIN = false;
            TCP(txploc)->SYN = false;
            TCP(txploc)->PSH = false;
            TCP(txploc)->URG = false;
            TCP(txploc)->ECE = false;
            TCP(txploc)->CWR = false;

            // Set the window size to zero
            TCP(txploc)->windowSize = 0;

            // Perform byte reordering for the Tcp header
            TCP(txploc)->srcePort = hton16( TCP(txploc)->srcePort );
            TCP(txploc)->destPort = hton16( TCP(txploc)->destPort );
            TCP(txploc)->sequence = hton32( TCP(txploc)->sequence );
            TCP(txploc)->ACKnumber = hton32( TCP(txploc)->ACKnumber );
            TCP(txploc)->windowSize = hton16( TCP(txploc)->windowSize );
            TCP(txploc)->urgentPointer = hton16( TCP(txploc)->urgentPointer );

            // Give the packet to the IPv4 api to be sent over the wire.
            if( !vIPdeal( interface, IPv4(rxploc)->sip,
                          sizeof(tagTCPgram), txploc, TCPTYPE ) )
            {
                return true;
            }

            // IPv4 API did not take control of the packet, deallocate the
            // packet before returning.
            ucfree( txploc );
            txploc = 0;
        }
    }

    return false;
}

void vTcpReject( int sfd )
{
    struct SockBase * sock = sfdtosocket( sfd );

    vTcpSend( sock->sfd, f_Rst | f_Ack, 0, 0, 0, 0 );

    // Clear out information about the socket.
    swaddr_cpy( &sock->remoteAddress, 0 );
    sock->remotePort = 0;
    TCPSOCK(sock)->localSequence = 0;
    TCPSOCK(sock)->ackedSequence = 0;
}

/*******************************************************************
 * bool vTCPchecksum( void* ploc )
 * 		This function is designed to calculate the checksum,
 * according to the TCP standard, and place the calculated value
 * into the correct position. The function will return true if the
 * checksum that existed matches the calculated value, and FALE if
 * it did not match.
 * 		ploc is the starting location of IPV4 header. This should
 * be called from the IPV4 API. Ignore the return value of this
 * function when sending a packet, do not ignore the return value
 * when receiving a packet.
 *******************************************************************/
uint16_t vTCPchecksum( const void *ploc )
{
    // Get a fast variable to hold the checksum result
    register uint32_t sum = 0;

    // Create a readonly 16bit pointer to the packet location
    const uint16_t *addr = (uint16_t *)TCP(ploc);

    // Calculate the IPv4 payload
    // byte reorder will not have been done yet, since this checksum will be
    // evaluated within the ip interface
    int count = hton16( IPv4(ploc)->TotalLength ) -
            IPv4(ploc)->HeaderLength * 4;

    // Add the pseudo header to the checksum
    sum += ( (uint16_t *)&(IPv4(ploc)->sip) )[0];
    sum += ( (uint16_t *)&(IPv4(ploc)->sip) )[1];
    sum += ( (uint16_t *)&(IPv4(ploc)->tip) )[0];
    sum += ( (uint16_t *)&(IPv4(ploc)->tip) )[1];
    sum += ( ( IPv4(ploc)->Protocol<<8 ) + hton16( count ) );

    // Loop through summing the tcp packet
    while( count > 1 )
    {
        sum += *addr++;
        count -= 2;
    }

    // If the packet was an odd number of bytes, this adds the leftover byte.
    if( count > 0 )
    {
        // Done this way so "an invalid read of size 1" does not occur.
        sum += *( (unsigned char *)addr );
    }

    while( sum >> 16 )
    {
        // Add the overflow values to the 16 bit checksum
        sum = ( sum & 0x0000ffff ) + ( sum >> 16 );
    }

    return ~sum;
}

void receiveAck( struct SockBase *sock, const void *ploc )
{
    // todo: Push byte reordering to the first receiving of the packet
    uint32_t acknum  = TCP( ploc )->ACKnumber;

    if( TCPSOCK(sock)->socketState == Established )
    {
        // calculate how many bytes the remote client is acking
        int acking = acknum - TCPSOCK(sock)->ackedSequence;

        if( acking > 0 )
        {
            // Peer is acknowledging "acking" number of bytes that were sent
            // from this tcp/ip stack

            // Null read from the transmit buffer. This data has been
            // acknowledged by the peer.
            vReadBuf( TCPSOCK(sock)->txbfd, 0, acking );

            // Mark the sequence as having been acknowledged
            TCPSOCK(sock)->ackedSequence += acking;
        }
        else if( acking == 0 )
        {
            // This packet is not acknowledging any new data that was sent
            // from this tcp/ip stack.
        }
    }
    else
    {
        TCPSOCK(sock)->ackedSequence++;
    }
}

void vTcpDropData( struct SockBase * sock )
{
    // Back out any pending data from the sequence
    TCPSOCK(sock)->localSequence -= vBufGetPending( TCPSOCK(sock)->txbfd );

    // Drop any pending data sitting in the txbfd
    vReadBuf( TCPSOCK(sock)->txbfd, 0, vBufGetPending( TCPSOCK(sock)->txbfd ) );

    // Check if overflow data exists
    if( TCPSOCK(sock)->of )
    {
        // Free the overflow pointer
        ucfree( TCPSOCK(sock)->of );

        // Clear the overflow pointer
        TCPSOCK(sock)->of = 0;
    }

    // Clear the overflow size
    TCPSOCK(sock)->ofsize = 0;
}

/*!
 * \brief vTcpDisconnect Initiates a close of the Tcp Connection. Any pending
 * data which has not been acked can not be guaranteed to reach the peer. Any
 * data that has been buffered and not transmitted will be lost.
 * \param sock Pointer to the socket
 */
void vTcpDisconnect( struct SockBase* sock )
{
    if( sock && TCPSOCK(sock)->socketState == Established )
    {
        // Drop any data waiting to be acked or to be transmitted data, this
        // will adjust the value of the local sequence.
        vTcpDropData( sock );

        // Set the new state of the socket
        TCPSOCK(sock)->socketState = FinWait1;

        // Increment the sequence to account for the f_Fin flag
        ++TCPSOCK(sock)->localSequence;

        // Send the FIN+ACK packet to disconnect this end of the connection
        vTcpSend( sock->sfd, f_Fin | f_Ack, 0, 0, 0, 0 );

        // Save the time of this event
        TCPSOCK(sock)->rexmitTimer = msTime();

        // Clear the rexmitCount to zero
        TCPSOCK(sock)->rexmitCount = 0;
    }
}

// Attempts to fill the txbfd and send what is remaining
void vTcpFlush( struct SockBase* sock )
{
    // Only allow this operation to take place if the connection is
    // Established (connected).
    if( sock &&
            TCPSOCK(sock)->socketState == Established &&
            TCPSOCK(sock)->ofsize )
    {
        // Get the number of bytes free in the txbfd (space available)
        int count = vBufGetFree( TCPSOCK(sock)->txbfd );

        // Space was available & enough data is waiting
        if( count )
        {
            if( TCPSOCK(sock)->ofsize < count )
            {
                // Can only write what is available.
                count = TCPSOCK(sock)->ofsize;
            }

            // Copy the data into the transmit buffer
            count = vWriteBuf( TCPSOCK(sock)->txbfd, TCPSOCK(sock)->of, count );

            // When sending data, the sequence must be adjusted prior
            // to the send.
            TCPSOCK(sock)->localSequence += count;

            // Send the data on the wire.
            vTcpSend( sock->sfd, f_Ack, 0, 0, TCPSOCK(sock)->of, count );

            // Update the event time stamp & retransmit count
            TCPSOCK(sock)->rexmitTimer = msTime();
            TCPSOCK(sock)->rexmitCount = 0;

            // Update the overflow buffers
            TCPSOCK(sock)->ofsize -= count;

            if( TCPSOCK(sock)->ofsize )
            {
                // Shift down the data.
                vmemcpy( TCPSOCK(sock)->of,
                         TCPSOCK(sock)->of + count,
                         TCPSOCK(sock)->ofsize );

                // Shrink the overflow buffer to the new size.
                TCPSOCK(sock)->of = (char *)ucrealloc(
                            TCPSOCK(sock)->of, TCPSOCK(sock)->ofsize );
            }
            else
            {
                // Deallocate the memory of the overflow buffer
                ucfree( TCPSOCK(sock)->of );

                // Clear the overflow buffer's pointer
                TCPSOCK(sock)->of = 0;
            }
        }
    }
}

int vTcpWrite( struct SockBase* sock, const void *data, int size )
{
    if( sock && TCPSOCK(sock)->socketState == Established )
    {
        // Create a temp pointer for resizing
        void* temp = ucrealloc( TCPSOCK(sock)->of,
                                TCPSOCK(sock)->ofsize + size );

        // Check the return of the resize
        if( temp )
        {
            // Set temp to the socket's overflow pointer
            TCPSOCK(sock)->of = (char *)temp;

            // Copy the data into the end of the reallocated buffer.
            vmemcpy( TCPSOCK(sock)->of + TCPSOCK(sock)->ofsize, data, size );

            // Increase the overflow size
            TCPSOCK(sock)->ofsize += size;

            // Return the size that was written.
            return size;
        }
    }

    // No bytes were written, return the effect of that.
    return 0;
}

void vTcpTick( struct SockBase* sock )
{
    // See if we are expecting data to be acked
    if( TCPSOCK(sock)->ackedSequence !=
            TCPSOCK(sock)->localSequence )
    {
        // tolerance algorithem
        int tolerance = 100 + ( 10 * TCPSOCK(sock)->rexmitCount );

        // When the tolerance is 4 minuets
        if( TCPSOCK(sock)->rexmitCount >= 23900 )
        {
            TCPSOCK(sock)->socketState = Listen;

            // Clear information about the remote port and address.
            sock->remotePort = 0;
            swaddr_cpy( &sock->remoteAddress, 0 );

            // Clear out any sequence information
            TCPSOCK(sock)->ackedSequence = 0;
            TCPSOCK(sock)->localSequence = 0;

            // Flush any pending xmit data
            if( TCPSOCK(sock)->of )
            {
                ucfree( TCPSOCK(sock)->of );
                TCPSOCK(sock)->of = 0;

                TCPSOCK(sock)->ofsize = 0;
            }

            vReadBuf( TCPSOCK(sock)->txbfd, 0,
                      vBufGetPending( TCPSOCK(sock)->txbfd ) );
        }

        // If the packet has gone un-acknowledged for 1 or more mS
        if( TCPSOCK(sock)->rexmitTimer + tolerance <= msTime() )
        {
            // Determine what needs to be retransmitted
            switch( TCPSOCK(sock)->socketState )
            {
            case SyncReceived:
                // Retransmit the SYN+ACK
                vTcpSend( sock->sfd, f_Syn | f_Ack, 0, 0, 0, 0 );

                // Save the new event time.
                TCPSOCK(sock)->rexmitTimer = msTime();

                // Increment the rexmit count
                ++TCPSOCK(sock)->rexmitCount;

                break;

            case Established:
            {
                // Get the pending amount of data
                int count = vBufGetPending( TCPSOCK(sock)->txbfd );

                // Check if count has a non zero value.
                if( count )
                {
                    // Allocate a buffer to hold data to transmit
                    void* temp = ucalloc( count );

                    // Validate the return value of the allocation
                    if( temp )
                    {
                        // Peek all data from the queue
                        count = vPeekBuf(
                                    TCPSOCK(sock)->txbfd, temp, count );

                        // Check that count has a non zero value
                        if( count )
                        {
                            // No sequence adjustment needs to take place, the
                            // sequence should already be correctly adjusted,

                            // Buffer is full, this is prob not a push
                            vTcpSend( sock->sfd, f_Ack, 0, 0, temp, count );

                            // Save the new event time.
                            TCPSOCK(sock)->rexmitTimer = msTime();

                            // Increment the rexmit count
                            ++TCPSOCK(sock)->rexmitCount;
                        }

                        // deallocate the pointer, temp
                        ucfree( temp );
                    }
                }

                break;
            }

            case CloseWait:
            case Closing:
            case FinWait1:
                // Retransmit the FIN+ACK
                vTcpSend( sock->sfd, f_Fin | f_Ack, 0, 0, 0, 0 );

                // Save the time of this event.
                TCPSOCK(sock)->rexmitTimer = msTime();

                // Increment the retransmit counter
                ++TCPSOCK(sock)->rexmitCount;
                break;

            default:
                break;
            }
        }
    }

    // Attempt to flush any pending data that has not been transmitted.
    vTcpFlush( sock );
}

void vTcpAccept( int sfd )
{
    struct SockBase* sock = sfdtosocket( sfd );

    if( sock )
    {
        // By the protocol, a SYN packet counts as one byte in the
        // sequence
        ++TCPSOCK(sock)->localSequence;

        // Send a SYN & ACK to accept the packet on behalf of the
        // server application, since no handler was given.
        vTcpSend( sock->sfd, f_Syn | f_Ack, 0, 0, 0, 0 );

        // Save the new event time.
        TCPSOCK(sock)->rexmitTimer = msTime();

        // Increment the rexmit count
        TCPSOCK(sock)->rexmitCount = 0;

        // Set the socket's state
        TCPSOCK(sock)->socketState = SyncReceived;
    }
}

int vTcpCopy( int sfd_orig )
{
    // Get a pointer to the origional socket
    struct SockBase* orig = sfdtosocket( sfd_orig );

    // Verify the return from the socket translation. Only alow the copy if
    // the socket is in a listening state.
    if( orig && TCPSOCK(orig)->socketState == Listen )
    {
        // Create a copy of the origional socket
        struct SockBase* sock = sfdtosocket( vSocketCopy( orig->sfd ) );

        // vSocketCopy CAN resize the socket stack moving where pointers exists
        // regrab the pointer to the socket.
        orig = sfdtosocket( sfd_orig );

        // Verify the return from the socket copy and lookup
        if( sock )
        {
            // Allocate memory to contain the new copy of the socket
            sock->extention = ucalloc( sizeof( struct tcpSockExt ) );

            // Check that the allocation was successful
            if( sock->extention )
            {
                // Copy the origional extention to the new extention
                vmemcpy( sock->extention, orig->extention,
                         sizeof( struct tcpSockExt ) );

                // Request a transmit buffer
                TCPSOCK(sock)->rxbfd = vOpenBuf(
                            vBufGetSize( TCPSOCK(orig)->rxbfd ) );

                // Request a receive buffer
                TCPSOCK(sock)->txbfd = vOpenBuf(
                            vBufGetSize( TCPSOCK(orig)->txbfd ) );

                // Check that the buffers were allocated correctly
                if( TCPSOCK(sock)->rxbfd && TCPSOCK(sock)->txbfd )
                {
                    // Ensure the overflow buffer is initialized
                    TCPSOCK(sock)->of = 0;
                    TCPSOCK(sock)->ofsize = 0;

                    // Return the newly copied socket.
                    return sock->sfd;
                }
                else
                {
                    // Check if the receive buffer was created
                    if( TCPSOCK(sock)->rxbfd )
                    {
                        // Close the receive buffer
                        vCloseBuf( TCPSOCK(sock)->rxbfd );
                    }

                    // Check if the transmit buffer was creaed
                    if( TCPSOCK(sock)->txbfd )
                    {
                        // Close the transmit buffer
                        vCloseBuf( TCPSOCK(sock)->txbfd );
                    }

                }

                // Deallocated the memory on the sock->extention
                ucfree( sock->extention );

                // Clear the sock extention
                sock->extention = 0;
            }

            // Close the socket
            vCloseSocket( sock->sfd );
        }
    }

    return 0;
}
