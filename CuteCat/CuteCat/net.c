
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "net.h"

#ifdef __cplusplus
extern "C" {
#endif 

#pragma comment(lib, "WS2_32.Lib")

//#define		WOULDBLOCK			10035
#define		MAX_BACKLOG			512

int  StartupSocket()
{
#ifdef _WIN32
	WSADATA wsadata;
	WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif
	return 0;
}
	
int  CloseSocket(int sockfd)
{
#ifdef _WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif
	return 0;
}
	
int CreateSocket(int type)
{
	int sock ;
	sock = socket(AF_INET, type, 0);
	if(sock == -1)
	{
		return -1;
	}
	return sock;
}
	
int  CreateSocketConnect(int *sock, char *ip, unsigned short port)
{
	int ret;
	int sockfd ;
	if(*sock<=0)
	{
		struct sockaddr_in addr;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd == -1)
		{
			return -1;
		}
		{
			int flag = 1;		
#ifdef _WIN32
			ioctlsocket(sockfd, FIONBIO, &flag);
#else
			flag = fcntl(sockfd, F_GETFL, 0);
			fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);
#endif	
		}
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip);
		
		ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		if(ret < 0)
		{
#ifdef _WIN32
			if(WSAGetLastError()==WSAEWOULDBLOCK)
#else
				if(errno==10035) 
#endif
				{
					*sock=sockfd;
					return 1;
				}
				else
				{
#ifdef _WIN32
					closesocket(sockfd);
#else
					close(sockfd);
#endif
					return -2;
				}
		}
		return 0;
	}
	else
	{
		fd_set fdset; 
		struct timeval tv; 
		int valopt; 
		int lon;
		sockfd=*sock;
		tv.tv_sec = 0; 
		tv.tv_usec = 1000; 
		FD_ZERO(&fdset); 
		FD_SET(sockfd, &fdset); 
		ret = select(sockfd+1, NULL, &fdset, NULL, &tv); 
		if (ret < 0 )//&& errno != EINTR) 
		{ 
			return 1;
		} 
		else if (ret > 0) 
		{ 
			lon = sizeof(int); 
			if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0 || valopt)
			{  
#ifdef _WIN32
				closesocket(sockfd);
#else
				close(sockfd);
#endif
				*sock=0;
				return -3;
			}
			return 0;
		}
		return 1;
	}
}

int  CreateSocketListen(int * fd, char *ip, unsigned short port,int Nonblock)
{
	struct sockaddr_in addr;
	int ret ;
	int sockfd ;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		return -1;
	}
	if(Nonblock)
	{
		int flag = 1;
#ifdef _WIN32
		ioctlsocket(sockfd, FIONBIO, &flag);
#else
		flag = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);
#endif	
	}
	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0)
	{
#ifdef _WIN32
		printf("WSAGetLastError():%d\n",WSAGetLastError());
		closesocket(sockfd);
#else
		close(sockfd);
#endif
		return -2;
	}
	ret = listen(sockfd, MAX_BACKLOG);
	if(ret < 0)
	{
#ifdef _WIN32
		closesocket(sockfd);
#else
		close(sockfd);
#endif	
		return -3;
	}	
	*fd = sockfd;
	return 0;
}

#ifdef __cplusplus
}
#endif 