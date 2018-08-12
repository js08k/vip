#include "httpd.h"
#include "vipv4.h"
#include "vsocket.h"
#include "vtcp.h"
#include "shell.h"
#include "vheap.h"
#include "telnet.h"
#include "vstring.h"

#define cr 0x0d
#define lf 0x0a

struct ContentEntry
{
    char *ContentName;
    const char *ContentLocation;
    int ContentSize;
};

struct PageTable
{
    struct ContentEntry *table;
    int size;
} pageTable;

void addContent( const char* ContentName, const char *ContentLocation, int ContentSize )
{
    void *temp;

    temp = ucrealloc( pageTable.table,
                      sizeof(struct ContentEntry) * ( pageTable.size + 1 ) );

    if( temp )
    {
        pageTable.table = (struct ContentEntry *)temp;

        struct ContentEntry* entry = &pageTable.table[pageTable.size++];

        entry->ContentName = (char *)ucalloc( vstrlen( ContentName ) + 1 );

        // todo: Check return of allocation

        // Copy the string (this will include the '\0' character)
        vstrcpy( entry->ContentName, ContentName );

        // Copy the location of the content
        entry->ContentLocation = ContentLocation;

        // Copy the size of the content
        entry->ContentSize = ContentSize;
    }
}

struct httpSession
{
    // Socket File descriptor the request is coming from
    int sfd;

    // Buffer holding the httpd request
    unsigned char *buffer;

    // Size of the buffer holding the httpd request
    int size;
};

void httpdWriteContentLength( int sfd, int length );

#define MaxHttpCon 5

// List of available or active http sessions
struct httpSession httpSessionList[MaxHttpCon];

struct httpSession* sfdToHttpSession( int sfd )
{
    int iter;

    // Iterate through the sessions
    for( iter = 0; iter < MaxHttpCon; ++iter )
    {
        // Search for the first matching session
        if( httpSessionList[iter].sfd == sfd )
        {
            // Return a pointer to the session
            return &httpSessionList[iter];
        }
    }

    // Return a null pointer, no session was found.
    return 0;
}

void vInitializeHttpd( void )
{
    int i;

    // Initialize the available session list
    for( i = 0; i < MaxHttpCon; ++i )
    {
        httpSessionList[i].sfd = 0;
        httpSessionList[i].buffer = 0;
        httpSessionList[i].size = 0;
    }

    pageTable.size = 0;
    pageTable.table = 0;

    struct ipv4_addr addr;

    // Grab the information on interface 0
    if( interfacetoaddr( &addr, 0 ) )
    {
        // Int to hold the httpd server socket
        int httpdsfd;

        // Create a socket
        // todo: Test with much smaller window size
        httpdsfd = vSocket( stream, httpdCallback, 1024 );

        // Bind the socket to port 80 (http)
        vBind( httpdsfd, addr.address, 80 );
    }
}

void httpdCallback(int sfd, int event, const void * nullarg )
{
    (void)nullarg;

    // Lookup the session that will coorespond to the sfd
    struct httpSession* session = sfdToHttpSession( sfd );

    // Switch based on the event type
    switch( event )
    {
    case eNewConnection:

        // Look up the first available session
        session = sfdToHttpSession( 0 );

        if( session )
        {
            // Copy the server socket to the available session
            session->sfd = vTcpCopy( sfd );

            // Connect to the client using the copied socket
            vTcpAccept( session->sfd );
        }
        else
        {
            // Reject the connection using the server's listening socket
            vTcpReject( sfd );
        }

        break;

    case eConnected:
        break;

    case eDisconnected:
        if( session )
        {
            // Close the socket, this will deallocate the socket's resources
            vCloseSocket( session->sfd );

            // Check if the session has a allocated buffer
            if( session->buffer )
            {
                // deallocate the memory the buffer is using
                ucfree( session->buffer );

                // Zero the buffer's pointer
                session->buffer = 0;
            }

            // Set the size of the buffer back to zero
            session->size = 0;

            // Set the session's socket file descriptor to zero
            session->sfd = 0;
        }

        break;

    case ePushedData:
        if( session )
        {
            // Read in the header portion of the message from the client. This
            // will return true once it has found a null line (\r\n\r\n string)
            if( httpdReadInHeader( session ) )
            {
                // Replace all the "\r" and "\n" characters in the buffer
                // with 0. This is the first part of the string splitting.
                httpdZeroDelim( session );

                int i;
                char **argv = 0;
                int argc = 0;

                // Iterate through the bytes within the buffer, looking for
                // the beginning of strings seperated by "\0".
                for( i = 0; i < session->size; ++i )
                {
                    // Look for the beginning of the string with a valid
                    // character or a valid character with the previous
                    // character invalid (null character)
                    if( ( i == 0 && session->buffer[i] ) ||
                            ( i > 0 && session->buffer[i] &&
                              !session->buffer[i - 1] ) )
                    {
                        // Temporary pointer for checking the return of a
                        // resize
                        void *temp;

                        // Increment the client header size to hold the new
                        // string that was found.
                        ++argc;

                        // Resize our allocation for the new pointer
                        temp = ucrealloc( argv, argc * sizeof(char *) );

                        // Check if the reallocation failed
                        if( !temp )
                        {
                            // Unable to allocate for the string, decriment
                            // the client header size.
                            --argc;

                            // Leave the for loop, and continue on. There
                            // was not enough available memory
                            break;
                        }

                        // The allocation suceeded, copy the temporary
                        // pointer to the client header pointer.
                        argv = (char **)temp;

                        // Copy the location of the string into the argv
                        // pointer array
                        argv[ argc - 1 ] = (char *)&session->buffer[i];
                    }
                }

                // Check if argv has a valid pointer
                if( argv )
                {
                    // Parse the http message header
                    httpdParseHeader( session->sfd, argc, argv );

                    ucfree( argv );

                    argv = 0;

                    argc = 0;
                }

                // Check if the session buffer still has data in it
                if( session->buffer )
                {
                    // deallocate the session's buffer
                    ucfree( session->buffer );

                    // Zero the session's buffer pointer
                    session->buffer = 0;

                    // Set the size of the session's buffer to zero
                    session->size = 0;
                }
            }
        }

        break;
    }
}

bool httpdReadInHeader( struct httpSession *session )
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
            if( i - 3 >= 0 &&
                    session->buffer[i] == '\n' &&
                    session->buffer[i-1] == '\r' &&
                    session->buffer[i-2] == '\n' &&
                    session->buffer[i-3] == '\r' )
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

void httpdZeroDelim( struct httpSession *session )
{
    int iter;
    int spaceDelimCount = 0;

    // Cycle through the data in the session buffer
    for( iter = 0; iter < session->size; ++iter )
    {
        // Look for any '\r' or '\n'
        if( session->buffer[iter] == cr ||
                session->buffer[iter] == lf )
        {
            // Clear out the cr or lf character
            session->buffer[iter] = 0;
        }
        else if( session->buffer[iter] == ' ' &&
                 spaceDelimCount < 2 )
        {
            // Only allow the first to space characters to perform as string
            // deliminators.
            ++spaceDelimCount;
            session->buffer[iter] = 0;
        }
    }
}

void httpdParseHeader(int sfd, int argc , char *argv[] )
{
    if( !vstrcmp( argv[0], "GET" ) )
    {
        httpdGet( sfd, argc, argv );
    }
    else if( !vstrcmp( argv[0], "HEAD" ) )
    {
        httpdHead( sfd, argc, argv );
    }
    else if( !vstrcmp( argv[0], "POST" ) )
    {
        httpdPost( sfd, argc, argv );
    }
}

void httpdGet( int sfd, int argc, char *argv[] )
{
    (void)argc;
    const char *response = "HTTP/1.0 200\r\n";
    const char *serverField = "Server: vipdev/0.1\r\n";
    const char *contentTypeField = "Content-Type: text/html\r\n";

    int i;

    for( i = 0; i < pageTable.size; ++i )
    {
        struct ContentEntry *entry = &pageTable.table[i];

        if( vstrcmp( argv[1], entry->ContentName ) == 0 )
        {
            vSockWrite( sfd, response, vstrlen( response ) );
            vSockWrite( sfd, serverField, vstrlen( serverField ) );
            vSockWrite( sfd, contentTypeField, vstrlen( contentTypeField ) );
            httpdWriteContentLength( sfd, entry->ContentSize );
            vSockWrite( sfd, "\r\n", 2 );
            vSockWrite( sfd, entry->ContentLocation, entry->ContentSize );
            vSockFlush( sfd );
            return;
        }
    }

    const char *notfound = "HTTP/1.0 404 Not Found\r\n";
    const char *body = "<html><head>\n<title>404 Not Found</title>\n</head><body><h1>Not Found</h1>\n</body></html>\n";
    vSockWrite( sfd, notfound, vstrlen( notfound ) );
    vSockWrite( sfd, serverField, vstrlen( serverField ) );
    vSockWrite( sfd, contentTypeField, vstrlen( contentTypeField ) );
    httpdWriteContentLength( sfd, vstrlen( body ) );
    vSockWrite( sfd, "\r\n", 2 );
    vSockWrite( sfd, body, vstrlen( body ) );
    vSockFlush( sfd );
}

void httpdHead( int sfd, int argc, char *argv[] )
{
    (void)sfd;
    (void)argc;
    (void)argv;
    // todo: Implement httpdHead
}

void httpdPost( int sfd, int argc, char *argv[] )
{
    (void)sfd;
    (void)argc;
    (void)argv;
    // todo: Implement httpdPost
}

void httpdWriteContentLength( int sfd, int length )
{
    if( length < 9999 )
    {
        const char *field = "Content-Length: ";
        const char *endline = "\r\n";

        // Buffer to contain up to 9999 in ascii and a null character
        char buffer[5];

        // Copy the body size in ascii form into the content size
        vitoa( buffer, length );

        vSockWrite( sfd, field, vstrlen( field ) );
        vSockWrite( sfd, buffer, vstrlen( buffer ) );
        vSockWrite( sfd, endline, vstrlen( endline ) );
    }
}


