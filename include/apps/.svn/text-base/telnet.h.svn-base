#ifndef _telnet_h
#define _telnet_h

#include "vtypes.h"

// Forward declare the telnetSession
struct telnetSession;

void vInitializeTelnet( void );
void telnetCallback(int sfd, int event, const void * voidarg);
bool telnetReadIn( struct telnetSession* session );
void telnetParseControls( struct telnetSession* session );
void telnetZeroDelim( struct telnetSession *session );
char telnetGetChar(int sfd );
void telnetSendChar(int sfd, uint8_t character );
void telnetPrint(const char *string, int sfd );
void telnetExit(int sfd, int, char** );

#endif
