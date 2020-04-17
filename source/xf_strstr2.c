/********************************************************************************
<TAGS>string</TAGS>

DESCRIPTION: 16 March 2020
	Find an exact word (needle) in a delimited line (haystack)

ARGUMENTS:
	char *haystack : string in which to find needle
	char *needle   : string to be found in haystack
	char delimiter : delimiter separating words in haystack
			- a single character, typically char delim='\t'

RETURN VALUE:
	The word-number in haystack matching needle (zero = first word)
	-1 if needle not found
	-2 if haystack is an empty string
	-3 if needle is an empty string

EXAMPLE: find "pig" at word 3 (counting from zero) in a CSV list...

	char haystack[]="dog,cat,pig,cow\0", needle[]="pig", delim=',';
	long word= xf_strstr2(haystack,needle,delim);
	if(word>=0) printf("found \"%s\" in \"%s\" at word %ld\n",needle,haystack,word);
	else if(word==-1) printf("Not found!\n");
	else if(word==-2) printf("Bad haystack!\n");
	else if(word==-3) printf("Bad needle!\n");

*********************************************************************************/
#include <stdlib.h>
#include <string.h>

long xf_strstr2(char *haystack, char *needle, char delim) {

	long ii=0,jj=0,len1=0,len2=0,word=0;

	if(haystack!=NULL) len1=strlen(haystack);
	else return -2; /* bad haystack */
	if(needle!=NULL) len2=strlen(needle);
	else return(-3); /* bad needle */

	for(ii=0;ii<len1;ii++) {
		/* for every delimiter found before a match, increment the word-counter */
		if(haystack[ii]==delim) {word++;jj=0;}
		if(haystack[ii]==needle[jj]) {
			jj++;
			/* make sure the match is preceded by a delimiter if this is the first matching character and we're not at the start of haystack */
			if(ii>0 && jj==1) { if(haystack[ii-1]!=delim) { jj=0; continue; } }
			/* if we've found all the characters in the needle sequence...  */
			if(jj==len2) { /* if full match is found... */
				/* if not at the end of the haystack, make sure the match is followed by a delimiter */
				if(ii<(len1-1)) { if(haystack[(ii+1)]!=delim) { jj=0; continue; } }
				/* otherwise, stop the search */
				break;
	}}}
	/* if the full sequence was not found, reset word */
	if(jj!=len2) word=-1;

	return(word);
}
