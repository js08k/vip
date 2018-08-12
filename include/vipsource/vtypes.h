#ifndef _vIPtypes_h
#define _vIPtypes_h

#ifndef __ARM__
#include <stdint.h>
#endif

#ifndef __cplusplus
#ifdef false
#undef false
#endif
#ifdef true
#undef true
#endif

typedef enum
{
    false = 0,
    true = 1
} bool;

#else
/****************************************
 * data types for Multi-IP stack on linux
 ****************************************/
typedef int8_t			sint8_t;
typedef int16_t			sint16_t;
typedef int32_t			sint32_t;
#endif

#ifdef __ARM__
/*************************************
 * data types for Multi-IP stack on uC
 ************************************/
typedef unsigned char 	uint8_t;
typedef unsigned short 	uint16_t;
typedef unsigned long 	uint32_t;
typedef signed char		int8_t;
typedef signed short	int16_t;
typedef signed long		int32_t;
typedef signed char 	sint8_t;
typedef signed short	sint16_t;
typedef signed long		sint32_t;
#else
/****************************************
 * data types for Multi-IP stack on linux
 ****************************************/
typedef int8_t			sint8_t;
typedef int16_t			sint16_t;
typedef int32_t			sint32_t;
#endif

/* Define the hardware address structure */
struct __hwaddr	
{
	uint8_t data[6];
};
/* Defining the hardware address type */
typedef struct __hwaddr hwaddr;
bool hwaddr_cmp( const hwaddr *, const hwaddr * );
void hwaddr_cpy( hwaddr *, const hwaddr * );
void hwaddr_toraw( uint8_t *, const hwaddr * );
hwaddr hwaddr_fromraw( uint8_t *src );

/* Define the software address structure */
struct __swaddr	{
	uint8_t data[4];
};

/* Defining the software address type */
typedef struct __swaddr swaddr;

/* software address modifiers */
bool swaddr_cmp( const swaddr *addr1, const swaddr *addr2 );
void swaddr_cpy( swaddr *dest, const swaddr *src );
void swaddr_set( swaddr *dest, uint32_t src );
void swaddr_toraw( uint8_t *dest, const swaddr *src );
swaddr swaddr_fromInt( const uint32_t addr );
uint32_t swaddr_toInt( const swaddr *addr );

swaddr swaddr_hton( const swaddr addr );

#ifdef null
#undef null
#endif
#define null 0	/* TODO: change to ( (void *)0 ) */

#ifdef __LP64__
typedef unsigned long int	pointer;
#else
/* TODO: fix pointer size for other than 64 bit */
typedef unsigned int		pointer;
#endif

#define IPv4TYPE 0x0800
#define ARPTYPE 0x0806
#define ICMPTYPE 0x01
#define TCPTYPE	 0x06
#define UDPTYPE	 0x11

#endif
