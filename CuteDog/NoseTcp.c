#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "cdogcore.h"
#include "cdogNose.h"
//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
struct ether_header
{
	u_int8_t	ether_dhost[6];

	u_int8_t	ether_shost[6];

	u_int16_t	ether_type;

};

typedef u_int32_t	in_addr_t;
/*
struct in_addr
{
	in_addr_t	s_addr;
};*/

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
struct ip_header
{
#if defined WORDS_BIGENDIAN
	u_int8_t		ip_version:4,ip_header_length:4;
#else
	u_int8_t		ip_header_length:4, ip_version:4;
#endif
	u_int8_t		ip_tos_taowenliu;
	u_int16_t		ip_length;
	u_int16_t		ip_id;
	u_int16_t		ip_off;
	u_int8_t		ip_ttl;
	u_int8_t		ip_protocol;
	u_int16_t		ip_checksum;
	struct in_addr	ip_souce_address;
	struct in_addr	ip_destination_address;
};

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
struct tcp_header
{
	u_int16_t	tcp_source_port;
	u_int16_t	tcp_destination_port;
	u_int32_t	tcp_acknowledgement;
	u_int32_t	tcp_ack;

#ifdef WORDS_BIGENDIAN
	u_int8_t	tcp_offset : 4,
	tcp_reserved : 4;
#else
	u_int8_t	tcp_reserved : 4,
	tcp_offset : 4;
#endif
	u_int8_t	tcp_flags;
	u_int16_t	tcp_windows;
	u_int16_t	tcp_checksum;
	u_int16_t	tcp_urgent_pointer;
};

/*
 =======================================================================================================================
 */
static
void tcp_protocol_packet_callback
(
	u_char						*argument,
	const struct pcap_pkthdr	*packet_header,
	const u_char				*packet_content
)
{
	struct tcp_header	*tcp_protocol;
	u_char				flags;
	int					header_length;
	u_short				source_port;
	u_short				destination_port;
	u_short				windows;
	u_short				urgent_pointer;
	u_int				sequence;
	u_int				acknowledgement;
	u_int16_t			checksum;

	const u_char 		*pTcpData;
	u_int 				TcpData_Len;
	char				fp[8]={0},tp[8]={0};
	struct ip_header	*ip_protocol;
	char				source_ip[32]={0},dest_ip[32]={0};

	ip_protocol = (struct ip_header *)(packet_content + 14);
	PCLIENTINFO pClientInfo=(PCLIENTINFO)argument;
	tcp_protocol = (struct tcp_header *) (packet_content + 14 + 20);
	source_port = ntohs(tcp_protocol->tcp_source_port);
	destination_port = ntohs(tcp_protocol->tcp_destination_port);
	header_length = tcp_protocol->tcp_offset * 4;
	sequence = ntohl(tcp_protocol->tcp_acknowledgement);
	acknowledgement = ntohl(tcp_protocol->tcp_ack);
	windows = ntohs(tcp_protocol->tcp_windows);
	urgent_pointer = ntohs(tcp_protocol->tcp_urgent_pointer);
	flags = tcp_protocol->tcp_flags;
	checksum = ntohs(tcp_protocol->tcp_checksum);
	strncpy(source_ip,inet_ntoa(ip_protocol->ip_souce_address),sizeof(source_ip));
	strncpy(dest_ip,inet_ntoa(ip_protocol->ip_destination_address),sizeof(dest_ip));
/*
	printf("------------------------  TCP Protocol  (Transport Layer)  -------------------------------------------\n");
	printf("Source Port:%d\n", source_port);
	printf("Destination Port:%d\n", destination_port);
*/
	/*
	while(i<len)
	{
			printf("%2x \t",packet_content[i]);
			++i;
	}
	i=0;
	while(i<len )
	{
			if(strlen(packet_content+i)>0 && i>54 && i<170)
				if(isgraph(packet_content[i]))	
					printf(" ****LEN = %d %s\n",i,packet_content+i);
			++i;
	}*/
/*	
	switch(destination_port)	
	{
	case 80:	printf("HTTP protocol\n"); break;
	case 21:	printf("FTP protocol\n"); break;
	case 23:	printf("TELNET protocol\n"); break;
	case 25:	printf("SMTP protocol\n"); break;
	case 110:	printf("POP3 protocol\n"); break;
	default:	break;
	}

	printf("Sequence Number:%u\n", sequence);
	printf("Acknowledgement Number:%u\n", acknowledgement);
	printf("Header Length:%d\n", header_length);
	printf("Reserved:%d\n", tcp_protocol->tcp_reserved);
	printf("Flags:");

	if(flags & 0x08) printf("PSH ");
	if(flags & 0x10) printf("ACK ");
	if(flags & 0x02) printf("SYN ");
	if(flags & 0x20) printf("URG ");
	if(flags & 0x01) printf("FIN ");
	if(flags & 0x04) printf("RST ");
	printf("\n");
	printf("Window Size:%d\n", windows);
	printf("Checksum:%d\n", checksum);
	printf("Urgent pointer:%d\n", urgent_pointer);
*/
	pTcpData=packet_content+14+20+header_length;
	TcpData_Len=packet_header->caplen-14-20-header_length;

	//printf("\t smsp packets [%s]\n",packet_content+82);   66+16 ???? smsp ->smst 
	//hex_and_ascii_print(packet_content,packet_header->caplen);
/*	printf("\tRawData Caplen=[%d],Len=[%d] PORT: [%d] >> [%d]",packet_header->caplen,packet_header->len,source_port,destination_port);
	hex_and_ascii_print_with_offset("\n\t",pTcpData,TcpData_Len,0);
	PRT_LO(TcpData_Len,pTcpData);
	printf("\n");
*/
	sprintf(fp,"%d",source_port);
	sprintf(tp,"%d",destination_port);
	SENDTCP2CATS_LO(source_ip,fp,dest_ip,tp,pTcpData,TcpData_Len,pClientInfo);
}

/*
 =======================================================================================================================
 */
static
void ip_protocol_packet_callback
(
	u_char						*argument,
	const struct pcap_pkthdr	*packet_header,
	const u_char				*packet_content
)
{
	struct ip_header	*ip_protocol;

	u_int				header_length;
	u_int				offset;
	u_char				tos;
	u_int16_t			checksum;

//	printf("-------   IP Protocol  (Network Layer)   --------\n");
	ip_protocol = (struct ip_header *) (packet_content + 14);

	checksum = ntohs(ip_protocol->ip_checksum);
	header_length = ip_protocol->ip_header_length * 4;
	tos = ip_protocol->ip_tos_taowenliu;
	offset = ntohs(ip_protocol->ip_off);
/*
	printf("IP Version:%d\n", ip_protocol->ip_version);
	printf("Header length:%d\n", header_length);
	printf("TOS:%d\n", tos);
	printf("Total length:%d\n", ntohs(ip_protocol->ip_length));
	printf("Identification:%d\n", ntohs(ip_protocol->ip_id));
	printf("Offset:%d\n", (offset & 0x1fff) * 8);
	printf("TTL:%d\n", ip_protocol->ip_ttl);
	printf("Protocol:%d\n", ip_protocol->ip_protocol);
*/
	switch(ip_protocol->ip_protocol)
	{
	case 6:		/*printf("The Transport Layer Protocol is TCP\n");*/ break;
	case 17:	/*printf("The Transport Layer Protocol is UDP\n");*/ break;
	case 1:		/*printf("The Transport Layer Protocol is ICMP\n");*/ break;
	default:	break;
	}
/*
	printf("Header checksum:%d\n", checksum);
	printf("Source address:%s\n", inet_ntoa(ip_protocol->ip_souce_address));
	printf("Destination address:%s\n", inet_ntoa(ip_protocol->ip_destination_address));
*/
	switch(ip_protocol->ip_protocol)
	{
	case 6:		tcp_protocol_packet_callback(argument, packet_header, packet_content); break;
	default:	break;
	}
}

/*
 =======================================================================================================================
 */
void tcp_ethernet_protocol_packet_callback
(
	u_char						*argument,
	const struct pcap_pkthdr	*packet_header,
	const u_char				*packet_content
)
{
	u_short				ethernet_type;
	struct ether_header *ethernet_protocol;
	u_char				*mac_string;
	static int			packet_number = 1;
/*	printf("**************************************************\n");
	printf("The %d  TCP  packet is captured.\n", packet_number);
	printf("-------    Ehternet Protocol (Link Layer)    -----\n");
*/	
	ethernet_protocol = (struct ether_header *) packet_content;
//	printf("Ethernet type is :\n");
	ethernet_type = ntohs(ethernet_protocol->ether_type);

/*	printf("%04x\n", ethernet_type);
	switch(ethernet_type)
	{
	case 0x0800:	printf("The network layer is IP protocol\n"); break;
	case 0x0806:	printf("The network layer is ARP protocol\n"); break;
	case 0x8035:	printf("The network layer is RARP protocol\n"); break;
	default:		break;
	}
	printf("Mac Source Address is : \n");
*/	
	mac_string = ethernet_protocol->ether_shost;
/*	printf
	(
		"%02x:%02x:%02x:%02x:%02x:%02x\n",
		*mac_string,
		*(mac_string + 1),
		*(mac_string + 2),
		*(mac_string + 3),
		*(mac_string + 4),
		*(mac_string + 5)
	);
*/
//	printf("Mac Destination Address is : \n");
	mac_string = ethernet_protocol->ether_dhost;
/*	printf
	(
		"%02x:%02x:%02x:%02x:%02x:%02x\n",
		*mac_string,
		*(mac_string + 1),
		*(mac_string + 2),
		*(mac_string + 3),
		*(mac_string + 4),
		*(mac_string + 5)
	);
*/
	switch(ethernet_type)
	{
	case 0x0800:	ip_protocol_packet_callback(argument, packet_header, packet_content); break;
	default:		break;
	}
//	printf("**************************************************\n");
	packet_number++;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
