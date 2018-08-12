#ifndef _vIPstring_h
#define _vIPstring_h

#include "vtypes.h"

int vstrlen( const char * );
swaddr vstrtoip( const char *string );
void  viptostring( char *string, swaddr IP );
void vitoa( char *string, int val );
int vatoi( const char *string );

int vmemcmp( const void *, const void *, const int );
int vstrcmp( const char *, const char * );
void vstrcpy( char *, const char * );
void vstrcat( char *, const char * );
void chartohex( char, char * );


#endif
