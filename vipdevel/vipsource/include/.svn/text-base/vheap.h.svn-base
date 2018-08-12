/*******************************************************************
 * File: ucHeap.c
 * Written by: Joshua Strickland
 *
 * Version: 1.0		September 2010
 *			1.1		November  2010
 *			1.1.1	June	  2011
 *
 * Dependencies: vIPheap.h
 *
 * Note:
 * 	1.0: This source file needs to be verified of all situations
 * That it will work properly. Designed to work for uc0S, a real
 * time operating system
 *	1.1: Redesigned to work with Virtual IP as a buffer for
 * incoming and outgoing IP packets. Files renamed from ucHeap to
 * vIPheap.c/vIPheap.h.
 *  1.1.1: Changed buffer handle to be a bfd (buffer file descriptor)
 *  this stops modification to the buffer outside the API. Buffer
 *  check-summing was removed since the purpose of bfds replaced the
 *  purpose of the checksums. A buffer peek function was created
 *  allowing the next character to be read without being removed. A
 *  mass peek function was created functioning just as a read does but
 *  without removing the data from the buffer. Structure definitions
 *  were moved to the source file to follow a more proprietary format.
 *  Function added to enable the ability to uncover a heap being
 *  corrupted. This can be run during an idle task and is fairly
 *  accurate as long as ucfree/ucalloc/ucrealloc are not in the middle
 *  of being run in another task/process on a single core processor.
 *  Using this function uncovered a problem when trying to allocate
 *  1500 bytes when the block size was 8 bytes and offset pointer
 *  was 7 bits. #error directive has been added to the header bring
 *  this issue to attention during compile time.
 *******************************************************************/
#ifndef _vheap_h
#define _vheap_h
#ifndef __ARM__
#include <stdlib.h>
//#define __glib_malloc__ 1
#endif
#include "vtypes.h"

/***************************
	Defines:
****************************/
#define HEAP32K 32768
#define HEAP16K	16384
#define HEAP8K	8192
#define	HEAP4K	4096
#define HEAP2K	2048
#define HEAP1K	1024
#define HEAP64	64

/* Size of a single bloc (in bytes) */
#define BLOCKSIZE 32
#define MAXOFFSET 0x7F

#if ( ( BLOCKSIZE * MAXOFFSET ) < 1501 )
#error Block size is too small.
#endif

/* Size of the heap defined in blocks */
#define HEAPSIZE ( ( HEAP32K ) / BLOCKSIZE )

/****************************
	Heap API:
*****************************/
void 		initializeucheap	( void );
void 		*ucalloc			( uint16_t size );
void 		*ucrealloc			( void *base, uint16_t size );
void 		*ucfree				( void *base );

bool 		vMemCheck			( void );
int		 	vUsage				( void );
int			vWatermark			( void );

/****************************
	File Descriptor API:
*****************************/
bool 		vInitializefd		( void );
int		 	vfdGet				( void );
bool 		vfdDel				( int );

/****************************
	Buffer API:
*****************************/
void 		vInitializeBuffer	( void );
uint16_t 	vOpenBuf			( uint16_t size );
uint16_t	vPeekBuf			( uint16_t bfd, void *buffer, uint16_t size );
uint16_t 	vReadBuf			( uint16_t bfd, void *buffer, uint16_t size );
uint16_t 	vWriteBuf			( uint16_t bfd, const void *buffer, uint16_t size );
uint16_t 	vBufGetFree			( uint16_t bfd ); /* number of bytes free */
uint16_t    vBufGetSize         ( uint16_t bfd );
uint16_t 	vBufGetPending		( uint16_t bfd ); /* number of bytes in the buffer */
void 		vCloseBuf			( uint16_t bfd );

void 		vmemcpy				( void *destination, const void *source, int size );

#endif /* end _vheap_h */
