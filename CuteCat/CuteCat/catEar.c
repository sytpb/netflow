
#ifndef __HEAD__CATEAR
#define __HEAD__CATEAR

#ifdef __cplusplus
extern "C" {
#endif 
	
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "../include/list.h"
#include "../include/ini.h"
#include "catcore.h"
#include "print.h"

#pragma comment(lib, "../libs/ini.lib")
#pragma comment(lib, "../libs/iksemel.lib")
	//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}
	
#define CHECK_CNN_LOGIN \
	CnnDOG(0);\
	HandShake()\
	
#define IKS_PARSE(x,y)\
	iks_parse((x)->Prs,(const char*)(y),0,1)
/*
#define IKS_PARSE_EX(x,y,z)\
{  const char *current,*limit;\
    current=y;\
    limit=y+z-1;\
    while(current<limit)\
    {\
	Ret=iks_parse((x)->Prs,current,0,1);\
	current=strchr(current,'\0');\
	if(current == NULL)break;\
	++current;\
	}\
}
*/
#define IKS_PARSE_EX(x,y,z)\
	IKSParseEX(x,y,z)

#define	HANDSHAKE_STREAM	"<?xml version='1.0' encoding='UTF-8'?><stream:stream to='dog' xmlns='jabber:cat' xmlns:stream='http://etherx.jabber.org/streams'>"
#define	CLOSE_STREAM		"</stream:stream>"

#define E_OK 1
	
static CLIENTINFO		ClientInfo;
static SERVER			DOG;
static char				VMRecvBuf[MSGINITSIZE*256];
static char				VMSendBuf[1024];
	
int nCnnAnOK=0;

//typedef int (FunIOProcessDogMsg)(void *data,int iType,iks *node);
int (*IOProcessDogMsg)(void *data,int iType,iks *node);
//int (*IOHexMessage)(char *Node,size_t Len);

void InitCat(char *Name)
{
	StartupSocket();
	ClientInfo.Socket=INVALID_SOCKET;
	
	memset(VMSendBuf,0,1024);
	memset(VMRecvBuf,0,MSGINITSIZE);
	memset(ClientInfo.ReMsg,0,MSGINITSIZE);
	memset(ClientInfo.Name,0,sizeof(ClientInfo.Name));
	
	ClientInfo.pSndData=VMSendBuf;
	ClientInfo.iCurSendLen=0;
	ClientInfo.Prs=NULL;
	strcpy(ClientInfo.Name,Name);
	ClientInfo.Status=CLIENT_UNSHAKE;
	ClientInfo.Prs=iks_stream_new("jabber:dog",(void*)&ClientInfo,IOProcessDogMsg);	
}

void CnnDOG(void* v)	
{
	if(nCnnAnOK==1)return;
	if(ClientInfo.Socket!=INVALID_SOCKET)
	{
		CloseSocket(ClientInfo.Socket);
		ClientInfo.Socket=INVALID_SOCKET;
	}
	//??????????? 
	CreateSocketConnect(&ClientInfo.Socket,DOG.strIP,DOG.port);		//ret ?
	ClientInfo.SockStatus=STATUS_SOCKET_CONNECT;
	nCnnAnOK=1;

}

void HandShake()
{
	int len=iks_strlen(HANDSHAKE_STREAM);
	if(nCnnAnOK && ClientInfo.Status==CLIENT_UNSHAKE)
	{
		sprintf(VMSendBuf,"%s",HANDSHAKE_STREAM);
		ClientInfo.iCurSendLen=iks_strlen(VMSendBuf);
	}
}
static
void IKSParseEX(CLIENTINFO *x,char *y,unsigned int z)
{
	const char *current,*limit;
	int Ret=0;
	current=y;
	limit=(y)+(z)-1;

	while(current<limit)
	{
		Ret=iks_parse((x)->Prs,current,0,1);
		switch(Ret)
		{
		case IKS_OK:
			//puts ("OK");
			break;
		case IKS_NOMEM:
			puts ("Not enough memory");
			break;
		case IKS_BADXML:
			puts ("XML document is not well-formed");
			break;
		case IKS_HOOK:
			puts ("Our hooks didn't like something");
			break;
		}
		if(Ret!=IKS_OK) iks_parser_reset(ClientInfo.Prs);

		current=strchr(current,'\0');
		if(current == NULL) break;
			++current;
	}
}
static
int IOProcessParser(char *stream,unsigned int len)
{	
	int Ret=0;
	if(len<=0)return 0;
	//return_if_fail(pClientInfo);
	if(iks_strcasecmp(stream,CLOSE_STREAM)==0)
	{
		ClientInfo.Status=CLIENT_STOPPED;
	}

//	IKS_PARSE_EX(&ClientInfo,stream,len);
	Ret = IKS_PARSE(&ClientInfo,stream);
///*
	switch(Ret)
	{
	case IKS_OK:
		//puts ("OK");
		break;
	case IKS_NOMEM:
		puts ("Not enough memory");
		break;
	case IKS_BADXML:
		puts ("XML document is not well-formed");
		break;
	case IKS_HOOK:
		puts ("Our hooks didn't like something");
		break;
	}
	if(Ret!=IKS_OK) iks_parser_reset(ClientInfo.Prs);
//*/
	return 1;
}


DWORD WINAPI ClientRecSndThread(LPVOID lpParam)
{
	fd_set	rFds, wFds;
	struct timeval tv;
	int	runflag=1;
	int sn,rn,hb=0;
	time_t tn,tl;	
	tv.tv_sec =0;
	tv.tv_usec=1000;
	tn=tl=time(0);
	
	while(runflag)
	{
		tn=time(0);
		if(((tn-tl)>2)||((tl-tn)>2))
		{
			tl=tn;
			CHECK_CNN_LOGIN;
		}
		
		FD_ZERO( &rFds );
		FD_ZERO( &wFds );
		FD_SET(ClientInfo.Socket,&rFds);
		FD_SET(ClientInfo.Socket,&wFds);
		
		if(select(FD_SETSIZE,&rFds,&wFds,NULL,&tv)>0)
		{
			if((INVALID_SOCKET!=ClientInfo.Socket)&&FD_ISSET(ClientInfo.Socket,&rFds))
			{
				rn=recv(ClientInfo.Socket,VMRecvBuf,MSGINITSIZE*256,0);
				if(rn>0)
				{
					VMRecvBuf[rn]='\0';
					PRT_RAW(("RECV DATA LEN [%d] LAST[%c]:: %s\n",rn,VMRecvBuf[rn-1],VMRecvBuf));
					PRT_RAWSN(VMRecvBuf,rn);
					//PRT_OK("\n\t",VMRecvBuf,rn);
					IOProcessParser(VMRecvBuf,rn);
				}
				else if(rn<=0)
				{
					goto CLIENTTOEND;
				}
			}
			if(ClientInfo.iCurSendLen)
			{
				sn=send(ClientInfo.Socket,ClientInfo.pSndData,ClientInfo.iCurSendLen,0);
				if(sn>0)
				{
					ClientInfo.iCurSendLen-=sn;
					if(ClientInfo.iCurSendLen>0)
					{
						ClientInfo.pSndData+=sn;
					}
					Dbg(("\n\t*SEND DATA:: %s",ClientInfo.pSndData));
				}
				else if(sn<=0)
				{
CLIENTTOEND:		ClientInfo.SockStatus=STATUS_SOCKET_CLOSED;
					Dbg(("%d socket close \n",DOG.port));
					CloseSocket(ClientInfo.Socket);
					ClientInfo.Socket=INVALID_SOCKET;
					IOProcessParser(CLOSE_STREAM,iks_strlen(CLOSE_STREAM));
				}
			}
			else if(STATUS_SOCKET_CLOSED==ClientInfo.SockStatus)
			{
				goto CLIENTTOEND;
			}
			
		}
		Sleep(16);
	}
	return 0;
}

void WorkMain(void* v)
{
	int ret=0;
	DWORD ThId;	
	CreateThread(NULL,0,ClientRecSndThread,NULL,0,&ThId);
}	

int PushSendStream(char *pSendData,int len)
{
	//return_val_if_fail(pSendData,1);
	if(len == 0)
		len=strlen(pSendData);
	strncpy(VMSendBuf,pSendData,MSGINITSIZE);
	ClientInfo.iCurSendLen=len;

	return E_OK;
}

void SetClientStatus(CLIENTSTATUS Status)
{
	ClientInfo.Status=Status;
}

int Start(int iType,char *Name,void *pFunIOProcessDogMsg)
{
	cjIniReadString("cat","ip","0.0.0.0",DOG.strIP,32,".\\cat.cfg");
	DOG.port=cjIniReadInt("cat","port",0,".\\cat.cfg");	
	//return_val_if_fail(pFunIOProcessDogMsg,0);
	//return_val_if_fail(pFunIOHexMessage,0);
	if(strcmp(DOG.strIP,"0.0.0.0")==0 || DOG.port==0)
	{
		Dbg(("Read cat.cfg file error will exit now !!! "))
			Sleep(300);
		return 0;
	}
	Dbg(("--------------CAT START!!!! [dog ip=%s dog port=%d]---------------",DOG.strIP,DOG.port));
	IOProcessDogMsg=pFunIOProcessDogMsg;
	//IOHexMessage=pFunIOHexMessage;
	if(iType == CUTE_CAT_HOOKER)
	{	
	}
	else if(iType == CUTE_CAT_PIONEER)
	{	
	}
	InitCat(Name);
	WorkMain(NULL);
	return 1;
}

int Stop()
{
	//runflag=0;
	return 1;
}

	
#ifdef __cplusplus
}
#endif 
#endif /* __HEAD__*/