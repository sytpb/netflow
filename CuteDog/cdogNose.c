#include "pcap.h"
#include <string.h>
#include <ctype.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
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

void hex_and_ascii_print(const u_char *cp,u_int length)
{
		register unsigned int i;
		register int s1, s2;
		register int nshorts;
		char hexstuff[1500], *hsp;
		char asciistuff[1500], *asp;
		nshorts = length / sizeof(unsigned short);
		i = 0;
		hsp = hexstuff; asp = asciistuff;
		while (--nshorts >= 0) 
		{
				s1 = *cp++;
				s2 = *cp++;
				(void)snprintf(hsp,sizeof(hexstuff) - (hsp - hexstuff),
								" %02x%02x", s1, s2);
				hsp += 5;
				*(asp++) = (isgraph(s1) ? s1 : '.');
				*(asp++) = (isgraph(s2) ? s2 : '.');
				i++;
		}
		if (length & 1) {
				s1 = *cp++;
				(void)snprintf(hsp, sizeof(hexstuff) - (hsp - hexstuff),
								" %02x", s1);
				hsp += 3;
				*(asp++) = (isgraph(s1) ? s1 : '.');
				++i;
		}
		if (i > 0) 
		{
				*hsp = *asp = '\0';
				(void)printf("%s0x%04x: %-*s  %s",
								"\n\t", 32, 32,
								hexstuff, asciistuff);
		}
}

void
hex_and_ascii_print_with_offset(register const char *ident,
				register const u_char *cp, register u_int length, register u_int oset)
{
		register u_int i;
		register int s1, s2;
		register int nshorts;
		char hexstuff[HEXDUMP_SHORTS_PER_LINE*HEXDUMP_HEXSTUFF_PER_SHORT+1], *hsp;
		char asciistuff[ASCII_LINELENGTH+1], *asp;

		nshorts = length / sizeof(u_short);
		i = 0;
		hsp = hexstuff; asp = asciistuff;
		while (--nshorts >= 0) {
				s1 = *cp++;
				s2 = *cp++;
				(void)snprintf(hsp, sizeof(hexstuff) - (hsp - hexstuff),
								" %02x%02x", s1, s2);
				hsp += HEXDUMP_HEXSTUFF_PER_SHORT;
				*(asp++) = (isgraph(s1) ? s1 : '.');
				*(asp++) = (isgraph(s2) ? s2 : '.');
				i++;
				if (i >= HEXDUMP_SHORTS_PER_LINE) {
						*hsp = *asp = '\0';
						(void)printf("%s0x%04x: %-*s  %s",
										ident, oset, HEXDUMP_HEXSTUFF_PER_LINE,
										hexstuff, asciistuff);
						i = 0; hsp = hexstuff; asp = asciistuff;
						oset += HEXDUMP_BYTES_PER_LINE;
				}
		}
		if (length & 1) {
				s1 = *cp++;
				(void)snprintf(hsp, sizeof(hexstuff) - (hsp - hexstuff),
								" %02x", s1);
				hsp += 3;
				*(asp++) = (isgraph(s1) ? s1 : '.');
				++i;
		}
		if (i > 0) {
				*hsp = *asp = '\0';
				(void)printf("%s0x%04x: %-*s  %s",
								ident, oset, HEXDUMP_HEXSTUFF_PER_LINE,
								hexstuff, asciistuff);
		}
}
/*
 =======================================================================================================================
 */
/*
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
	pTcpData=packet_content+14+20+header_length;
	TcpData_Len=packet_header->caplen-14-20-header_length;

	sprintf(fp,"%d",source_port);
	sprintf(tp,"%d",destination_port);
	SENDTCP2CATS_LO(source_ip,fp,dest_ip,tp,pTcpData,TcpData_Len,pClientInfo);
}
*/
/*
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

	ip_protocol = (struct ip_header *) (packet_content + 14);

	checksum = ntohs(ip_protocol->ip_checksum);
	header_length = ip_protocol->ip_header_length * 4;
	tos = ip_protocol->ip_tos_taowenliu;
	offset = ntohs(ip_protocol->ip_off);
	switch(ip_protocol->ip_protocol)
	{
	case 6:		break;
	case 17:	break;
	case 1:		break;
	default:	break;
	}
	switch(ip_protocol->ip_protocol)
	{
	case 6:		tcp_protocol_packet_callback(argument, packet_header, packet_content); break;
	default:	break;
	}
}
*/
/*
 =======================================================================================================================
 */
/*
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
	ethernet_protocol = (struct ether_header *) packet_content;
	ethernet_type = ntohs(ethernet_protocol->ether_type);

	mac_string = ethernet_protocol->ether_shost;
	mac_string = ethernet_protocol->ether_dhost;
	switch(ethernet_type)
	{
	case 0x0800:	ip_protocol_packet_callback(argument, packet_header, packet_content); break;
	default:		break;
	}
	packet_number++;
}
*/
/*
 =======================================================================================================================
 =======================================================================================================================
 */
int SetCapFilter(pcap_t *Handle,char *FilterStr,bpf_u_int32 NetIP,PCLIENTINFO pClientInfo)
{
		int Ret;
		struct bpf_program	Bpf;
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		
		return_val_if_fail(Handle,0);
		return_val_if_fail(FilterStr,0);

		pcap_compile(Handle, &Bpf, FilterStr,0,NetIP);
		Ret=pcap_setfilter(Handle, &Bpf);
		
		if(Ret==0)
		{
				pDogMsg=IoGetSendMsgCell(pClientInfo);
				return_val_if_fail(pDogMsg,0);
				hRespStream=pDogMsg->hStream;
				push_stream_string(hRespStream,"<iq type='result'><query xmlns='filter:new' action='create'><error id='OK' content=''/></query></iq>");
				
				IoPutSendMsgCell(pDogMsg);
				return 1;
		}
		else
		{
				Dbg(("SetCapFilter:: Fail !!!!"));
				pDogMsg=IoGetSendMsgCell(pClientInfo);
				return_val_if_fail(pDogMsg,0);
				hRespStream=pDogMsg->hStream;
				push_stream_string(hRespStream,"<iq type='result'><query xmlns='filter:new' action='create'><error id='401' content='set failed'/></query></iq>");
				
				IoPutSendMsgCell(pDogMsg);
				return 0;
		}
		return 0;
}

void (*FunCallBack)(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content);

void *StartDogNoseThread(void *v)
{
	//pcap_t *Handle=(pcap_t*)v;
	PCUTEDOG pCuteDog=(PCUTEDOG)v;
	PCLIENTINFO pClientInfo=pCuteDog->Master;
	pcap_t *Handle=(pcap_t*)pCuteDog->Handle;

	return_val_if_fail(pCuteDog,NULL);
	return_val_if_fail(pCuteDog->Handle,NULL);
	return_val_if_fail(pCuteDog->Master,NULL);

	pthread_detach(pthread_self());	
	pcap_loop(Handle, -1,FunCallBack, (u_char*)pClientInfo);
	//if(Handle!=NULL)pcap_close(Handle);  //double free
	return 0;
}
pcap_t				*pcap_handle;
pthread_t 			tidp;
int StartDogNose(const char* FilterStr,const char* Name,PCLIENTINFO pClientInfo,int model)
{
	char				error_content[PCAP_ERRBUF_SIZE];
	char				*net_interface,lo_mem[4]={0};
	//char				FilterStr[] = "tcp dst port 5555 or dst port 22";
	//char				FilterStr[] = "tcp dst port 8023 or src port 8023 or dst port 48501 or src port 48501";
	//char				FilterStr[] = "tcp dst port 8023 or src port 8023 or dst port 48501";
	bpf_u_int32			net_mask;
	bpf_u_int32			net_ip;
	PCUTEDOG			pCuteDog;
	if(model==0)
	{

		strcpy(lo_mem,"lo");		//to 199.132
		net_interface=lo_mem;
	}
	else if(model==1)
	{		
		net_interface = pcap_lookupdev(error_content);
	}
	if(strstr(FilterStr,"udp") != NULL)
			FunCallBack = udp_ethernet_protocol_packet_callback;
	FunCallBack = tcp_ethernet_protocol_packet_callback;

	pcap_lookupnet(net_interface, &net_ip, &net_mask, error_content);
	pcap_handle = pcap_open_live(net_interface, BUFSIZ, 1, 0, error_content);
	if(SetCapFilter(pcap_handle,(char*)FilterStr,net_ip,pClientInfo)==0)
	{
			return 0;
	}
	if(pcap_datalink(pcap_handle) != DLT_EN10MB) return 0;
	pCuteDog=AddCuteDog2List(pcap_handle,&tidp,Name,FilterStr,pClientInfo);		//???? ->filter
	return_val_if_fail(pCuteDog,-1);
	if(!(pthread_create(&tidp,NULL,StartDogNoseThread,(void*)pCuteDog)))
	{
		Dbg(("[%s] start pcap succeed!  CLIENT:[%s:%ld] CuteDogName [%s] Net [%s]\n",Name,pCuteDog->Master->strIP,pCuteDog->Master->lIPPort,pCuteDog->Name,net_interface));
		return 1;
	}
#ifndef SER_DOG	
	pthread_join(tidp,NULL);          
#endif	
	return 0;
}
int StopDogNose(PCLIENTINFO pClientInfo)
{
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		char Body[512]={0};
		int Ret=-1;
		return_val_if_fail(pClientInfo->CuteDog,0);
		return_val_if_fail(pClientInfo->CuteDog->Handle,0);
		pcap_close(pClientInfo->CuteDog->Handle);
		Ret=pthread_cancel(*pClientInfo->CuteDog->Thid);
		if(Ret!=0)
		{
				Dbg(("Cancel Thread Fail !!!!!"));
				snprintf(Body,sizeof(Body),"<iq type='result'><query xmlns='filter:new' action='cancel'><error id='401'/></query></iq>");
		}
		else
		{
				Dbg(("Cancel Thread succeed !!!!!!"));
				pClientInfo->CuteDog->Handle=NULL;
				snprintf(Body,sizeof(Body),"<iq type='result'><query xmlns='filter:new' action='cancel'><error id='OK' content=''/></query></iq>");
		} 
		pDogMsg=IoGetSendMsgCell(pClientInfo);
		return_val_if_fail(pDogMsg,0);
		hRespStream=pDogMsg->hStream;
		push_stream_string(hRespStream,Body);

		IoPutSendMsgCell(pDogMsg);
		return Ret;
}

