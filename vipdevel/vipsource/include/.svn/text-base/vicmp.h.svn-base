/************************************
 * Virtual IP stack					*
 * File:	vIPicmp.h				*
 * Author:	Joshua Strickland		*
 * Date:	November 2010			*
 * Version:	1.0						*
 *									*
 * ICMP currently only supports		*
 * replying to a ping request. This	*
 * Allow for measuring the network	*
 * latency.							*
 ************************************/
#ifndef _vIPicmp_h
#define _vIPicmp_h

//#include "vbase.h"
#include "vtypes.h"

typedef struct _tagicmpGram
{
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	identifier;
	uint16_t	sequence;
} tagICMPgram;

bool vICMPhandler(int position, swaddr sip, uint32_t datn, const void *ploc );

#endif // end _vIPicmp_h
