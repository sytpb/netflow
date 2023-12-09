#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "cdogcore.h"
#include "cdogNose.h"
struct ether_header
{
    u_int8_t ether_dhost[6];
    u_int8_t ether_shost[6];
    u_int16_t ether_type;
};
typedef u_int32_t in_addr_t;
/*
struct in_addr
{
    in_addr_t s_addr;
};
*/
struct ip_header
{
    #if defined(WORDS_BIGENDIAN)
        u_int8_t ip_version: 4,  /* �汾 */
        ip_header_length: 4; /* �ײ����� */
    #else
        u_int8_t ip_header_length: 4, ip_version: 4;
    #endif
    u_int8_t ip_tos;
    /* �������� */
    u_int16_t ip_length;
    /* �ܳ��� */
    u_int16_t ip_id;
    /* ��ʶ */
    u_int16_t ip_off;
    /* ƫ�� */
    u_int8_t ip_ttl;
    /* ����ʱ�� */
    u_int8_t ip_protocol;
    /* Э������ */
    u_int16_t ip_checksum;
    /* У��� */
    struct in_addr ip_souce_address;
    /* ԴIP��ַ */
    struct in_addr ip_destination_address;
    /* Ŀ��IP��ַ */
};
/*
-----------------------------------------------------------------------------------------------------------------------
������UDPЭ���ʽ�Ķ���
-----------------------------------------------------------------------------------------------------------------------
 */
struct udp_header
{
    u_int16_t udp_source_port;
    /* Դ�˿ں� */
    u_int16_t udp_destination_port;
    /* Ŀ�Ķ˿ں� */
    u_int16_t udp_length;
    /* ���� */
    u_int16_t udp_checksum;
    /* У��� */
};
/*
=======================================================================================================================
 */
static
void udp_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content)
{
    struct udp_header *udp_protocol;
    u_short source_port;
    u_short destination_port;
    u_short length;
    
	const u_char 		*pUdpData;
	u_int 				UdpData_Len;
	char				fp[8]={0},tp[8]={0};
	struct ip_header	*ip_protocol;
	char				source_ip[32]={0},dest_ip[32]={0};

	udp_protocol = (struct udp_header*)(packet_content + 14+20);
	PCLIENTINFO pClientInfo=(PCLIENTINFO)argument;
    
	source_port = ntohs(udp_protocol->udp_source_port);
    destination_port = ntohs(udp_protocol->udp_destination_port);
    length = ntohs(udp_protocol->udp_length);
    printf("----------  UDP Protocol  (Transport  Layer)  ----------\n");
    printf("Source port:%d\n", source_port);
    printf("Destination port:%d\n", destination_port);
    switch (destination_port)
    {
        case 138:
            printf("NETBIOS Datagram Service\n");
            break;
        case 137:
            printf("NETBIOS Name Service\n");
            break;
        case 139:
            printf("NETBIOS session service\n");
            break;
        case 53:
            printf("name-domain server \n");
            break;
        default:
            break; /* ����û�з��� */
    }
    printf("Length:%d\n", length);
    printf("Checksum:%d\n", ntohs(udp_protocol->udp_checksum));
	ip_protocol = (struct ip_header *)(packet_content + 14);
	strncpy(source_ip,inet_ntoa(ip_protocol->ip_souce_address),sizeof(source_ip));
	strncpy(dest_ip,inet_ntoa(ip_protocol->ip_destination_address),sizeof(dest_ip));

	pUdpData=packet_content+14+20;
	UdpData_Len=length;
	sprintf(fp,"%d",source_port);
	sprintf(tp,"%d",destination_port);
	SENDTCP2CATS_LO(source_ip,fp,dest_ip,tp,pUdpData,UdpData_Len,pClientInfo);
}
/*
=======================================================================================================================
�����Ƿ���IPЭ��ĺ����Ķ���
=======================================================================================================================
 */
static
void ip_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content)
{
    struct ip_header *ip_protocol;
    /* IPЭ����� */
    u_int header_length;
    /* �ײ����� */
    u_int offset;
    /* ƫ�� */
    u_char tos;
    /* �������� */
    u_int16_t checksum;
    /* У��� */
    printf("----------  IP Protocol  (Network Layer)  ----------\n");
    ip_protocol = (struct ip_header*)(packet_content + 14);
    /* ���IPЭ���������� */
    checksum = ntohs(ip_protocol->ip_checksum);
    /* ���У��� */
    header_length = ip_protocol->ip_header_length *4;
    /* ����ײ����� */
    tos = ip_protocol->ip_tos;
    /* �������� */
    offset = ntohs(ip_protocol->ip_off);
    /* ���ƫ���� */
    printf("IP Version:%d\n", ip_protocol->ip_version);
    /* ��ð汾�� */
    printf("Header length:%d\n", header_length);
    printf("TOS:%d\n", tos);
    printf("Total length:%d\n", ntohs(ip_protocol->ip_length));
    /* ����ܳ��� */
    printf("Identification:%d\n", ntohs(ip_protocol->ip_id));
    /* ��ñ�ʶ */
    printf("Offset:%d\n", (offset &0x1fff) *8);
    printf("TTL:%d\n", ip_protocol->ip_ttl);
    /* ���TTL */
    printf("Protocol:%d\n", ip_protocol->ip_protocol);
    /* ���Э������ */
    switch (ip_protocol->ip_protocol) /* ����Э�������ж� */
    {
        case 6:
            printf("The Transport Layer Protocol is TCP\n");
            break;
            /* �ϲ�Э��ΪTCPЭ�� */
        case 17:
            printf("The Transport Layer Protocol is UDP\n");
            break;
            /* �ϲ�Э��ΪUDPЭ�� */
        case 1:
            printf("The Transport Layer Protocol is ICMP\n");
            break;
            /* �ϲ�Э��ΪICMPЭ�� */
        default:
            break;
    }
    printf("Header checksum:%d\n", checksum);
    printf("Source address:%s\n", inet_ntoa(ip_protocol->ip_souce_address));
    /* ���ԴIP��ַ */
    printf("Destination address:%s\n", inet_ntoa(ip_protocol->ip_destination_address));
    /* ���Ŀ��IP��ַ */
    switch (ip_protocol->ip_protocol)
    {
        case 17:
            udp_protocol_packet_callback(argument, packet_header, packet_content);
            break;
            /* ����ϲ�Э��ΪUDPЭ�飬�͵��÷���UDPЭ��ĺ�����ע���ʱ�Ĳ���������ʽ */
        default:
            break;
    }
}
/*
=======================================================================================================================
 */
void udp_ethernet_protocol_packet_callback(u_char *argument, const struct pcap_pkthdr *packet_header, const u_char *packet_content)
{
    u_short ethernet_type;
    struct ether_header *ethernet_protocol;
    u_char *mac_string;
    static int packet_number = 1;
    printf("**************************************************\n");
    printf("The %d  UDP  packet is captured.\n", packet_number);
    printf("--------   Ehternet Protocol (Link Layer)    --------\n");
    ethernet_protocol = (struct ether_header*)packet_content;
    printf("Ethernet type is :\n");
    ethernet_type = ntohs(ethernet_protocol->ether_type);
    printf("%04x\n", ethernet_type);
    switch (ethernet_type)
    {
        case 0x0800:
            printf("The network layer is IP protocol\n");
            break;
        case 0x0806:
            printf("The network layer is ARP protocol\n");
            break;
        case 0x8035:
            printf("The network layer is RARP protocol\n");
            break;
        default:
            break;
    }
    printf("Mac Source Address is : \n");
    mac_string = ethernet_protocol->ether_shost;
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
    printf("Mac Destination Address is : \n");
    mac_string = ethernet_protocol->ether_dhost;
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *mac_string, *(mac_string + 1), *(mac_string + 2), *(mac_string + 3), *(mac_string + 4), *(mac_string + 5));
    switch (ethernet_type)
    {
        case 0x0800:
            ip_protocol_packet_callback(argument, packet_header, packet_content);
            break;
        default:
            break;
    }
    printf("**************************************************\n");
    packet_number++;
}
/*
=======================================================================================================================
 */
/*
void main()
{
    pcap_t *pcap_handle;
    char error_content[PCAP_ERRBUF_SIZE];
    char *net_interface;
    struct bpf_program bpf_filter;
    char bpf_filter_string[] = "udp";
    bpf_u_int32 net_mask;
    bpf_u_int32 net_ip;
    net_interface = pcap_lookupdev(error_content);
    pcap_lookupnet(net_interface, &net_ip, &net_mask, error_content);
    pcap_handle = pcap_open_live(net_interface, BUFSIZ, 1, 0, error_content);
    pcap_compile(pcap_handle, &bpf_filter, bpf_filter_string, 0, net_ip);
    pcap_setfilter(pcap_handle, &bpf_filter);
    if (pcap_datalink(pcap_handle) != DLT_EN10MB)
        return ;
    pcap_loop(pcap_handle,  - 1, ethernet_protocol_packet_callback, NULL);
    pcap_close(pcap_handle);
}
*/
