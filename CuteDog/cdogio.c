#include "iksemel.h"
#include "cdogcore.h"
#include "typedef.h"
#include "stream.h"
#include <string.h>
#include <unistd.h>

//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}
#define IKS_PARSE(x,y)\
		iks_parse((x)->Prs,(const char*)(y),0,1)
#define GET_STREAM_PTR(x)\
		(const char*)get_data_ptr((x));

int CatHandShake(iks *node,PCLIENTINFO pClientInfo)
{
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		
		return_val_if_fail(pClientInfo,0);
		pDogMsg=IoGetSendMsgCell(pClientInfo);
		return_val_if_fail(pDogMsg,0);
		hRespStream=pDogMsg->hStream;
		// ED  maro from ??
		push_stream_string(hRespStream,"<stream:stream from='dog' xmlns='jabber:cat' xmlns:stream='http://etherx.jabber.org/streams' id='00'>");
		IoPutSendMsgCell(pDogMsg);
		return 1;
}
int StartDogNose(const char*,const char*,PCLIENTINFO,int);

int RespCatLogin0(iks *node,PCLIENTINFO pClientInfo)
{
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		const char *DogName=NULL;	
		return_val_if_fail(pClientInfo,0);
		DogName=iks_find_cdata(node,"username");

		pDogMsg=IoGetSendMsgCell(pClientInfo);
		return_val_if_fail(pDogMsg,0);
		hRespStream=pDogMsg->hStream;
		if(DogName==NULL)return 0;	
		if(!CheckNameInList(DogName))
		{
			push_stream_string(hRespStream,"<iq type='result'><query xmlns='jabber:iq:auth'><error id='OK'/></query></iq>");
		}
		else
		{
			push_stream_string(hRespStream,"<iq type='result'><query xmlns='jabber:iq:auth'><error id='400'/></query></iq>");
		}
		IoPutSendMsgCell(pDogMsg);
		return 1;
}	
int RespCatAllFilters(iks *node,PCLIENTINFO pClientInfo)
{
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		char Body[1024+128]={0},Filter[1024]={0};
		char *action=iks_find_attrib(node,"action");
		return_val_if_fail(pClientInfo,0);

		if(strcmp(action,"show")==0)
		{
				GetAllCuteDogsFilterString(Filter,sizeof(Filter));
				if(*Filter!='\0')
				{		
						snprintf(Body,sizeof(Body),"%s%s%s","<iq type='result'><query xmlns='filter:show' >",Filter,"</query></iq>");
						pDogMsg=IoGetSendMsgCell(pClientInfo);
						return_val_if_fail(pDogMsg,0);
						hRespStream=pDogMsg->hStream;
						push_stream_string(hRespStream,Body);
						IoPutSendMsgCell(pDogMsg);
				}

		}		

	return 0;
}
int RespCatSetHooker(iks *node,PCLIENTINFO pClientInfo)
{
		PDOG2CATIOMSG	pDogMsg;
		HSTREAM hRespStream;
		char Body[512]={0};
		char *action=iks_find_attrib(node,"action");
		char *Item=iks_find_cdata(node,"item");
		char *CuteCatName=iks_find_attrib(iks_find(node,"item"),"cat");

		Dbg(("******** RespCatSetHooker item=[%s] cutecatname=[%s]",Item,CuteCatName));
		if(iks_strcmp(action,"hook")==0)
		{
			if(AddHooker2CuteDog(Item,CuteCatName,pClientInfo))
			{
				pClientInfo->lType=CLIENT_USER_HOOKER;
				snprintf(Body,sizeof(Body),"%s","<iq type='result'><query xmlns='filter:hook' action='hook'><error id='OK'/></query></iq>");
			}
			else
			{
				snprintf(Body,sizeof(Body),"%s","<iq type='result'><query xmlns='filter:hook' action='hook'><error id='301'/></query></iq>");
			}
			pDogMsg=IoGetSendMsgCell(pClientInfo);
			return_val_if_fail(pDogMsg,0);
			hRespStream=pDogMsg->hStream;
			push_stream_string(hRespStream,Body);
			IoPutSendMsgCell(pDogMsg);
		}
		else if(iks_strcmp(action,"cancel")==0)
		{
			if(DelHookerInCuteDog(pClientInfo))
			{
				snprintf(Body,sizeof(Body),"%s","<iq type='result'><query xmlns='filter:hook' action='cancel'><error id='OK'/></query></iq>");
			}
			else
			{
				snprintf(Body,sizeof(Body),"%s","<iq type='result'><query xmlns='filter:hook' action='cancel'><error id='301'/></query></iq>");
			}
			pDogMsg=IoGetSendMsgCell(pClientInfo);
			return_val_if_fail(pDogMsg,0);
			hRespStream=pDogMsg->hStream;
			push_stream_string(hRespStream,Body);
			IoPutSendMsgCell(pDogMsg);
		}
		return 0;
}
int RespCatCreateFilter(iks *node,PCLIENTINFO pClientInfo)
{
		int iModel=0;
		char *action=iks_find_attrib(node,"action");
		char *DogName=iks_find_cdata(node,"username");
		char *FilterStr=iks_find_cdata(node,"filterstring");
		char *model=iks_find_attrib(node,"model");
		if(iks_strcmp(model,"lo")==0)
		{
				iModel=0;
		}
		else if(iks_strcmp(model,"net")==0)
		{
				iModel=1;
		}	
		if(iks_strcmp(action,"create")==0)
		{
			return_val_if_fail(FilterStr,0);
			if(StartDogNose(FilterStr,DogName,pClientInfo,iModel))
			{
					pClientInfo->lType=CLIENT_USER_PIONEER;
					CuteDogBroadCast();
			}
		}
		else if(iks_strcmp(action,"cancel")==0)
		{
			StopDogNose(pClientInfo);	
			DelCuteDogInList(pClientInfo);
			CuteDogBroadCast();
		}
		return 1;
}
void IOXMLIq(iks *node,PCLIENTINFO pClientInfo)
{
	//set and query key  --> value    maroc ???
	char *type=iks_find_attrib(node,"type");
	char *xmlns=iks_find_attrib(iks_find(node,"query"),"xmlns");
	iks	 *x=iks_find(node,"query");

	if(strcmp(type,"set")==0 && strcmp(xmlns,"jabber:iq:auth")==0) 	{RespCatLogin0(x,pClientInfo);return;}
	if(strcmp(type,"get")==0 && strcmp(xmlns,"filter:show")==0)   	{RespCatAllFilters(x,pClientInfo);return;}
	if(strcmp(type,"set")==0 && strcmp(xmlns,"filter:hook")==0)    	{RespCatSetHooker(x,pClientInfo);return;}
	if(strcmp(type,"set")==0 && strcmp(xmlns,"filter:new")==0)    	{RespCatCreateFilter(x,pClientInfo);return;}
	//if(strcmp(type,"set")==0 && strncmp(xmlns,"command",7)==0)   {CatLogin3(node,pClientInfo);return;}
	Dbg(("-----------------------------IOXMLIq type:%s xmlns %s",type,xmlns));
}
void IOProcessParser(PCLIENTINFO pClientInfo,HSTREAM hStream)
{
	const char *pCatMsg = GET_STREAM_PTR(hStream);
	return_if_fail(pClientInfo);
	return_if_fail(pClientInfo->Prs);
	switch (IKS_PARSE(pClientInfo,pCatMsg)) 
	{
		case IKS_OK:
			puts ("OK");
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
}

int IOProcessCatMsg(void *data,int type,iks *node)
{
	ikspak *pak = NULL;
	PCLIENTINFO pClientInfo=(PCLIENTINFO)(data);

	switch(type)
	{
		case IKS_NODE_START: 
			printf("***---node start\n");
			CatHandShake(node,pClientInfo);
			break;
		case IKS_NODE_NORMAL:
			//printf("***---normal\n");
			pak = iks_packet(node);
			switch (pak->type) 
			{
				case IKS_PAK_NONE:
					printf("JABBER: NONE\n");
					break;
				case IKS_PAK_MESSAGE:
					printf("JABBER: MESSAGE\n");
					break;
				case IKS_PAK_PRESENCE:			
					printf("JABBER:  presence!!\n");
					break;
				case IKS_PAK_S10N:
					printf("JABBER: subscribe!!\n");
					break;
				case IKS_PAK_IQ:
					printf("--------------------JABBER: IQ Mesage --------------!\n");
					IOXMLIq(node,pClientInfo);
					break;
				default:
					printf("JABBER: I Dont know %d\n", pak->type);
					break;
			}

			break;
		case IKS_NODE_ERROR:
			printf("***---error\n");
			break;
		case IKS_NODE_STOP:
			printf("***---stop\n");
			break;
	}
	return 0;
}		
int IOProcessAdminMsg(void *data,int type,iks *node)
{
	ikspak *pak = NULL;
	switch(type)
	{
		case IKS_NODE_START: 
			printf("***---node start\n");
			break;
		case IKS_NODE_NORMAL:
			printf("***---normal\n");
			pak = iks_packet(node);
			break;
	}
	return 0;
}		
