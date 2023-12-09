#include <unistd.h>
#include "pcap.h"

#define ASCII_LINELENGTH 300
#define HEXDUMP_BYTES_PER_LINE 16
#define HEXDUMP_SHORTS_PER_LINE (HEXDUMP_BYTES_PER_LINE / 2)
#define HEXDUMP_HEXSTUFF_PER_SHORT 5 /* 4 hex digits and a space */
#define HEXDUMP_HEXSTUFF_PER_LINE \
				(HEXDUMP_HEXSTUFF_PER_SHORT * HEXDUMP_SHORTS_PER_LINE)

#define PRT_TCP(x,y) \
if(((struct pcap_pkthdr*)(x))->len>54)\
	printf("TCP Msg [%s] \n",((y)+54));

#define PRT_LO(x,y) \
if(x>0 && (*y)== 0x3c)\
	printf("\n\tTCP Msg [%-*s]",0x40,((y)));

#define SENDTCP2CATS_LO(a,b,c,d,x,y,z)\
if((y)>0)\
	SendTcp2Cats((const char*)a,(const char*)b,(const char*)c,(const char*)d,x,y,z);


void udp_ethernet_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content);
void tcp_ethernet_protocol_packet_callback(u_char *argument,const struct pcap_pkthdr	*packet_header,const u_char	*packet_content);
