/********************************************************************************
<TAGS>string</TAGS>

DESCRIPTION:
	- safe concatenation of two input strings, using a delimiter of choice
	- allocates sufficient memory for the expanded string before calling strcat

ARGUMENTS:
	char *string1   : first input string - the one to be lengthened
	char *string2   : second input string - to be added to string1, must be defined
	char *delimiter : the delimiter to put between them, must be defined

RETURN VALUE:
	Pointer to the lengthened string1, or NULL on failure

*********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xf_strcat1(char *string1,char *string2,char *delimiter) {

	size_t s1,s2,s3,newsize;

	/* delimiter must be defined */
	if(delimiter!=NULL) s3=strlen(delimiter);
	else return(NULL);

	/* string to be appended must be defined */
	if(string2!=NULL) s2=strlen(string2);
	else return(NULL);

	/* if string1 is not empty, use strcat to append delimiter and string2 */
	if(string1!=NULL) {
		s1=strlen(string1);
		newsize= s1+s2+s3+1;
		string1=(char *)realloc(string1,newsize);
		if(string1==NULL) return(NULL);
		strcat(string1,delimiter);
		strcat(string1,string2);
	}
	/* otherwise if string1 is empty, start by just copying from string2 with no delimiter */
	else {
		newsize=s2+1;
		string1=(char *)realloc(string1,newsize);
		if(string1==NULL) return(NULL);
		strcpy(string1,string2);
	}

	/* return a pointer to the new string1 */
	return(string1);
}
