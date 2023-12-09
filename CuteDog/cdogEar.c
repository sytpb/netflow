#include "list.h"
#include "iksemel.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
//#include "cdog.h"
#include "cdogcore.h"
#include "stream.h"
#include "fifo.h"

#define MAX_BACKLOG	32
//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}

#define PACK_NEW_SOCKETCLOSE_STREAM(x)\
		push_stream_string((x),"<SOCKETSHUTDOWN>");

#define IOSOCKET_CLOSE(x) strcmp((char*)get_stream_ptr((x)),"<SOCKETSHUTDOWN>")==0

#define INIT_DOG_STREAM(x)\
	(x)=((PDOG2CATIOMSG)pDog2CatIOMsgForFree)+i;\
	(x)->hStream=init_stream(MSGINITSIZE);

#define FIFOPOP(x,y)\
		FifoPop((x),(void**)(y))

#define FIFOPUSH(x,y)\
		FifoPush((x),(void*)(y));

#define POP_DOGMSG_2LISTTAIL \
{	PDOG2CATIOMSG pSendMsg;\
	while(RET_OK ==FIFOPOP(pFifoDogMsg,&pSendMsg))\
	{\
		pClientInfo=pSendMsg->pClientInfo;\
		if(pSendMsg->iSessionID!=pClientInfo->iSessionID\
						||pClientInfo->lSocketStatus!=STATUS_SOCKET_CONNECT)\
		{\
				FIFOPUSH(pFifoDogMsgFree,pSendMsg);\
				continue;\
		}\
		list_add_tail(&(pSendMsg->list),&(pClientInfo->ListSendMsg));\
	}\
}


typedef struct tagCCS
{
	int					SocketListen;
	char*				strIP;	
	unsigned short		port;
	char				bpf[32];
}SERVER,*PSERVER;
//typedef void*	IKS;

void IOProcessParser(PCLIENTINFO pClientInfo,HSTREAM hStream);
int  IOProcessCatMsg(void *data,int type,iks *node);

SERVER		server;
CLIENTINFO	ClientInfo[CLIENTNUM];
LISTHEAD	FreeClientList;
LISTHEAD	WorkClientList;
LISTHEAD	CuteDogList;

//MUTEX		mutexWorkClientList;
//CLIENTINFO	ClientInfo[CLIENTNUM];
//LISTHEAD	FreeClientList;
//LISTHEAD	WorkClientList;

void*			pCat2DogIOMsgForFree=NULL;
PCAT2DOGIOMSG	Cat2DogIOMsg[CAT2DOGIOMSGNUM];
unsigned int	uiCliPutMsgIndex=0;
unsigned int 	uiIoGetMsgIndex=0;
unsigned int 	uiCliGetMsgIndex=1;	
unsigned int 	uiCliGetMsgNextIndex=2;

void*			pDog2CatIOMsgForFree=NULL;
//FifoRing  		FifoDogMsg;				gcc error hide struct ..
//FifoRing  		FifoDogMsgFree;
FifoRing		*pFifoDogMsg=NULL;
FifoRing		*pFifoDogMsgFree=NULL;

int Load_Config()
{
	iks *x, *y;
	int e=0,ret=0;
	e = iks_load ("cdog.xml", &x);
	if (e != IKS_OK) 
		printf ("parse error %d\n", e);
	if(iks_find (x, "end")) puts ("XML doc correct !!");
	y = iks_child (x);									
	if(y)
	{
		if (iks_type (y) == IKS_CDATA)
		{ 
			server.strIP=iks_find_cdata(x,"ip");
			//printf("IP:%s\n",server.strIP);
			y = iks_next (y);
			server.port=atoi(iks_find_cdata(x,"port"));
			//printf("port:%d\n",server.port);
			strncpy(server.bpf,iks_find_cdata(x,"bpf"),sizeof(server.bpf));
			//printf("bpf: %s \n",server.bpf);
			Dbg(("Server IP [%s] Port [%d] Default Option [%s]",server.strIP,server.port,server.bpf));
			ret=1;
		}	
	}
	iks_delete (x);
	return ret;
}
static char *my_strcat (char *dest, char *src, size_t len)
{
		if (0 == len) len = strlen (src);
		memcpy (dest, src, len);
		return dest + len;
}

static char *escape (char *dest, char *src, size_t len)
{
		char c;
		int i;
		int j = 0;

		for (i = 0; i < len; i++) {
				c = src[i];
				if ('&' == c || '<' == c || '>' == c || '\'' == c || '"' == c) {
						if (i - j > 0) dest = my_strcat (dest, src + j, i - j);
						j = i + 1;
						switch (c) {
								case '&': dest = my_strcat (dest, "&amp;", 5); break;
								case '\'': dest = my_strcat (dest, "&apos;", 6); break;
								case '"': dest = my_strcat (dest, "&quot;", 6); break;
								case '<': dest = my_strcat (dest, "&lt;", 4); break;
								case '>': dest = my_strcat (dest, "&gt;", 4); break;
						}
				}
		}
		if (i - j > 0) dest = my_strcat (dest, src + j, i - j);
		return dest;
}
void tst_base64(char *tcpData,int size)
{
		char Hello[128]={0},Hello1[128]={0};
		char *Enchar,*Dechar,*pTT;
		char TT[]={0x84,0x09,0x0f,0x26,0x1f,0x22,0x8f,0x41,0x4f,0x42,0x03,0x83,0xad,0xe4,0x36,0xa3,0x0,0xf9,
				0x58,0xdc,0xfc,0x74,0xfe,0x19,0x7b,0x14,0xe3,0x7d,0xb4,0x0e,0x18,0x7c,0x0e,0x66,0x3b,0x25};
		char *tcpDataEncode,*tcpDecode;
		int i=0;
		for(i=0;i<size;++i)
		{
				printf("%02x\t",tcpData[i]);
		}
		printf(" \n end ####### tcpData LEN=%d\n",size);
		tcpDataEncode=base64_encode((unsigned char*)tcpData,size);
		tcpDecode=base64_decode(tcpDataEncode);
		for(i=0;i<size;++i)
		{
				printf("%02x\t",tcpDecode[i]);
		}
		printf("\n-------------- DE DE DE .......\n");
		pTT=(char*)TT;
		memcpy(Hello1,TT,36);
		for(i=0;i<36;++i)
				printf("%02x\t",Hello1[i]);

		printf("\n------------------------------------------------------------------- \n");
		Enchar=iks_base64_encode(pTT,36);
		return_if_fail(Enchar);
		Dechar=iks_base64_decode(Enchar);
		return_if_fail(Dechar);
		memcpy(Hello,Dechar,36);

		for(i=0;i<36;++i)
				printf("%02x\t",Hello[i]);

}
/*1024 muti 3/4=768*/
/*4096 muti 3/4=3072*/
#define MAX_TCPDATA_SIZE	3072
int SendTcp2Cats(const char*from,const char *fp,const char *to, const char*tp,const u_char *tcpData,size_t size,PCLIENTINFO pClientInfo)
{
		char message[4096+512]={0}/*,body[1024]={0}*/;
		char *pEnBody=NULL;
		size_t size1=0;
		PDOG2CATIOMSG	pDogMsg;
		PCUTEDOG		pCuteDog;
		HSTREAM hRespStream;
		return_val_if_fail(pClientInfo,0);
		pDogMsg=IoGetSendMsgCell(pClientInfo);
		return_val_if_fail(pDogMsg,0);
		hRespStream=pDogMsg->hStream;
		//escape(body,(char*)tcpData,size);
		size1=size > MAX_TCPDATA_SIZE ? MAX_TCPDATA_SIZE:size;
		pEnBody=base64_encode(tcpData,size1);
		++pClientInfo->iMsgNumID;
		snprintf(message,sizeof(message),"<message from='%s' fp='%s' to='%s' tp='%s'><body size='%d' rawsize='%d' sn='%d'>%s</body></message>",from,fp,to,tp,size1,size,pClientInfo->iMsgNumID,pEnBody);
		//Dbg(("******* Message:%s",message));
		if(hRespStream)
		{
				push_stream_string(hRespStream,message);
				IoPutSendMsgCell(pDogMsg);
		}
		if(pEnBody!=NULL)
		{
			free(pEnBody);
			pEnBody=NULL;
		}
		pCuteDog=pClientInfo->CuteDog;
		if(list_notempty(&pCuteDog->ListHooker))
		{
				PCLIENTINFO tmpClientInfo;
				LISTHEAD *entrytmp=NULL;
				list_for_each(entrytmp,&(pCuteDog->ListHooker))
				{
						tmpClientInfo=list_entry(entrytmp,CLIENTINFO,list1);
						pDogMsg=IoGetSendMsgCell(tmpClientInfo);
						return_val_if_fail(pDogMsg,0);
						hRespStream=pDogMsg->hStream;
						if(hRespStream)
						{
								push_stream_string(hRespStream,message);
								IoPutSendMsgCell(pDogMsg);
						}
				}
		}

		return 1;
}

void GetAllCuteDogsFilterString(char *Filter,size_t size)
{
		LISTHEAD	*entrytmp = NULL;
		PCUTEDOG	pCuteDog = NULL;
		char 		Row[128]={0};
		char 		*head=Filter;
		size_t		pos=0;

		if(list_notempty(&CuteDogList))
		{
				list_for_each(entrytmp,&(CuteDogList))
				{
						pCuteDog=list_entry(entrytmp,CUTEDOG,list);
						snprintf(Row,sizeof(Row),"<name>%s</name><filterstring>%s</filterstring>",pCuteDog->Name,pCuteDog->Filter);
						Dbg(("---------------------- Name:%s   Filter:%s",pCuteDog->Name,pCuteDog->Filter));
						snprintf(head,size-pos,"%s",Row);
						head+=pos;
				}
		}

}

void CuteDogBroadCast()
{
		LISTHEAD	*entry = NULL,*next=NULL;
		PCLIENTINFO pTmpClientInfo=NULL;
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		char 	Body[1024+128]={0},Rows[1024]={0};
		GetAllCuteDogsFilterString(Rows,sizeof(Rows));
		if(*Rows!='\0')
		{		
				snprintf(Body,sizeof(Body),"%s%s%s","<iq type='result'><query xmlns='filter:show'>",Rows,"</query></iq>");
		}
		else
		{
				snprintf(Body,sizeof(Body),"%s","<iq type='result'><query xmlns='filter:show' ></query></iq>");
		}
		if(list_notempty(&WorkClientList))
		{
				list_for_each_safe(entry,next,&(WorkClientList))	//worklist
				{
						pTmpClientInfo=list_entry(entry,CLIENTINFO,list);
						if(pTmpClientInfo) 
						{
								pDogMsg=IoGetSendMsgCell(pTmpClientInfo);
								return_if_fail(pDogMsg);
								hRespStream=pDogMsg->hStream;
								
								pDogMsg=IoGetSendMsgCell(pTmpClientInfo);
								return_if_fail(pDogMsg);
								hRespStream=pDogMsg->hStream;
								if(hRespStream)
								{
										push_stream_string(hRespStream,Body);
										IoPutSendMsgCell(pDogMsg);
								}
						}
				}
		}

}
int AddHooker2CuteDog(const char *CuteDogName,const char *CuteCatName,PCLIENTINFO pClientInfo)
{
		LISTHEAD	*entrytmp = NULL;
		PCUTEDOG	pCuteDog = NULL;
		Dbg(("AddHooker2CuteDog************* [%s] [%s] %p",CuteDogName,CuteCatName,pClientInfo));
		return_val_if_fail(CuteDogName,0);
		return_val_if_fail(pClientInfo,0);
		if(list_notempty(&CuteDogList))
		{
				list_for_each(entrytmp,&(CuteDogList))
				{
						pCuteDog=list_entry(entrytmp,CUTEDOG,list);
						if(strcmp(CuteDogName,pCuteDog->Name)==0)
						{
								list_add(&(pClientInfo->list1),&pCuteDog->ListHooker);
								pClientInfo->CuteDog=pCuteDog;
								Dbg(("CuteDog:%s Hooked by:%s CuteDogList:%p,listHooker:%p pCuteDogName:%s",CuteDogName,CuteCatName,&CuteDogList,&pCuteDog->ListHooker,pCuteDog->Name));
								return 1;
						}
				}
		}
		Dbg(("Hooker not find item %s",CuteDogName));
		//resp Hooker fail ?
		return 0;
}

int DelHookerInCuteDog(PCLIENTINFO pClientInfo)
{
		LISTHEAD	*entry = NULL,*next=NULL;
		LISTHEAD	*entry1 = NULL,*next1=NULL;
		PCUTEDOG	pCuteDog = NULL;
		PCLIENTINFO pTmpClientInfo=NULL;
		
		if(list_notempty(&CuteDogList))
		{
				list_for_each_safe(entry,next,&(CuteDogList))
				{
					pCuteDog=list_entry(entry,CUTEDOG,list);
					Dbg(("**************** name:%s CuteDogList:%p",pCuteDog->Name,&CuteDogList));
					if(list_notempty(&pCuteDog->ListHooker))
					{
							list_for_each_safe(entry1,next1,&(pCuteDog->ListHooker))
							{
									pTmpClientInfo=list_entry(entry1,CLIENTINFO,list1);
									if(pTmpClientInfo==pClientInfo) 
									{
											list_del(&pClientInfo->list1);
											return 1;
									}

							}
					}
				}
		}
		return 0;
}
int DelHookersWithCuteDog(PCUTEDOG pCuteDog)
{
		PCLIENTINFO pTmpClientInfo=NULL;
		LISTHEAD	*entry = NULL,*next=NULL;
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		char message[512]={0};
		if(list_notempty(&pCuteDog->ListHooker))
		{
				list_for_each_safe(entry,next,&(pCuteDog->ListHooker))
				{
						pTmpClientInfo=list_entry(entry,CLIENTINFO,list1);
						pDogMsg=IoGetSendMsgCell(pTmpClientInfo);
						return_val_if_fail(pDogMsg,0);
						hRespStream=pDogMsg->hStream;
						snprintf(message,sizeof(message),"<iq type='set'><query xmlns='filter:notice' action='lose'><username>%s</username></query></iq>",pCuteDog->Name);
						if(hRespStream)
						{
								push_stream_string(hRespStream,message);
								IoPutSendMsgCell(pDogMsg);
						}
						list_del(&pTmpClientInfo->list1);
				}
		}
		return 1;
}
int DelCuteDogInList(PCLIENTINFO pClientInfo)
{
		PCUTEDOG pCuteDog=NULL;
		return_val_if_fail(pClientInfo->CuteDog,0);
		pCuteDog=pClientInfo->CuteDog;
		return_val_if_fail(pCuteDog,0);
		DelHookersWithCuteDog(pCuteDog);
		list_del(&pCuteDog->list);
		if(pClientInfo->CuteDog)
		{
				pCuteDog->Handle=NULL;
				pCuteDog->Thid=NULL;
				Dbg(("DelCuteDog::Del a Cutedog of [%s]%p",pCuteDog->Name,pCuteDog));
				free(pCuteDog);
				pClientInfo->CuteDog=NULL;
				return 1;
		}
		return 0;
}
PCUTEDOG AddCuteDog2List(void *Handle,void *Thid,const char *Name,const char *FilterStr,PCLIENTINFO pClientInfo)
{
		return_val_if_fail(Handle,NULL);
		PCUTEDOG tmpCuteDog =(PCUTEDOG)malloc(sizeof(CUTEDOG));
		memset(tmpCuteDog,0,sizeof(CUTEDOG));

		tmpCuteDog->Master=pClientInfo;
		strncpy(tmpCuteDog->Name,Name,sizeof(tmpCuteDog->Name)-1);
		strncpy(tmpCuteDog->Filter,FilterStr,sizeof(tmpCuteDog->Filter)-1);	//???
		tmpCuteDog->Handle=Handle;
		tmpCuteDog->Thid=Thid;

		list_add(&(tmpCuteDog->list),&CuteDogList);
		INIT_LIST_HEAD(&tmpCuteDog->ListHooker);
		pClientInfo->CuteDog=tmpCuteDog;

		Dbg(("AddCuteDog2List %p strip=%s ListHooker:%p",tmpCuteDog,tmpCuteDog->Master->strIP,&tmpCuteDog->ListHooker));
		return tmpCuteDog;
}
int CheckNameInList(const char *Name)
{	
		LISTHEAD	* entrytmp = NULL;
		PCUTEDOG	 pCuteDog = NULL;
		if(list_notempty(&CuteDogList))
		{
				list_for_each(entrytmp,&(CuteDogList))
				{
						pCuteDog=list_entry(entrytmp,CUTEDOG,list);
						if(strcmp(Name,pCuteDog->Name)==0)return 1;
				}
		}
		return 0;
}

PDOG2CATIOMSG IoGetSendMsgCell(PCLIENTINFO pClientInfo)
{
	PDOG2CATIOMSG pSendMsg;
	if(RET_FAIL==FIFOPOP(pFifoDogMsgFree,&pSendMsg))
	{
			Dbg(("IoGetSendMsgCell Fail!!!!!!!!! must rebuild program for this setting"));
			return NULL;
	}

	pSendMsg->pClientInfo=pClientInfo;
	pSendMsg->iSessionID=pClientInfo->iSessionID;
	reset_stream(pSendMsg->hStream);
	return pSendMsg;
}

void IoPutSendMsgCell(PDOG2CATIOMSG pSendMsg)
{
	return_if_fail(pSendMsg);
	FIFOPUSH(pFifoDogMsg,pSendMsg);
}

int  CreateSocketListen(int * fd, char *ip, unsigned short port,int Nonblock)
{
	struct sockaddr_in addr;
	int ret,flag=1,len;
	int sockfd;
	len=sizeof(int);
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
		flag = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, len)<0)
	{
		perror("fcntl(sock,SETFL,opts)");   
		exit(1);
	}
	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0)
	{
		close(sockfd);
		return -2;
	}
	ret = listen(sockfd, MAX_BACKLOG);
	if(ret < 0)
	{
		close(sockfd);
		return -3;
	}	
	*fd = sockfd;
	return 0;
}
int runflag=1;
void* processIOThread(void *v)
{
	PCAT2DOGIOMSG pCat2DogIOMsg;
	PCLIENTINFO pClientInfo;
	HSTREAM hStream;
	Dbg(("processio \n"));
	while(runflag)
	{

			if(uiCliPutMsgIndex==uiIoGetMsgIndex)
			{
					usleep(16*1000);
					continue;
			}
			pCat2DogIOMsg=Cat2DogIOMsg[uiIoGetMsgIndex];
			pClientInfo=pCat2DogIOMsg->pClientInfo;
			hStream=pCat2DogIOMsg->hStream;
			if(pCat2DogIOMsg->iSessionID!=pClientInfo->iSessionID||STATUS_IOCMDCLOSE==pClientInfo->lStatus||pClientInfo==NULL)
			{//log ????		
			}
			else
			{
					IOProcessParser(pClientInfo,hStream);	
					if(IOSOCKET_CLOSE(hStream))
					{
							pClientInfo->lStatus=STATUS_IOCLOSEED;
							pClientInfo->FunFreeObj(pClientInfo);
							//pClientInfo->Object=NULL;
					}
					//IOProcessParser(pClientInfo,hStream);	
			}
			uiIoGetMsgIndex=(uiIoGetMsgIndex+1)%CAT2DOGIOMSGNUM;
			//Dbg(("PUT INDEX :[%d] GET INDEX [%d]",uiCliPutMsgIndex,uiIoGetMsgIndex));
	}
	return (void*)0;
}

static unsigned char CliRecSocketBuf[256*1024];
unsigned int ClientRecSndThread(void *v)
{
	fd_set	rFds, wFds;
	struct timeval tv;

	int socklen,tmp;
	struct sockaddr_in srccaddr;
	struct in_addr addr;
	int sn,rn;
	int sd_accept;
	PCLIENTINFO pClientInfo;
	PDOG2CATIOMSG pSendMsg;
	LISTHEAD	* entry = NULL, * next = NULL;
	LISTHEAD	* entrytmp = NULL;
	unsigned char *endbuf,*lpBuf;
	time_t tn,tl;
	int maxfd=server.SocketListen;
	unsigned int	iSessionID=0;

	tv.tv_sec =0;
	tv.tv_usec=1000;
	tn=tl=time(0);
	while(1)
	{
		tn=time(0);
		if(((tn-tl)>5)||((tl-tn)>5))
		{
			tl=tn;
		}
		POP_DOGMSG_2LISTTAIL;
		FD_ZERO( &rFds );
		FD_ZERO( &wFds );
		FD_SET(server.SocketListen,&rFds);
		list_for_each_safe(entry,next,&WorkClientList)
		{
			pClientInfo=list_entry(entry,CLIENTINFO,list);
			if(STATUS_IOCLOSEED == pClientInfo->lStatus)
			{
				if(list_notempty(&(pClientInfo->ListSendMsg)))
				{
						list_for_each(entrytmp,&(pClientInfo->ListSendMsg))
						{
								pSendMsg=list_entry(entrytmp,DOG2CATIOMSG,list);
								FIFOPUSH(pFifoDogMsgFree,pSendMsg);
						}
						INIT_LIST_HEAD(&(pClientInfo->ListSendMsg));
				}
				if(pClientInfo->pCurSendMsg)
				{
						FIFOPUSH(pFifoDogMsgFree,pClientInfo->pCurSendMsg);
						pClientInfo->pCurSendMsg=NULL;
				}
				close(pClientInfo->Socket);
				pClientInfo->iSessionID		=0;
				pClientInfo->iID			= -1;
				pClientInfo->lSocketStatus	=STATUS_SOCKET_CLOSED;
				pClientInfo->lStatus		=STATUS_UNKNOWN;
				pClientInfo->lType			=CLIENT_UNKNOWN;
				
				if(pClientInfo->Prs) iks_parser_delete(pClientInfo->Prs); 
				pClientInfo->Prs=NULL;
				//Lock(mutex);
				list_move(&(pClientInfo->list),&FreeClientList);
				//Unlock(mutex);
				Dbg(("release res %s:%ld",pClientInfo->strIP,pClientInfo->lIPPort));
				Dbg(("PUT INDEX :[%d] GET INDEX [%d]",uiCliPutMsgIndex,uiIoGetMsgIndex));
				continue;
			}
			if(STATUS_SOCKET_CLOSED == pClientInfo->lSocketStatus)
			{
					continue;
			}				
			if(pClientInfo->iCurSendLen>0)
			{
					FD_SET(pClientInfo->Socket,&wFds);
			}
			else if(list_notempty(&(pClientInfo->ListSendMsg)))
			{
					pSendMsg=list_entry(pClientInfo->ListSendMsg.next,DOG2CATIOMSG,list);
					pClientInfo->iCurSendLen=get_stream_len(pSendMsg->hStream);
					pClientInfo->pSndData=get_stream_ptr(pSendMsg->hStream);
					pClientInfo->pCurSendMsg=pSendMsg;
					list_del(&(pSendMsg->list));
					FD_SET(pClientInfo->Socket,&wFds);
			}
			if(STATUS_IOCMDCLOSE!=pClientInfo->lStatus)
			{
				FD_SET(pClientInfo->Socket,&rFds);
			}
		}
		if(select(maxfd+1,&rFds,&wFds,NULL,&tv)>0)
		{
			if(FD_ISSET(server.SocketListen,&rFds))
			{
				socklen=sizeof(srccaddr);
				sd_accept=accept(server.SocketListen,(struct sockaddr*)&srccaddr,(socklen_t*)&socklen);

				if(-1==sd_accept)
				{

				}
				else if(list_empty(&FreeClientList))
				{
					//log ???
					close(sd_accept);
				}
				else
				{
					maxfd=sd_accept;
					tmp=1;
					setsockopt(sd_accept,SOL_SOCKET,SO_REUSEADDR,(char*)&tmp, sizeof(tmp));
					//setsockopt(sd_accept,SOL_SOCKET,TCP_NODELAY ,(char*)&tmp, sizeof(tmp));
					//cjSetSocketNonblock(sd_accept);
					pClientInfo=list_entry(FreeClientList.next,CLIENTINFO,list);

					pClientInfo->lStatus		=STATUS_SOCKET_CONNECT;
					//tmpCuteDog->Name[32]='\0';
					pClientInfo->Socket			=sd_accept;
					pClientInfo->IPAddress		=srccaddr.sin_addr.s_addr;
					pClientInfo->lIPPort		=srccaddr.sin_port;
					addr.s_addr					=srccaddr.sin_addr.s_addr;
					strcpy(pClientInfo->strIP,inet_ntoa(addr));
					pClientInfo->iSessionID		=++iSessionID;
					pClientInfo->iMsgNumID		=0;
					//pClientInfo->iID			= -1;
					pClientInfo->lSocketStatus	=STATUS_SOCKET_CONNECT;
					pClientInfo->lStatus		=STATUS_UNKNOWN;
					pClientInfo->lType			=CLIENT_UNKNOWN;
					pClientInfo->Prs=iks_stream_new("jabber::cat",pClientInfo,IOProcessCatMsg);
					//					Lock(mutex);
					list_move(&(pClientInfo->list),&WorkClientList);//move to work list at the end
					//					unlock(mutex);
					Dbg(("%s:%ld:%u connect",pClientInfo->strIP,pClientInfo->lIPPort,pClientInfo->iSessionID));
				}
			}
			list_for_each_safe(entry,next,&WorkClientList)
			{
				pClientInfo=list_entry(entry,CLIENTINFO,list);
				if((STATUS_IOCMDCLOSE!=pClientInfo->lStatus)&&FD_ISSET(pClientInfo->Socket,&rFds))
				{
					rn=recv(pClientInfo->Socket,CliRecSocketBuf,256*1024,0);
					if(rn>0)
					{
						lpBuf = (unsigned char*)CliRecSocketBuf;
						while(lpBuf < CliRecSocketBuf + rn)
						{	
							if(NULL==pClientInfo->pCurCat2DogIOMsg)
							{
									if(uiCliGetMsgNextIndex==uiIoGetMsgIndex)
									{
											//log full !!!
											break;
									}
									pClientInfo->pCurCat2DogIOMsg=Cat2DogIOMsg[uiCliGetMsgIndex];
									uiCliGetMsgIndex=uiCliGetMsgNextIndex;
									uiCliGetMsgNextIndex=(uiCliGetMsgNextIndex+1)%CAT2DOGIOMSGNUM;
									pClientInfo->pCurCat2DogIOMsg->iSessionID=pClientInfo->iSessionID;
									pClientInfo->pCurCat2DogIOMsg->pClientInfo=pClientInfo;
									reset_stream(pClientInfo->pCurCat2DogIOMsg->hStream);
									//Dbg(("%s:%ld:%u recv",pClientInfo->strIP,pClientInfo->lIPPort,pClientInfo->iSessionID));
							}
							endbuf = copy_stream((unsigned char*)lpBuf,(unsigned char*)CliRecSocketBuf + rn,pClientInfo->pCurCat2DogIOMsg->hStream);
							if(endbuf == NULL)
							{
									break;		//??
							}
							Cat2DogIOMsg[uiCliPutMsgIndex]=pClientInfo->pCurCat2DogIOMsg;
							pClientInfo->pCurCat2DogIOMsg=NULL;
							uiCliPutMsgIndex=(uiCliPutMsgIndex+1)%CAT2DOGIOMSGNUM;
							lpBuf = endbuf;
						}	
					}
					else if(rn<=0)
					{
						goto CLIENTTOEND;
					}
				}
				if(pClientInfo->iCurSendLen)
				{
					sn=send(pClientInfo->Socket,(char*)(pClientInfo->pSndData),pClientInfo->iCurSendLen,0);
					if(sn>0)
					{
							//Dbg(("send to:[%lu]:[%s]",pClientInfo->lIPPort,(char*)pClientInfo->pSndData));
							pClientInfo->iCurSendLen-=sn;
							if(pClientInfo->iCurSendLen>0)
							{
									pClientInfo->pSndData+=sn;
							}
							else
							{
									if(pClientInfo->pCurSendMsg)
									{
											FIFOPUSH(pFifoDogMsgFree,pClientInfo->pCurSendMsg);
									}
									pClientInfo->iCurSendLen=0;
									pClientInfo->pCurSendMsg=NULL;
									if(list_notempty(&(pClientInfo->ListSendMsg)))
									{
											pSendMsg=list_entry(pClientInfo->ListSendMsg.next,DOG2CATIOMSG,list);
											pClientInfo->iCurSendLen=get_stream_len(pSendMsg->hStream);
											pClientInfo->pSndData=get_stream_ptr(pSendMsg->hStream);
											//Dbg(("****sndData:%s",pClientInfo->pSndData));
											pClientInfo->pCurSendMsg=pSendMsg;
											list_del(&(pSendMsg->list));

									}
									else if(STATUS_IOCMDCLOSE==pClientInfo->lStatus)
									{
											goto CLIENTTOEND;
									}
							}
					}
					else if(sn<=0)
					{
CLIENTTOEND:					pClientInfo->lStatus=STATUS_SOCKET_CLOSED;
								if(NULL==pClientInfo->pCurCat2DogIOMsg)
								{
										if(uiCliGetMsgNextIndex==uiIoGetMsgIndex)
										{
												continue;
										}
										pClientInfo->pCurCat2DogIOMsg=Cat2DogIOMsg[uiCliGetMsgIndex];
										uiCliGetMsgIndex=uiCliGetMsgNextIndex;
										uiCliGetMsgNextIndex=(uiCliGetMsgNextIndex+1)%CAT2DOGIOMSGNUM;
										pClientInfo->pCurCat2DogIOMsg->iSessionID=pClientInfo->iSessionID;
										pClientInfo->pCurCat2DogIOMsg->pClientInfo=pClientInfo;
										reset_stream(pClientInfo->pCurCat2DogIOMsg->hStream);
								}
								//pack_new(shutdown);
								PACK_NEW_SOCKETCLOSE_STREAM(pClientInfo->pCurCat2DogIOMsg->hStream);
								Cat2DogIOMsg[uiCliPutMsgIndex]=pClientInfo->pCurCat2DogIOMsg;
								pClientInfo->pCurCat2DogIOMsg=NULL;
								uiCliPutMsgIndex=(uiCliPutMsgIndex+1)%CAT2DOGIOMSGNUM;
								//iks_parser_delete (pClientInfo->Prs);//?????
								//close(pClientInfo->Socket);
								Dbg(("%s close",pClientInfo->strIP));
								usleep(16);
					}
				}
				else if(STATUS_IOCMDCLOSE==pClientInfo->lStatus)
				{
					goto CLIENTTOEND;
				}
			}
		}
		usleep(8*1000);
	}
	return 0;
}

void FreeObj(PCLIENTINFO pClientInfo)
{
	if(pClientInfo->Prs)iks_parser_delete(pClientInfo->Prs); 
	pClientInfo->Prs=NULL;
	if(pClientInfo->lType==CLIENT_USER_PIONEER)
	{
			StopDogNose(pClientInfo); 
			DelCuteDogInList(pClientInfo);
			Dbg(("FreeOjb:: Del CuteDog [%ld]........",pClientInfo->lIPPort));
			//StopDogNose(pClientInfo); 
	}
	else if(pClientInfo->lType==CLIENT_USER_HOOKER)
	{
			DelHookerInCuteDog(pClientInfo);
			Dbg(("FreeOjb:: Del Hooker [%ld] ........",pClientInfo->lIPPort));
	}
}

#define E_MALLOC 0
int InitServer()
{
	int iRet=1;
	int i;

	memset(Cat2DogIOMsg,0,sizeof(Cat2DogIOMsg));
	pCat2DogIOMsgForFree=malloc(sizeof(CAT2DOGIOMSG)*(CAT2DOGIOMSGNUM-1));
	for(i=0;i<CAT2DOGIOMSGNUM-1;i++)
	{
			Cat2DogIOMsg[i+1]=((PCAT2DOGIOMSG)(pCat2DogIOMsgForFree))+i;
			Cat2DogIOMsg[i+1]->hStream=init_stream(MSGINITSIZE);
			if(NULL==Cat2DogIOMsg[i+1]->hStream)
			{
					for(i--;i>=0;i--)
					{
							del_stream(Cat2DogIOMsg[i+1]->hStream);	
					}
					free(pCat2DogIOMsgForFree);
					pCat2DogIOMsgForFree=NULL;
					return E_MALLOC;
			}
	}
	memset(ClientInfo,0,sizeof(ClientInfo));
	INIT_LIST_HEAD(&FreeClientList);
	INIT_LIST_HEAD(&WorkClientList);
	INIT_LIST_HEAD(&CuteDogList);
	for(i=0;i<CLIENTNUM;i++)
	{
		list_add(&(ClientInfo[i].list),&FreeClientList);
		INIT_LIST_HEAD(&(ClientInfo[i].ListSendMsg));
		ClientInfo[i].FunFreeObj=FreeObj;
	}
	pFifoDogMsg=FifoNew(DOG2CATIOMSGNUM);
	pFifoDogMsgFree=FifoNew(DOG2CATIOMSGNUM);
	pDog2CatIOMsgForFree=malloc(sizeof(DOG2CATIOMSG)*(DOG2CATIOMSGNUM-1));
	for(i=0;i<DOG2CATIOMSGNUM-1;i++)
	{
			PDOG2CATIOMSG pTmpDogMsg=NULL;
			INIT_DOG_STREAM(pTmpDogMsg);
			if(NULL==pTmpDogMsg->hStream)
			{
					for(i--;i>=0;i--)
					{
							FIFOPOP(pFifoDogMsgFree,&pTmpDogMsg);
							del_stream(pTmpDogMsg->hStream);	
					}
					free(pDog2CatIOMsgForFree);
					pDog2CatIOMsgForFree=NULL;
					return E_MALLOC;
			}
			//FifoPush(pFifoDogMsgFree,pDog2CatIOMsgForFree+i);
			FIFOPUSH(pFifoDogMsgFree,pTmpDogMsg);
	}

	// 	mutexWorkClientList=cjMutexInit();
	if(!Load_Config ())return 0;
	if(CreateSocketListen(&server.SocketListen,server.strIP,(unsigned short)server.port,1))
	{
		Dbg(("IP:%s Port:%d,Create Listen Socket fail",server.strIP,server.port));
		return 0;
	}	
	return iRet;
}

int StartEar(void)
{
	pthread_t tidp;
	if(!InitServer())
	{
			Dbg(("Init Error.... !!!!"));
			return 0;
	}
	if(!(pthread_create(&tidp,NULL,processIOThread,(void*)NULL)))
	{
		ClientRecSndThread((void*)NULL);
		Dbg(("error exit now! \n"));
	}
	pthread_join(tidp,NULL);
	return 0;
}
/*
int IOStart(void)
{
#ifdef SER_DOG
	Dbg(("You Started a CDOG SERVER !!"));	
	StartEar();
#else
	Dbg(("you started a perfessioal cdog  !!"));	
	StartDogNose(NULL);
#endif

}*/
