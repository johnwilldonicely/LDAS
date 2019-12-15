/******************************************************************************
Read an ASCII header in a data file, up to a keyword (header_stop)
The keyword may denote the beginning of a binary portion of the file
Leaves file pointer at first byte after last byte of header
Returns number of bytes in header
??? modify in future to read x-number of bytes, as well
*******************************************************************************/
extern void hux_error(char message[]);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hux_getfileheader(
				FILE *fpin,			/* pointer to input file */ 
				char *header,		/* storage for header - make sure enough memory is allocated */
				char *header_stop,	/* null-terminated keyword signalling end of header */ 
				int headermax		/* maximum number of characters to read before bailing */
				)
{
	unsigned int headerlen=strlen(header_stop);
	int x=-1,y=0;
	while(fread(&header[++x],sizeof(char),1,fpin)!=0 && x < headermax) {
		if(header[x]==header_stop[y]) { /* on first iteration x and y will be zero */
			y++;			/* increment position to the next character to be compared in header_stop */
			if(y>=headerlen)break;	/* if the next character is z (length of header_stop) or greater, then the current character was the last one */
		} 
		else {
			y=0; 
			if(header[x]==header_stop[0]) y++;
			if(y>=headerlen) break; /* unlikely unless header_stop is only one character long! */ 
		}
	}
	if(y==0) {x=-1; header[0]='\0';} /* if keyword was not found, return error flag */
	else {x++; header[x]='\0';} 	/* counter-position for last character of header is zero-offset, so increment by one */
	return(x); /* return actual header size in bytes - note that fpin is now at first byte after last byte in header */
}
