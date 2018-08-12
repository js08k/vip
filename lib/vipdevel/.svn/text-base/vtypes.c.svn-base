#include "vtypes.h"
#include "vheap.h"

/*!
	\brief Hardware address compare function
	\details Iterates through each octet of the hardware address and returns
	false on the first mismatch. Returns true if all octets are matching.
	\note C++ equivalent:
		public:
			bool hwaddr::compare( const hwaddr &addr2 ) const;
*/
bool hwaddr_cmp( const hwaddr *addr1, const hwaddr *addr2 )	{
	if( addr1 && addr2 &&
		( addr1->data[0] == addr2->data[0] ) &&
		( addr1->data[1] == addr2->data[1] ) &&
		( addr1->data[2] == addr2->data[2] ) &&
		( addr1->data[3] == addr2->data[3] ) &&
		( addr1->data[4] == addr2->data[4] ) &&
		( addr1->data[5] == addr2->data[5] ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*!
 *  \todo C++ equivalent:
 *  	public:
 *         void hwaddr::copy( const hwaddr &addr );
*/
void hwaddr_cpy( hwaddr *dest, const hwaddr *src )
{
    vmemcpy( dest, src, sizeof( hwaddr ) );
}


void hwaddr_toraw( uint8_t *dest, const hwaddr *src )
{
    vmemcpy( dest, src, sizeof( hwaddr ) );
}

/*!
	\brief Software address compare function
	\details Iterates through each byte of the software address and returns
	false on the first mismatch. Returns true if all byte are matching. Returns
	false if either pointer is NULL.
*/
bool swaddr_cmp( const swaddr *addr1, const swaddr *addr2 )
{
	if( addr1 && addr2 &&
		( addr1->data[0] == addr2->data[0] ) &&
		( addr1->data[1] == addr2->data[1] ) &&
		( addr1->data[2] == addr2->data[2] ) &&
		( addr1->data[3] == addr2->data[3] ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*!
	\brief Software address copy function
	\details Copies the address detailed by addr2 into this. If either
	address is passed as NULL this function will return immediately.
	\note C++ equivalent:
		public:
			void swaddr::copy( const swaddr &addr2 );
*/
void swaddr_cpy( swaddr *dest, const swaddr *src )
{
    vmemcpy( dest, src, sizeof( swaddr ) );
}

void swaddr_set( swaddr *dest, uint32_t src )
{
	if(	dest )
	{
		dest->data[0] = src & 0xff;
        dest->data[1] = ( src >> 8 ) & 0xff;
        dest->data[2] = ( src >> 16 ) & 0xff;
        dest->data[3] = ( src >> 24 ) & 0xff;
	}
}

void swaddr_toraw( uint8_t *dest, const swaddr *src )
{
    vmemcpy( dest, src, sizeof( swaddr ) );
}

// todo: Turn addr into a refrence when in C++
uint32_t swaddr_toInt( const swaddr *addr )
{
	// Return the data
    return *( (uint32_t *)addr );
}

swaddr swaddr_fromInt( const uint32_t addr )
{
    return *( (swaddr *)&addr );
}

swaddr swaddr_hton( const swaddr addr )
{
	// todo: actually perform the realistic hton operation
	return addr;
}

/*
//void swaddr_tocstr( const swaddr *this, char *str, int *size )
//{
//	// Verify that NULL Pointers are not passed
//	if( !this || !str || !size )
//		return;
//
//
//}
//
//void swaddr_fromcstr( swaddr *this, const char *str )
//{
//
//}
 */
