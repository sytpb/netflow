#ifndef FIFO_H 
#define FIFO_H 

#include "typedef.h"

struct _FifoRing;
typedef struct _FifoRing FifoRing;
typedef struct _FifoRing* PFifoRing;


FifoRing	*FifoNew(size_t length);
Ret			FifoPush(FifoRing *thiz, void* data);
Ret			FifoPop(FifoRing *thiz, void** data);
void 		FifoDestroy(FifoRing* thiz);

#endif
