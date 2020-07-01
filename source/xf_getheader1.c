/*
<TAGS>file database</TAGS>
DESCRIPTION:
	Extract an ASCII header from an ASCII or binary file
	- assumes the header ends in a keyword
	- stored header will include the keyword

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile  : the name of the input file (or "stdin")
	char *keyword : the keyword markning the end of the header
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	on sucess:  A pointer to the header - must be freed by calling function or main
	on failure: NULL
	char array will hold message (if any)

SAMPLE CALL:
	char *header=NULL,message[256];
	header= xf_getheader1(data.wfm,"WAVES_START",message );
	ifheader!=NULL) printf("%s\n",header);
	else { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xf_getheader1(char *infile, char *keyword, char *message) {

	char *thisfunc="xf_getheader1\0";
	char *header=NULL,buff;
	long ii,jj,kk,nn,keylen,matchtot,found;
	FILE *fpin;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(infile==NULL) { sprintf(message,"%s [ERROR]: infile is undefined",thisfunc); return(NULL); }

	/* INITIALIZE THE VARIABLES */
	keylen= strlen(keyword);
	nn= matchtot= found= 0;

	/* OPEN THE BINARY FILE */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else fpin=fopen(infile,"rb");
	if(fpin==NULL) { sprintf(message,"%s: file \"%s\" not found",thisfunc,infile); return(NULL);}

	/* SCAN THE FILE */
	while(1) {
		/* dynamically add memory */
		header= realloc(header,(nn+1)*sizeof(char));
		/* read one byte (character) of input */
		kk= fread(&buff,1,1,fpin);
		if(kk!=1) { header[nn]='\0'; break; }
		/* store the character in the header string and update the character-count */
		header[nn++]=buff;
		/* check if the character matches the current expected value of the stop-string */
		if(buff==keyword[matchtot]) {
			matchtot++;
			if(matchtot==keylen) { found=1; break; }
		}
		/* otherwise reset matchtot */
		else matchtot=0;
	}

	/* MAKE SURE THE HEADER WAS ACTUALLY FOUND */
	if(found==0) { sprintf(message,"%s: header keyword \"%s\" not found in \"%s\"",thisfunc,keyword,infile); return(NULL);}

	/* RETURN POINTER TO THE HEADER */
	return (header);
}
