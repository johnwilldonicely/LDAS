/*
<TAGS>string</TAGS>

DESCRIPTION:
	Return the comment-status of an input line

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *line : input, pointer to a character array
	int &skip  : current status of skip
			: initialize to zero for first time the function is called
			: allows commenting to be carried over from previous lines contiaining a quote or /*
RETURN VALUE:
	Always zero

TEST PROGRAM:

	skip=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		xf_linecomment1(line,&skip);
		printf("%s",line);
	}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_linecomment1(char *line, int *skip1) {

	int skip=(*skip1);
	char prev=' ';
	size_t ii,nn;

 	nn= strlen(line);

	for(ii=0;ii<nn;ii++) {
		/* handle single-quotes - may signal start or end of quote */
		if(line[ii]=='\'' && prev!='\\' && (skip==0 || skip==1)) { if(skip==1) skip=0; else {skip=1;continue;} }
		/* handle double-quotes - may signal start or end of quote  */
		if(line[ii]=='\"' && prev!='\\' && (skip==0 || skip==2)) { if(skip==2) skip=0; else {skip=2;continue;} }
  		/* handle hash-symbols */
		else if(line[ii]=='#' && skip==0) { skip=3; }
		/* handle C++ style double-slash */
  		else if(ii>0 && line[ii]=='/' && prev=='/' && skip==0) { skip=4;}
		/* handle C-style slash-asterix comments */
		else if(ii>0 && line[ii]=='*' && prev=='/' && skip==0) { skip=5;}
  		else if(ii>0 && line[ii]=='/' && prev=='*' && skip==5) { skip=0; line[ii-1]='*'; }

		/* handle newline - will terminate comments of types 3 & 4 (hash or double-slash) */
		/* otherwise, store as previous character */
  		else if(line[ii]=='\n') { if(skip==3 || skip==4) skip=0; }
		else {
			prev=line[ii];
			/* and if this text is commented or quoted, change to an underscore */
			if(skip>0) line[ii]='_';
		}
	}

	(*skip1)=skip;
	return(0);
}
