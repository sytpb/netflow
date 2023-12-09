#include <stdio.h> 
#include <pcap.h>  

/*the printer running when packet have captured*/          

void printer()     
{            
		printf("A packet is captured!\n"); 
		return;  
}          

int main (int argc, char* argv[])  
{         
		char mbuf[1024]={0};	
#ifdef HELLO		
		printf("hello my test !!! \n");
#else 
		printf("not defined HELLO maro \n");
#endif		
		/*the error code buf of libpcap*/    
		char ebuf[PCAP_ERRBUF_SIZE];    
		/*create capture handler of libpcap*/    
		pcap_t *pd = pcap_open_live ("eth0", 68, 0, 1000, ebuf); 
		printf("pd=%p \n", pd);
		/*start the loop of capture, loop 5 times, enter printer when capted*/       
		pcap_loop (pd, 10, printer, mbuf);
 		printf("%s \n",mbuf);		
		pcap_close (pd);      
		return 0; 

}

