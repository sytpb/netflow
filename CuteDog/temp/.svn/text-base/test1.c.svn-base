#include "pcap.h"
#include <stdio.h>

void packet_callback(u_char * argument ,const struct pcap_pkthdr * packet_header ,const u_char * packet_content)
{
		int i=0;
		static int packet_number = 0;
		packet_number ++;
		//printf("%d content [%s] [%s]\n",packet_number,(packet_content+packet_header->len),(packet_content+packet_header->caplen));
		printf("****caplen=[%d] len=[%d] packets[ %s ] \n",packet_header->caplen,packet_header->len,packet_content+54);

		for( i=0;i<packet_header->len;++i)
				printf("%02X\t",packet_content[i]);
		printf("\n");
}

int main()
{
		pcap_t * pcap_handle;
		char error_content[PCAP_ERRBUF_SIZE];
		char * net_interface;
		bpf_u_int32 net_ip;
		bpf_u_int32 net_mask;
		struct bpf_program bpf_filter;
		const char *bpf_filter_string = "192.168.141.128";
		//const char *bpf_filter_string = "127.0.0.1";


		net_interface = pcap_lookupdev(error_content);
		pcap_lookupnet(net_interface,
						&net_ip,
						&net_mask,
						error_content);
		pcap_handle = pcap_open_live(net_interface,
						BUFSIZ,
						1,
						0,
						error_content);
		pcap_compile(pcap_handle,
						&bpf_filter,
						bpf_filter_string,
						0,
						net_ip);
		pcap_setfilter(pcap_handle,
						&bpf_filter);

		pcap_loop(pcap_handle,0,packet_callback,NULL);
		pcap_close(pcap_handle);
		return 0;
}

