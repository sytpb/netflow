

#ifndef __HEAD__PRINT
#define __HEAD__PRINT

#define ASCII_LINELENGTH 300
#define HEXDUMP_BYTES_PER_LINE 16
#define HEXDUMP_SHORTS_PER_LINE (HEXDUMP_BYTES_PER_LINE / 2)
#define HEXDUMP_HEXSTUFF_PER_SHORT 5 /* 4 hex digits and a space */
#define HEXDUMP_HEXSTUFF_PER_LINE \
		(HEXDUMP_HEXSTUFF_PER_SHORT * HEXDUMP_SHORTS_PER_LINE)

#define PRINT_CLOSE		0x00
#define PRINT_TCPDUMP	0x01
#define PRINT_RAW		0x02

#define PRT_RAW(a) \
    if(DebugFlag & PRINT_RAW)\
		{printf("\n\t");printf a;}

#define PRT_RAWSN(a,b) \
    if(DebugFlag & PRINT_RAW)\
		{find_and_printf_sn(a,b);}

#define PRT_PARSESN(a) \
    if(DebugFlag & PRINT_RAW)\
		{printf("\n\t");printf a;}

#define PRT_OK(a,b,c,d)\
	if(DebugFlag & PRINT_TCPDUMP)\
		{hex_and_ascii_print(a,b,c);printf("\n\t¡ø--------------------[%s:%d]-----------------------¡ø",d,c);}

#ifdef _WIN32
	typedef unsigned char	u_char;
	typedef unsigned int	u_int;
	typedef unsigned short	u_short;
	#define snprintf _snprintf
#endif

#ifdef __cplusplus
extern "C" {
#endif 
int		DebugFlag;
void find_and_printf_sn(char *ptr,unsigned int len);
void hex_and_ascii_print(register const char *ident, register const u_char *cp,register u_int length);
void hex_print_with_offset(register const char *ident, register const u_char *cp, register u_int length,register u_int oset);

#ifdef __cplusplus
}
#endif 

#endif /*__HEAD__PRINT*/