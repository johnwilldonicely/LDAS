/*
<TAGS>string</TAGS>

DESCRIPTION:
	String substitution function
	Replaces str1 with str2 in source

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *source : pointer to the input string
	char *str1 : substring to be replaced
	char *str2 : the replacement substring - if "", str1 will simply be removed

RETURN VALUE:
	Returns a pointer to a new string of characters

SAMPLE CALL:


*/
/********************************************************************************
*********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* xf_strsub1 (char *source, char *str1, char *str2) {

	char *result=NULL;
	long ii,jj,kk,lensource,lenresult=0,lenstr1=0,lenstr2=0;
	int sizeofchar=sizeof(char);

	lensource= strlen(source),
	lenstr1= strlen(str1);
	lenstr2= strlen(str2);
	//TEST: fprintf(stderr,"source:%s	str1:%s	str2:%s\n",source,str1,str2);

	/* return a pointer to the original if there is nothing to be replaced or if the original string was empty */
	if(lensource<1 || lenstr1<1) return(source);

	for(ii=0;ii<lensource;ii++) { // ii is the counter for each position in the source string

		// scan forward for the length of the target string looking for a match
		jj=0; while(jj<lenstr1 && (ii+jj)<lensource && source[ii+jj]==str1[jj]) jj++;

		/* if a match was not found just copy the curent character to the result */
		if(jj<lenstr1) {
			result= realloc(result,((lenresult+1+1)*sizeofchar));
			result[lenresult]= source[ii];
			lenresult++;
			continue;
		}

		/* if a match was found, insert str2 into result and increment ii by the length of str1 */
		else {
			result= realloc(result,((lenresult+lenstr2+1)*sizeofchar));
			for(kk=0;kk<lenstr2;kk++) result[(lenresult+kk)]=str2[kk];
			lenresult+= lenstr2;
			ii+= (lenstr1-1);
			continue;
		}
	}

	/* add the terminating NULL character */
	result[lenresult]='\0';

	return(result);
}
