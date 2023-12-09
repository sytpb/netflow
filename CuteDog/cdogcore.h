#ifndef CDOGCORE_H
#define CDOGCORE_H
#include "list.h"
#include "stream.h"
#include "iksemel.h"
#include "typedef.h"
//????????? python chengwenchang yali ce shi
/*
typedef struct tagCCS
{
	CCSINIT				cCCSInit;
	unsigned long		IPAddress;

	int					SocketListen;
}CCSERVER,*PCCSERVER;
*/
typedef enum
{
	STATUS_SOCKET_CLOSED=0,
	STATUS_SOCKET_CONNECT
}CLIENTSOCKETSTATUS;

typedef enum
{
	STATUS_UNKNOWN,
	STATUS_ACCEPT,
	STATUS_TIMEOUT1,
	STATUS_TIMEOUT2,
	STATUS_BUSY,
	STATUS_IDLE,
	STATUS_ERRORLOGIN,
	STATUS_SHUNDOWN,
	STATUS_GETVSBASEINFO,
	STATUS_GETVSSTREAMINFO,
	STATUS_GETVSDECODEINFO,
	STATUS_GETVSDATAINFO,
	STATUS_RECBINARYDATA,
	STATUS_SENDBINARYDATA,

	STATUS_IOCMDCLOSE,//when send all buff,recsend unit auto close it
	STATUS_IOCLOSEED
		
}CLIENTSTATUS;

typedef enum
{
	CLIENT_UNKNOWN,
	CLIENT_USER_PIONEER,
	CLIENT_USER_HOOKER,
	CLIENT_USER_ADMIN
}CLIENTTYPE;
struct tagCLIENTINFO;
typedef	struct tagCLIENTINFO  CLIENTINFO;
typedef	struct tagCLIENTINFO* PCLIENTINFO;

typedef struct tagCUTEDOG 	  CUTEDOG;
typedef struct tagCUTEDOG	  *PCUTEDOG;

typedef struct
{
	struct	list_head	list;
	unsigned int		iSessionID;
	PCLIENTINFO			pClientInfo;
	HSTREAM				hStream;	
}DOG2CATIOMSG,*PDOG2CATIOMSG;

typedef struct tagCAT2DOGIOMSG
{
	unsigned int	iSessionID;
	PCLIENTINFO		pClientInfo;
	HSTREAM			hStream;			
}CAT2DOGIOMSG,*PCAT2DOGIOMSG;
struct tagCLIENTINFO
{
	struct	list_head	list;
	struct	list_head	list1;
	
	int					iID;
	CLIENTTYPE			lType;
	int					iGrpID;
	unsigned int		uiPriOpt;

	CLIENTSTATUS 		lStatus;

	CLIENTSOCKETSTATUS  lSocketStatus;
	unsigned long		IPAddress;
	char				strIP[ 32 ];

	unsigned long		lIPPort;/*ushort type convert*/

	iksparser			*Prs;
	int					Socket;
	unsigned int		iSessionID;
	unsigned int		iMsgNumID;

	PCAT2DOGIOMSG		pCurCat2DogIOMsg;

	int					iCurSendLen;
	unsigned char*		pSndData;		
	PDOG2CATIOMSG		pCurSendMsg;
	struct	list_head	ListSendMsg;

	PCUTEDOG			CuteDog;
	void*				Object;
	void(*FunFreeObj)(struct tagCLIENTINFO*);
};

enum
{
	PROTOCOL_TYPE_TCP = 0,
	PROTOCOL_TYPE_UDP
};

struct tagCUTEDOG
{
		struct 			list_head list;
		unsigned int 	iType;	
		char 			Name[32];
		char			Filter[256];
		PCLIENTINFO 	Master;
		struct 			list_head ListHooker;
		pthread_t		*Thid;
		void			*Handle;
		//char			Handle[0];
		//void*			Handle[0];
};

#define		CLIENTNUM			16

#define		CAT2DOGIOMSGNUM		4096

#define		MSGINITSIZE			1024		//1460+1??

#define		DOG2CATIOMSGNUM		4096


extern int runflag;
void 			CuteDogBroadCast();
int 			StopDogNose(PCLIENTINFO pClientInfo);

int 			SendTcp2Cats(const char*from,const char *fp,const char *to, const char*tp,const u_char *TcpData,size_t size,PCLIENTINFO pClientInfo);
void 			GetAllCuteDogsFilterString(char *Filter,size_t size);

PCUTEDOG 		AddCuteDog2List(void *Handle,void *Thid,const char *Name,const char *FilterStr,PCLIENTINFO pClientInfo);
int 			DelCuteDogInList(PCLIENTINFO pClientInfo);

int 			AddHooker2CuteDog(const char *CuteDogName,const char *CuteCatName,PCLIENTINFO pClientInfo);
int 			DelHookerInCuteDog(PCLIENTINFO pClientInfo);

int 			CheckNameInList(const char *name);
PDOG2CATIOMSG 	IoGetSendMsgCell(PCLIENTINFO);
void  			IoPutSendMsgCell(PDOG2CATIOMSG pSendMsg);

char *base64_decode(const char *buf);
char *base64_encode(const unsigned char *buf, int len);				

#define TIMEFMT "%04d-%02d-%02d %02d:%02d:%02d"


#endif
