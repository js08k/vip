#ifndef _httpd_h
#define _httpd_h

#include "vtypes.h"

struct httpSession;

void vInitializeHttpd( void );
void httpdCallback( int sfd, int event, const void *nullarg );
bool httpdReadInHeader( struct httpSession *session );
void httpdZeroDelim( struct httpSession *session );
void httpdParseHeader( int sfd, int argc, char *argv[] );
void httpdGet( int sfd, int argc, char *argv[] );
void httpdHead( int sfd, int argc, char *argv[] );
void httpdPost( int sfd, int argc, char *argv[] );
void addContent( const char* ContentName, const char *ContentLocation, int ContentSize );

#endif
