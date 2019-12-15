/*
<TAGS>string</TAGS>

DESCRIPTION:
	Find the portion of string before or after a delimiter
	User determines whether the first or last delimiter is used
	User determines whether the preceeding or following portion of the input string is returned

USES:
	Find the extention of a filename, or the directory specified in a full-path filename

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *input : pointer to a character array
	char delimiter : delimiter which separates "input" into "pre" and "post" portions
	int firstlast : consider only the first (1) or last (2) delimiter in "input"
	int prepost : return the portion of "input" preceding (1) or following (2) the delimiter

RETURN VALUE:
	A pointer to a new string which is no longer than the input string.

	NULL is returned if the input string is empty or the delimiter is not found
	and "prepost" is set to "2" (i.e. user requested all text following the
	delimiter).

	An exact copy of "input" is returned if the delimiter is not found and
	"prepost" is set to "1" (i.e. user requested all text preceding the
	delimiter)

SAMPLE CALL:
	char *extension = xf_strcut1( filename, '.', 2,2);
--------------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <string.h>

char *xf_strcut1(char *input, char delimiter, int firstlast, int prepost) {

	size_t len1,i,j;
	char *output=NULL;

	/* DETERMINE LENGTH OF INPUT */
	len1=strlen(input); if(len1<1) return(NULL);

	/* ALLOCATE MEMORY */
	output=(char *)realloc(output,((len1+1)*sizeof(char))); if(output==NULL) return(NULL);

	/* FIND POSITION OF DELIMITER */
	j=len1;
	/* if we are meant to find the first... */
	if(firstlast==1) for(i=0;i<len1;i++) if(input[i]==delimiter) {j=i;break;}
	/* if we are meant to find the last... */
	if(firstlast==2) for(i=0;i<len1;i++) if(input[i]==delimiter) j=i;

	/* GENERATE THE OUTPUT STRING */
	if(prepost==1) {
		for(i=0;i<j;i++) output[i]=input[i];
		output[j]='\0';
	}
	if(prepost==2) {
		for(i=0,j=(j+1);j<len1;j++) output[i++]=input[j];
		output[i]='\0';
	}

	return(output);
}
