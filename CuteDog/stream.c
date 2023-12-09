#include <stdlib.h>
#include <stdio.h>
#include "stream.h"
#include <string.h>

#define MIN_MALLOC_LEN	32

typedef struct tagStream
{
		unsigned long size;
		unsigned char *chunk_ptr;
		unsigned char *chunk_buf;
		unsigned char *chunk_mem;
		unsigned char *chunk_read;
		unsigned char *chunk_write;
		unsigned long syncword;
		unsigned long endword;
}cstream;

HSTREAM init_stream(unsigned long size)
{
		cstream *stream = (cstream*)malloc(sizeof(cstream));
		stream->size = size;
		stream->chunk_buf = stream->chunk_mem = (unsigned char *)malloc(size+MIN_MALLOC_LEN);
		if(stream->chunk_buf == 0)
		{
				free(stream);
				return NULL;
		}
		memset(stream->chunk_mem,0,size + MIN_MALLOC_LEN);
		while ((unsigned long) stream->chunk_buf % 16)
				stream->chunk_buf++;
		stream->chunk_ptr = stream->chunk_buf;
		stream->chunk_read = stream->chunk_write = stream->chunk_buf;
		stream->endword = stream->syncword = 0;
		return (HSTREAM)stream;
}

unsigned char *copy_stream (unsigned char * current, unsigned char * end,HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		unsigned char * limit;
		unsigned int len_t;
		limit = current + (stream->chunk_buf + stream->size - stream->chunk_ptr);
		if (limit > end)
				limit = end;
		len_t=limit-current;
		memcpy(stream->chunk_ptr,current,len_t);
		current+=len_t;
		stream->chunk_ptr+=len_t;
		return current;			//one by one copy safe ? 
}
unsigned char* get_data_ptr(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		return stream->chunk_buf;
}
void del_stream(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		if(stream->chunk_mem)
				free(stream->chunk_mem);
		free(stream);
}
void reset_stream(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		stream->chunk_ptr = stream->chunk_buf;
		stream->chunk_read = stream->chunk_write = stream->chunk_buf;
		stream->endword = stream->syncword = 0;
}
void push_stream_buf(HSTREAM hstream,unsigned char *buf,int len)
{
		cstream *stream = (cstream*)hstream;
		unsigned int ret = 0;
		unsigned int wrotelen;
		if(len <= 0)
				return;
		if(!(ret = check_write_free(hstream,len,&wrotelen)))
				if(!realloc_stream_size(hstream,wrotelen,len))	return;
		memcpy(stream->chunk_write,buf,len);
		stream->chunk_write += len;
}

void push_stream_string(HSTREAM hstream,char *str)
{
		cstream *stream = (cstream*)hstream;
		int len = strlen(str);
		unsigned int ret = 0;
		unsigned int wrotelen;

		if(!(ret = check_write_free(hstream,len,&wrotelen)))
				if(!realloc_stream_size(hstream,wrotelen,len))	return;

		memcpy(stream->chunk_write,str,len);				//not safe ??  snprintf ?
		stream->chunk_write[len] = '\0';
		stream->chunk_write += (len+1);
}

unsigned int check_write_free(HSTREAM hstream,unsigned int push_len,unsigned int *wrotelen)
{
		cstream *stream = (cstream*)hstream;
		unsigned int _wrotelen = stream->chunk_write - stream->chunk_buf;	//include STREAM_HEAD_LEN
		unsigned int len_t = _wrotelen + push_len;
		*wrotelen = _wrotelen;
		if( len_t > stream->size)
				return 0;
		return 1;
}

int realloc_stream_size(HSTREAM hstream,unsigned int wrotesize,unsigned int writtingsize)
{
		cstream *stream = (cstream*)hstream;
		unsigned int len_t=stream->size+writtingsize;
		stream->chunk_mem = (unsigned char*)realloc(stream->chunk_mem,len_t+(len_t>>2));   //*1.5
		if(!stream->chunk_mem)
		{
			return 0;
		}
		stream->size = len_t+(len_t>>2);	
		stream->chunk_buf = stream->chunk_mem;
		stream->chunk_ptr = stream->chunk_buf + wrotesize;
		stream->chunk_read = stream->chunk_write = stream->chunk_buf + wrotesize;
		return 1;
}
unsigned char* get_stream_ptr(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		return stream->chunk_buf;
}
unsigned long get_xmlstream_len(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		unsigned long len = strlen((char*)stream->chunk_buf);
		return len+1;		//include '\0'
}
unsigned long get_stream_len(HSTREAM hstream)
{
		cstream *stream = (cstream*)hstream;
		unsigned long len = strlen((char*)stream->chunk_buf);
		return len;		
}
