#ifndef _shell_h
#define _shell_h

#include "vtypes.h"

struct statcmd
{
	char* command;
    void (* cmd)( int sfd, int argc, char *args[] );
};

bool vShellInit( void );
int vShellAdd( const char *command, void (* cmd)( int, int, char ** ) );
void parse(int sfd, int argc, char *args[] );
void vipexit(int sfd, int argc, char *argv[] );
void memcheck(int sfd, int argc, char *argv[] );
void vhelp(int sfd, int argc, char *argv[] );
void vIPadd(int sfd, int argc, char *argv[] );
void vIPdel(int sfd, int argc, char *argv[] );

#endif /* _ucstat_h */
