#ifndef __HEAD__NET
#define __HEAD__NET

#ifdef __cplusplus
extern "C" {
#endif 
	
int  StartupSocket();
int  CloseSocket(int sockfd);
int  CreateSocket(int type);
int  CreateSocketConnect(int *sock, char *ip, unsigned short port);
int  CreateSocketListen(int * fd, char *ip, unsigned short port,int Nonblock);


#ifdef __cplusplus
}
#endif

#endif /* __HEAD__*/