/********************************************************************************
<TAGS>string</TAGS>

DESCRIPTION:
	- concatenate string2 to string1, if it is not already in string1
	- this is similar to xf_strcat1, except:
		- requires string2 to be unique
		- delimiter must be a single character

	CAUTION: this function may not be safe for repeated calls

ARGUMENTS:
	char *string1   : first input string - the one to be lengthened - can be NULL
	char *string2   : second input string - to be added to string1
	char delimiter  : single-character delimiter to put between them
	long *wmatch	: the word (from zero) in string1 matching string2
				- pass address to variable - will be updated

DEPENDENCIES:
	xf_strstr2

RETURN VALUE:
	Success:
		- pointer to the lengthened string1, or NULL on failure
		- wmatch is updated with the new-word-number (from zero)
		- wmatch will be -1 if the word was not added (because it was found)

	Fail: NULL
*********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long xf_strstr2(char *haystack, char *needle, char delim);

char *xf_strcat2(char *string1,char *string2,char delimiter,long *wmatch, char*message) {

	char *thisfunc="xf_strcat2\0";
	char delim2[2];
	long ii,jj,tempmatch=-1;
	size_t p1,s1,s2;

	/* delimiter must be defined */
	if(delimiter=='\0') {sprintf(message,"%s [ERROR]: delimiter undefined",thisfunc); return(NULL);}
	/* string to be appended must be defined */
	if(string2!=NULL) s2=strlen(string2);
	else {sprintf(message,"%s [ERROR]: string2 undefined",thisfunc); return(NULL);}

	/* if string1 is empty, start by just copying from string2 with no delimiter */
	if(string1==NULL) {
		string1= realloc( string1, (s2*sizeof(*string1)) );
		strcpy(string1,string2);
		tempmatch=0;
	}
	/* otherwise, check for uniqueness of string2 and if true, append */
	else {
		sprintf(delim2,"%c",delimiter);
		ii= xf_strstr2(string1,string2,delimiter);
		//TEST: printf("%s --- %ld --- %s\n",string2,ii,string1);
		if(ii==-1) {
			s1= strlen(string1);
			tempmatch=1;
			for(p1=0;p1<s1;p1++) if(string1[p1]==delimiter) tempmatch++;
			string1= realloc(string1,(s1+1+s2)*sizeof(char));
			if(string1==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL);}
			strcat(string1,delim2);
			strcat(string1,string2);
		}
		else if(ii<-1) {sprintf(message,"%s [ERROR]: bad input to xf_strstr2",thisfunc); return(NULL);}
	}

	/* update tempmatch - the word in string1 that string2 matched */
	(*wmatch)= tempmatch;
	/* return a pointer to the new string1 */
	return(string1);
}
