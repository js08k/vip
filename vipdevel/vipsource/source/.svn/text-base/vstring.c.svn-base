#include "vstring.h"
#include "vheap.h"
#include "vipv4.h"
//#include "telnet.h"

/********************************************************
int vstrlen( const char *string )
	Returns the number of characters in a null terminated
string. The null character is not included in the count.
*********************************************************/
int vstrlen( const char *string )	{
	int n = 0;
	while( string[n] )	n++;
	return n;
}

/********************************************************
void vstrcpy( char *dest, const char *srce )
	Copies a null terminated string, srce, into the
location of dest. The null character is inserted at the
end of the copied string.
*********************************************************/
void vstrcpy( char *dest, const char *srce )	{
	int n = 0;
	/* Copy the source string until the null character is reached */
	do	{
		dest[n] = srce[n];
	} while( srce[n++] );

    dest[n] = '\0';
}

/*********************************************************
void vstrcat( char *dest, const char *srce )
	Appends the srce string to the destination
string. Both strings must be null terminated.
A null character is inserted at the end of the
resulting string.
**********************************************************/
void vstrcat( char *dest, const char *srce )	{
	int n = 0, i = 0;
	/* Move to the end of the destination string */
	while( dest[n] ) n++;
	/* Copy the source string until the null character is reached */
	do	{
		dest[n++] = srce[i];
	} while( srce[i++] );
}

/**********************************************************
int vatoi( const char *string )
	Given a null terminated string containing
only ascii numbers, this function returns the
integer value. Passing non integers in the string
will cause the result of the function to be unknown
***********************************************************/
int vatoi( const char *string )	{
	int i, pwr, power, index = 0, value = 0;
	/* Find the length of the string */
	pwr = vstrlen( string );
	/* Left most character's value is found by using powers
	   of 10 and the location from the end of the string. */
	while( string[index] )	{
		/* Each move from left to right of the string, is a smaller power of 10 */
		pwr--;
		/* Calculate the current power of 10 value */
		power = 1;
		for( i = pwr; i > 0; i-- )	{
			power = power * 10;
		}
		/* Add the current value being examined as an integer */
		/* 48 is the ascii value of zero, this causes ascii numbers to be integer values */
		value = value + ( ( string[index++] - '0' ) * power );
	}
	/* Return the value */
	return value;
}

/*******************************************************
void vitoa( char *string, int val )

	This function takes a integer and converts it to
an ascii string representation.
	String must be a pointer to a buffer with enough
space to store the integer value. The most space this
function can require is 10 characters.

TESTING: complete
********************************************************/
void vitoa( char *string, int val )
{
	int i = 0, j = 0, len;
	char temp[10];

	/* Ensure that string is not a null pointer */
	if( !string )	{
		return;
	}

	/* Set initial value to ascii 0 */
	temp[0] = '0';

	/* Zero the rest of the temp buffer */
	for( len = 1; len < 10; len++ )
		temp[len] = 0;

	/* The number is created in reverse */
	while( i++ < val )	{
		j = 0;

		/* Handle overflows */
		while( temp[j] == '9' )	{
			temp[j] = '0';
			j++;
		}

		if( temp[j] == 0 )	{
			/* An overflow did occur */
			temp[j] = '1';
		}
		else
			/* No overflow occurred */
			temp[j]++;
	}

	/* Get the length of the ascii representation */
	len = vstrlen( temp );

	/* Copy the ascii representation, reversing the numbers to the proper direction */
	for( i = 0; i < len; i++ )
		string[i] = temp[ len - 1 - i ];

	/* Null terminate the string */
	string[i] = '\0';
}

swaddr vstrtoip( const char *string )
{
    swaddr addr;
	char *temp, *str;
	int inx, i;

    str = (char *)ucalloc( vstrlen( string ) + 1 );

    if( !str )
    {
        vmemcpy( &addr, 0, sizeof( swaddr ) );
        return addr;
	}

	vmemcpy( str, string, vstrlen( string ) + 1 );
	temp = str;

	for( i=0; i < 4; i++ )	{
		inx = 0;
		while( temp[inx] != '.' && temp[inx] != '\0' )
			inx++;
		temp[inx++] = '\0';
        addr.data[i] = ( 0x000000ff & vatoi( temp ) );
		temp = &( temp[inx] );
	}

	ucfree( str );

    return addr;
}

void  viptostring( char *string, swaddr ip )
{
	int n;
	char temp[4];


	/* Convert the first octet of the IP address */
	vitoa( string, ip.data[0] );

	for( n = 1; n < 4; n++ )	{
		vstrcat( string, "." );
		vitoa( temp, ip.data[n] );
		vstrcat( string, temp );
	}
}

/*******************************************************************************
 * void vipMemcpy( void* destination, const void *source, unsigned short size )
 * 		Copies a number of bytes from the source to the destination, the
 * 	number of bytes are specified by the number "size".
 *******************************************************************************/
/*
void vMemcpy( void *destination, const void *source, uint32_t size )
{
	int i;

	if( source )
		for( i=0; i < size; i++ )
			((uint8_t*)destination)[i] = ((const uint8_t*)source)[i];
	else
		for( i=0; i < size; i++ )
			((uint8_t*)destination)[i] = null;
}
*/

int vmemcmp( const void *data, const void *datb, const int n )	{
	int cnt, val;
	for( cnt=val=0; cnt < n; cnt++ )	{
		val = val + ( (const char *)data )[cnt] - ( (const char *)datb )[cnt];
		if( val )	{
			return val;
		}
	}
	return val;
}

int vstrcmp( const char *str1, const char *str2 )
{
	uint32_t n = 0;
	sint8_t val = 0;
	 while( ( str1[n] != '\0' ) | ( str2[n] != '\0' ) )	{
		val = val + (int)str1[n] - (int)str2[n];
		if( val )
			return val;
		n++;
	}

	return val;
}

void chartohex( char character, char *str )	{
	uint8_t sval;
	int i;
	for( i=0; i < 2; i++ )	{
		if( i == 0 )
			sval = character>>4;
		else
			sval = character;

		sval = ( sval & 0x0f );

		if( sval > 9 )
			str[i] = ( 'a' + sval - 10 );
		else
			str[i] = ( '0' + sval );
	}

	str[i] = '\0';
}
