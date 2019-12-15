/********************************************************************************
<TAGS>string</TAGS>

DESCRIPTION:
	Find first or last occurrence of characters ("needle") in a longer string ("haystack")

ARGUMENTS:
	char *haystack : pointer to a character array to be searched
	char *needle   : the string of characters to be found in haystack
	int setcase    : should the search be case sensitive (1) or not (0)
	int firstlast  : find the first (1) or last (2) "needle" in "haystack"

RETURN VALUE:
	Position of needle in haystack: 0= first character
	-1 if needle not found
	-2 if haystack is an empty string
	-3 if needle is an empty string
	-4 if an invalid value for "setcase" was used
	-5 if an invalid value for "firstlast" was used

Modified Feb.10 2010: removed reference to external function hux_error : direct error handling instead
Modified August 9 2011: remove text reporting, rename to xf_, add error return values
Modified May 21 2016: return long, not int

*********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

long xf_strstr1(char *haystack,char *needle,int setcase, int firstlast) {

	long len1=strlen(haystack), len2=strlen(needle);
	long ii,matchtot=0,start=-1;

	if(len1<1) return -2; /* invalid length for haystack */
	if(len2<1) return(-3); /* invalid length for needle */
	if(setcase!=0&&setcase!=1) return(-4);
	if(firstlast!=1&&firstlast!=2) return(-5);

	for(ii=0;ii<len1;ii++) {
		/* does current element of haystack match expected element of needle? */
		if(haystack[ii]==needle[matchtot] || (setcase==0&&tolower(haystack[ii])==tolower(needle[matchtot]))) {
			matchtot++;
			if(matchtot==len2) {
				start=ii-len2+1;
				if(firstlast==1) return(start);
				else matchtot=0;
			}
		}
		/* if not, perhaps the current element matches the start of needle? */
		else if (haystack[ii]==needle[0] || (setcase==0&&tolower(haystack[ii])==tolower(needle[0]))) {
			matchtot=1;
			if(matchtot==len2) {
				start=ii-len2+1;
				if(firstlast==1) return(start);
				else matchtot=0;
			}
		}
		/* otherwise, reset the number of matched elements to zero */
		else matchtot=0;
	}


	/* return element number of haystack array at which first instance of needle was first found */
	return(start);
}
