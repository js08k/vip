#define VERSION "1.0.1"
/*
 * Stat Prompt
 *
 * Written by:
 * 		Joshua Strickland
 *
 * 1.0.0:
 * 		Initial working version
 * 1.0.1:
 * 		Code cleanup
 */
#include "shell.h"
#include "telnet.h"
#include "vstring.h"
#include "vheap.h"
#include "vipv4.h"

static bool ucStatInitialized = false;
static struct statcmd *cmd_table;
static int cmd_count;

bool vShellInit( void )
{
    if( ucStatInitialized )
        return false;

    cmd_count = 0;

    /* Set the initialized flag */
    ucStatInitialized = true;

    return true;
}

void parse( int sfd, int argc, char *args[] )	{
    int cmd;

    for( cmd = 0; cmd < cmd_count; cmd++ )
    {
        if( vstrcmp( args[0], cmd_table[cmd].command ) == 0 )
            break;
    }

    if( cmd == cmd_count )
    {
        telnetPrint( "Command not found\n", sfd );
    }
    else
    {
        cmd_table[cmd].cmd( sfd, argc, args );
    }
}

int vShellAdd( const char *command, void (* cmd)( int, int, char ** ) )	{
    struct statcmd cmdmv, cmnd;
    int cnt;
    void *temp;

    if( !ucStatInitialized )
    {
        return -1;
    }

    if( cmd_count > 0 )	{
        temp = (void *)cmd_table;
        temp = ucrealloc( (void *)cmd_table, sizeof( struct statcmd ) * ( cmd_count + 1 ) );

        if( !temp )
        {
            return -1;
        }

        cmd_table = (struct statcmd *)temp;
    }
    else
    {
        cmd_table = (struct statcmd *)ucalloc( sizeof( struct statcmd ) );

        if( !cmd_table )
        {
            return -1;
        }
    }

    cmnd.cmd = cmd;
    cmnd.command = (char *)ucalloc( vstrlen( command ) + 1 );

    if( !cmnd.command )
    {
        return -1;
    }

    vstrcpy( cmnd.command, command );

    if( cmd_count > 0 )	{
        for( cnt = 0; cnt < cmd_count; cnt++ )	{
            if( vstrcmp( cmd_table[cnt].command, command ) > 0 )	{	/* todo: implement a strcasecmp */
                cmdmv.cmd = cmd_table[cnt].cmd;
                cmdmv.command = cmd_table[cnt].command;

                cmd_table[cnt].cmd = cmnd.cmd;
                cmd_table[cnt].command = cmnd.command;

                cmnd.cmd = cmdmv.cmd;
                cmnd.command = cmdmv.command;
            }
        }
    }

    cmd_table[cmd_count].cmd = cmnd.cmd;
    cmd_table[cmd_count].command = cmnd.command;

    return ++cmd_count;
}

void ucstatClose( void)	{
    int cnt;
    if( cmd_table )	{
        for( cnt = 0; cnt < cmd_count; cnt++ )	{
            ucfree( cmd_table[cnt].command );
        }
        ucfree( cmd_table );
    }

    ucStatInitialized = 0;
}

void memcheck(int sfd, int argc, char *argv[] )
{
    char string[64];
    char temp[10];
    if( argc == 2 )	{
        if( vstrcmp( argv[1], "--error" ) == 0 )	{
            if( !vMemCheck() )	{
                telnetPrint( "No errors were found in the memory.\n", sfd );
            }
            else	{
                telnetPrint( "An error was found in the memory.\n", sfd );
            }
            return;
        }
        else if( vstrcmp( argv[1], "--usage" ) == 0 )	{
            vitoa( string, vUsage() );
            vstrcat( string, " of " );
            vitoa( temp, ( HEAPSIZE * BLOCKSIZE ) );
            vstrcat( string, temp );
            vstrcat( string, " bytes are in use.\n" );
            telnetPrint( string, sfd );
            return;
        }
        else if( vstrcmp( argv[1], "--watermark" ) == 0 )	{
            telnetPrint( "Watermark is at ", sfd );
            vitoa( string, vWatermark() );
            vstrcat( string, " bytes.\n" );
            telnetPrint( string, sfd );
            return;
        }
    }

    telnetPrint( "memcheck usage:\n", sfd );
    telnetPrint( "\t--error:\tCheck the heap for memory errors (corruption).\n", sfd );
    telnetPrint( "\t--usage:\tPrint the amount of memory currently in use.\n", sfd );
    telnetPrint( "\t--watermark:\tPrint the watermark for bytes that have been used.\n", sfd );
}

void vhelp(int sfd, int argc, char *argv[] )
{
    (void)argc;
    (void)argv;

    int cnt;

    for( cnt = 0; cnt < cmd_count; cnt++ )
    {
        telnetPrint( cmd_table[cnt].command, sfd );
        telnetPrint( "\n", sfd );
    }
}

void vIPadd(int sfd, int argc, char *argv[] )	{
    (void)sfd;
    (void)argc;
    if( argc == 2 )
    {
        vifup( vstrtoip( argv[1] ) );
    }
}

void vIPdel( int sfd, int argc, char *argv[] )	{

    if( argc == 2 )
    {
        swaddr addr = vstrtoip( argv[1] );

        if( addrtointerface( addr, null ) )
        {
            vifdown( addr );
            telnetPrint( "Interface removed.\n", sfd );
        }
        else
            telnetPrint( "Unable to find the interface.\n", sfd );
    }
}

