#ifndef __HEAD__CORE
#define __HEAD__CORE

#ifdef __cplusplus
extern "C" {
#endif 

#include "../include/iksemel.h"
#define		MSGINITSIZE			1024	//?!

typedef enum
{
	STATUS_SOCKET_CLOSED=0,
	STATUS_SOCKET_CONNECT
}SOCETSTATUS;

typedef enum
{
	CUTE_CAT_HOOKER=0,
	CUTE_CAT_PIONEER
}CUTECATTYPE;

typedef enum 
{
	CLIENT_UNSHAKE=0,
	CLIENT_SHAKEED,
	CLIENT_UNLOGIN,
	CLIENT_LOGINED,
	CLIENT_STOPPED
}CLIENTSTATUS;


typedef struct tagSERVER
{
	char				strIP[32];	
	unsigned short		port;
}SERVER,*PSERVER;

typedef	struct tagCLIENTINFO
{
	SOCETSTATUS 		SockStatus;
	CLIENTSTATUS		Status;
	int					Socket;
	
	int					iCurSendLen;
	char				*pSndData;
	char				ReMsg[MSGINITSIZE];
	unsigned int		iReMsgLen;
	iksparser			*Prs;
	char				Name[32];
}CLIENTINFO,*PCLIENTINFO;

typedef	struct tagCAPELEMENT
{
	size_t	iLen;
	char	pBuf[4];
	//char	*pBuf;
	//void  *Buf[0];
}CAPELEMENT;

void	SetClientStatus(CLIENTSTATUS Status);
int		PushSendStream(char *pSendData,int len);
int		Start(int iType,char *Name,void *pFunIOProcessDogMsg);
int		Stop();


//int vm_Start(int iType);
//int vm_Stop();
#ifdef __cplusplus
}
#endif

#endif /* __HEAD__*/