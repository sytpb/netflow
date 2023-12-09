
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "print.h"

#ifdef __cplusplus
extern "C" {
#endif 

void
find_and_printf_sn(char *buf,unsigned int len)
{
	const char *ptr,*limit,*head,*tmp;
	char sn[16]={0};
	
	if(len>0)
	{
		*(buf+len)='\0';
		head=buf;
	}

	limit=head+(len-1);
	while(head<limit)
	{
		while((ptr=strstr(head,"sn=")) != NULL)
		{
			sscanf(ptr,"sn='%[^']",sn);
			tmp=strchr(ptr,'\'');
			if(tmp == NULL)
				tmp=ptr+3;			
			head=tmp;
			printf("\t sn=[%s]",sn);
		}
		head++;
	}
}

void
hex_and_ascii_print_with_offset(register const char *ident,
    register const u_char *cp, register u_int length, register u_int oset)
{
	register u_int i;
	register int s1, s2;
	register int nshorts;
	char hexstuff[HEXDUMP_SHORTS_PER_LINE*HEXDUMP_HEXSTUFF_PER_SHORT+1], *hsp;
	char asciistuff[ASCII_LINELENGTH+1], *asp;

	nshorts = length / sizeof(u_short);
	i = 0;
	hsp = hexstuff; asp = asciistuff;
	while (--nshorts >= 0) {
		s1 = *cp++;
		s2 = *cp++;
		(void)snprintf(hsp, sizeof(hexstuff) - (hsp - hexstuff),
		    " %02x%02x", s1, s2);
		hsp += HEXDUMP_HEXSTUFF_PER_SHORT;
		*(asp++) = (isgraph(s1) ? s1 : '.');
		*(asp++) = (isgraph(s2) ? s2 : '.');
		i++;
		if (i >= HEXDUMP_SHORTS_PER_LINE) {
			*hsp = *asp = '\0';
			(void)printf("%s0x%04x: %-*s  %s",
			    ident, oset, HEXDUMP_HEXSTUFF_PER_LINE,
			    hexstuff, asciistuff);
			i = 0; hsp = hexstuff; asp = asciistuff;
			oset += HEXDUMP_BYTES_PER_LINE;
		}
	}
	if (length & 1) {
		s1 = *cp++;
		(void)snprintf(hsp, sizeof(hexstuff) - (hsp - hexstuff),
		    " %02x", s1);
		hsp += 3;
		*(asp++) = (isgraph(s1) ? s1 : '.');
		++i;
	}
	if (i > 0) {
		*hsp = *asp = '\0';
		(void)printf("%s0x%04x: %-*s  %s",
		     ident, oset, HEXDUMP_HEXSTUFF_PER_LINE,
		     hexstuff, asciistuff);
	}
}

void
hex_and_ascii_print(register const char *ident, register const u_char *cp,
    register u_int length)
{
	hex_and_ascii_print_with_offset(ident, cp, length, 0);
}

/*
 * telnet_print() wants this.  It is essentially default_print_unaligned()
 */
void
hex_print_with_offset(register const char *ident, register const u_char *cp, register u_int length,
		      register u_int oset)
{
	register u_int i, s;
	register int nshorts;

	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0) {
			(void)printf("%s0x%04x: ", ident, oset);
			oset += HEXDUMP_BYTES_PER_LINE;
		}
		s = *cp++;
		(void)printf(" %02x%02x", s, *cp++);
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("%s0x%04x: ", ident, oset);
		(void)printf(" %02x", *cp);
	}
}

#ifdef __cplusplus
}
#endif 