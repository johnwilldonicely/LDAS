/***********************************************************************
<TAGS>misc</TAGS>
DESCRIPTION:
	pause until a key is pressed
***********************************************************************/
#include <stdlib.h>
#include <stdio.h>

void xf_pause1(char message[]) {
	fprintf(stderr,"\n%s\n\n",message);
	getc(stdin);
	exit(0);
}
