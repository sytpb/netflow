#include <stdio.h>
#include <stdlib.h>
#include "cdog.h"
//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}
int StartEar(void);
int StartNose(void*);

int Start()
{
#ifdef SER_DOG
	Dbg(("You Started a CDOG SERVER !!"));	
	StartEar();
#else
	Dbg(("you started a perfessioal cdog  !!"));	
	StartDogNose(NULL);
#endif
	return 0;
}
int main()
{
		Start();
		return 0;
}
