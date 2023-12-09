#ifndef		__STREAMLIB_H_
#define		__STREAMLIB_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef		int *		HSTREAM;
typedef		int **		PHSTREAM;


HSTREAM init_stream(unsigned long size);
void 	del_stream(HSTREAM stream);
void 	reset_stream(HSTREAM stream);
void 	push_stream_buf(HSTREAM hstream,unsigned char *buf,int len);
void 	push_stream_string(HSTREAM hstream,char *str);

unsigned char *copy_stream (unsigned char * current, unsigned char * end,HSTREAM hstream);
unsigned char *get_data_ptr(HSTREAM hstream);

unsigned int 	check_write_free(HSTREAM hstream,unsigned int push_len,unsigned int *wrotelen);
int 			realloc_stream_size(HSTREAM hstream,unsigned int wrotesize,unsigned int writtingsize);

unsigned char* get_stream_ptr(HSTREAM hstream);
unsigned long get_xmlstream_len(HSTREAM hstream);
unsigned long get_stream_len(HSTREAM hstream);

#ifdef __cplusplus
}
#endif

#endif

