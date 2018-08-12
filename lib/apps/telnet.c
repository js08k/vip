#include "telnet.h"
#include "vheap.h"
#include "vsocket.h"
#include "vfilter.h"
#include "vconfig.h"
#include "vipv4.h"
#include "vstring.h"
#include "vtcp.h"
#include "shell.h"
#include "varp.h"

#define cr 0x0d
#define lf 0x0a

//Telnet commands
#define TELNET_IAC   0xff
#define TELNET_WONT  0xfc

#define MaxTelnetCon 5

struct telnetSession
{
    // Socket file descriptor to which the connection belongs
    int sfd;

    // Buffer to hold the latest Telnet command
    unsigned char *buffer;

    // Size of the current buffered request
    int size;
};

struct telnetSession telnetSessionList[MaxTelnetCon];

struct telnetSession* sfdToTelnetSession( int sfd )
{
    int iterrator;
    for( iterrator = 0; iterrator < MaxTelnetCon; ++iterrator )
    {
        if( telnetSessionList[iterrator].sfd == sfd )
        {
            return &telnetSessionList[iterrator];
        }
    }

    return 0;
}

void vInitializeTelnet( void )
{
    int i;
    for( i = 0; i < MaxTelnetCon; ++i )
    {
        telnetSessionList[i].sfd = 0;
        telnetSessionList[i].buffer = 0;
        telnetSessionList[i].size = 0;
    }

    struct ipv4_addr myaddr;

    if( interfacetoaddr( &myaddr, 0 ) )
    {
        int telnetsfd;

        // Create a socket
        telnetsfd = vSocket( stream, telnetCallback, 256 );

        // Initialize the socket
        vBind( telnetsfd, myaddr.address, TELNET );

        // Initialize the shell to be used by the telnet session
        vShellInit();

        // Add commands to the shell
        vShellAdd( "?", vhelp );
        vShellAdd( "arp", vARPprint );
        vShellAdd( "socket", vSockPrint );
        vShellAdd( "route", vIPprint );
        vShellAdd( "ifup", vIPadd );
        vShellAdd( "ifdown", vIPdel );
        vShellAdd( "memcheck", memcheck );
        vShellAdd( "filter", vFilter );
        vShellAdd( "exit", telnetExit );
    }
}

void telnetCallback( int sfd, int event, const void *voidarg )
{
    (void)voidarg;
    // Look up the session that will correspond to the sfd
    struct telnetSession* session = sfdToTelnetSession( sfd );

    switch( event )
    {
    case eNewConnection:
        session = sfdToTelnetSession( 0 );

        if( session )
        {
            // Copy the server's listening socket
            session->sfd = vTcpCopy( sfd );

            // Connect to the peer using the copied socket
            vTcpAccept( session->sfd );

            // End the call
            return;
        }

        // Reject the connection on the sfd
        vTcpReject( sfd );

        break;
    case eConnected:
        // Send an intro and a prompt
        telnetPrint( "StatPrompt 1.0.1\n> ", sfd );
        break;
    case eDisconnected:
        if( session )
        {
            // Close the socket
            vCloseSocket( session->sfd );

            // Ensure old data isn't in the session
            if( session->buffer )
            {
                ucfree( session->buffer );
                session->buffer = 0;
            }

            // Clear out the session size
            session->size = 0;

            // Clear the socket file descriptor
            session->sfd = 0;
        }
        break;
    case ePushedData:
        if( session )
        {
            if( telnetReadIn( session ) )
            {
                telnetParseControls( session );

                telnetZeroDelim( session );

                int i;
                int argc = 0;
                char **argv = 0;

                for( i = 0; i < session->size; ++i )
                {
                    if( ( i == 0 && session->buffer[i] ) ||
                            ( i > 0 && session->buffer[i] &&
                              !session->buffer[i - 1] ) )
                    {
                        void *temp;

                        // Resize our allocation for the new pointer
                        temp = ucrealloc( argv, ( argc + 1 ) * sizeof(char *) );

                        if( !temp )
                        {
                            break;
                        }

                        argv = (char **)temp;

                        argv[argc++] = (char *)&session->buffer[i];
                    }
                }

                if( argv )
                {
                    parse( session->sfd, argc, argv );

                    // Send a prompt
                    telnetPrint( "> ", session->sfd );

                    ucfree( argv );
                }

                if( session->buffer )
                {
                    ucfree( session->buffer );
                    session->buffer = 0;
                    session->size = 0;
                }
            }
        }

        break;
    }
}

bool telnetReadIn( struct telnetSession* session )
{
    // Default the return value to false.
    bool retValue = false;

    // Query how much data is pending in the receive buffer
    int pending = vSockPend( session->sfd );

    // Verify there is data pending
    if( pending )
    {
        // Reallocate to hold the full amount of pending data
        void *temp = ucrealloc( session->buffer, session->size + pending );

        if( !temp )
        {
            // Unable to reallocate, try again later
            return false;
        }

        // Save the temp buffer
        session->buffer = (unsigned char*)temp;

        // Peak the data so we can look for the command end and not remove past
        // the command end
        vSockPeek( session->sfd, session->buffer + session->size, pending );

        int i;

        // Look through the data that was read
        for( i = session->size; i < session->size + pending; ++i )
        {
            // Verify we will not cause a bad read and look for our end command
            // sequence to have occured.
            if( i - 1 >= 0 &&
                    session->buffer[i] == lf && session->buffer[i-1] == cr )
            {
                // Set the return value to true, showing we found our end
                // command sequence.
                retValue = true;

                // Break here, leave the rest of the data in the receive buffer
                // for a later read.
                break;
            }
        }

        // Change the pending value to what we are willing to "read"
        pending = i - session->size;

        // Remove the data from the buffer that we got from the peek read and
        // we want to keep
        vSockRead( session->sfd, 0, pending );

        // Add the pending bytes to the size of the session's size
        session->size += pending;

        // This is always a shrink operation or a no change, will not fail.
        session->buffer = (unsigned char *)ucrealloc( session->buffer,
                                                      session->size );
    }

    // return the result of this function
    return retValue;
}

void telnetParseControls( struct telnetSession* session )
{
    // Counter to track bytes that were involved with telnet controls
    int j = 0;
    int i;

    // Cycle through all the bytes in the session buffer
    for( i = 0; i + j < session->size; ++i )
    {
        // If the character is an control start character
        if( session->buffer[ i + j ] == TELNET_IAC )
        {
            // Verify the following two byte to describe the control exists
            if( i + j + 2 < session->size )
            {
                // Increment j to account for
                j = j + 2;
                telnetSendChar( session->sfd, TELNET_IAC );
                telnetSendChar( session->sfd, TELNET_WONT );
                telnetSendChar( session->sfd, session->buffer[ i + j ] );
            }

            --i;
            ++j;
        }
        else
        {
            session->buffer[ i ] = session->buffer[ i + j ];
        }
    }

    session->size -= j;

    session->buffer =
            (unsigned char *)ucrealloc( session->buffer, session->size );
}

void telnetZeroDelim( struct telnetSession *session )
{
    int i;
    for( i = 0; i < session->size; ++i )
    {
        if( session->buffer[i] == cr ||
                session->buffer[i] == lf ||
                session->buffer[i] == ' ' )
        {
            session->buffer[i] = 0;
        }
    }
}

char telnetGetChar( int sfd )
{
    // Give a default value to the character.
    char character = 0;

    // Attempt to read one character
    vSockRead( sfd, &character, 1 );

    // Return the character. If the read failed, the character is the
    // default character
    return character;
}

void telnetSendChar( int sfd, uint8_t character )
{
    vSockWrite( sfd, &character, 1 );
}

void telnetPrint(const char *string, int sfd )
{
    char *tempString = (char *)string;
    vSockWrite( sfd, tempString, vstrlen( tempString ) );
}

void telnetExit(int sfd, int argc, char *args[] )
{
    (void)argc;
    (void)args;
    vDisconnect( sfd );
}
