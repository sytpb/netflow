#include "cdogcore.h"
#include "iksemel.h"

struct tagCLIENTINFO
{
	struct	list_head	list;
	
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

	PCAT2DOGIOMSG		pCurCat2DogIOMsg;

	int					iCurSendLen;
	unsigned char*		pSndData;		
	PDOG2CATMSG			pCurSendMsg;
	struct	list_head	ListSendMsg;

	void*				Object;
	void(*FunFreeObj)(struct tagCLIENTINFO*);
};

