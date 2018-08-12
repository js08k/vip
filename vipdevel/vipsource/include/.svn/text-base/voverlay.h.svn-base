#ifndef _vipoverlay_h
#define _vipoverlay_h

/* Special overlay defines expecting a void pointer, ploc (packet location) */
/* ploc is expected to be the start of the packet */

// Returns a pointer to an MAC header struct
#define MAC(ploc) \
    ( (tagMACgram *)ploc )

// Returns a pointer to the payload of a MAC packet
#define MACpayload(ploc) \
    ( (char *)MAC(ploc) + sizeof( tagMACgram ) )

// Returns a pointer to an ARP header struct
#define ARP(ploc) \
    ( (tagARPgram *)MACpayload(ploc) )

// Returns a pointer to the payload of a ARP packet
#define ARPpayload(ploc) \
    ( (char *)( (char *)ARP(ploc) + sizeof( tagARPgram ) ) )

// Returns a pointer to an IPv4 header struct
#define IPv4(ploc) \
    ( (tagIPgram *)MACpayload(ploc) )

// Returns a pointer to the payload of a IPv4 packet
// note: HeaderLength is described in 32 bit words, this is the reason for
// multiplying by 4. Also the minimum header length is 20 bytes by the IPv4
// standard.
#define IPv4payload(ploc) \
    ( &( (char *)IPv4(ploc) ) \
    [ IPv4(ploc)->HeaderLength > 5 ? IPv4(ploc)->HeaderLength * 4 : 20 ] )
/*
// Old way IPv4payload was defined.
#define IPv4payload(ploc) \
    ( (char *)IPv4(ploc) + sizeof( tagIPgram ) )
*/

// Returns a pointer to an IPv6 header struct
#define IPv6(ploc) \
    ( (char *)MACpayload(ploc) )

// Returns a pointer to the payload of a IPv6 packet
#define IPv6payload(ploc) \
    ( (char *)( (char *)IPv6(ploc) + sizeof( tagIPv6gram ) ) )

// Returns a pointer to an ICMP header struct
#define ICMP(ploc) \
    ( (tagICMPgram *)IPv4payload(ploc) )

// Returns a pointer to the payload of a ICMP packet
#define ICMPpayload(ploc) \
    ( (char *)( (char *)ICMP(ploc) + sizeof( tagICMPgram ) ) )

// Returns a pointer to an TCP header struct
#define TCP(ploc) \
    ( (tagTCPgram *)IPv4payload(ploc) )

#define TCPoptions(ploc) \
    ( (char *)( (char *)IPv4payload(ploc) + sizeof( tagTCPgram ) ) )

// Returns a pointer to the payload of a TCP packet
// note: HeaderLength is described in 32 bit words, this is the reason for
// multiplying by 4. Also the minimum header length is 20 bytes by the Tcp
// standard.
#define TCPpayload(ploc) \
    ( &( (char *)TCP(ploc) ) \
    [ TCP(ploc)->dataOffset > 5 ? TCP(ploc)->dataOffset * 4 : 20 ] )
/*
// Old way IPv4payload was defined.
#define TCPpayload(ploc) \
    ( (char *)TCP(ploc) + sizeof( tagTCPgram ) )
*/

// Returns a pointer to an UDP header struct
#define UDP(ploc) \
    ( (tagUDPgram *)IPv4payload(ploc) )

// Returns a pointer to the payload of a UDP packet
#define UDPpayload(ploc) \
    ( (char *)( (char *)UDP(ploc) + sizeof( tagUDPgram ) ) )

#endif	/* end _vipoverlay_h */
