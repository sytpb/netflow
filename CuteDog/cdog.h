#ifndef DOG_INTERFACE_H
#define DOG_INTERFACE_H

typedef struct tagAimMsg
{
	char dev[8];		//lo eth0 ..
	char netip[32];		
	char filters[256];
}AimMsg,*pAimMsg;

int StartDogNose(AimMsg *paim);

int StopDogNose();

//int StartDogEar();
//int StopDogEar();

#endif
