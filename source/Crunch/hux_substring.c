/********************************************************************************
Test if source string includes target string
Returns integer (= first element of source string which matches the target string)
Returns 0 if match is at first character
Returns -1 if target is not found

Modified Feb.10 2010: removed reference to external function hux_error : direct error handling instead

*********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int hux_substring(
				  char *source,		/* character array to be searched */
				  char *target,		/* character array to find in source */
				  int casesensitive	/* should match be case sensitive? (0=no, 1=yes) */
				  )
{
	unsigned int lensource=strlen(source), lentarget=strlen(target);
	int i,matchtot=0,start=-1;
	
	if(casesensitive!=0&&casesensitive!=1) {fprintf(stderr,"Error[hux_substring]: argument \"casesensitive\" should be 0 or 1\n"); exit(1);}
	if(lensource<1) {fprintf(stderr,"Error[hux_substring]: source string is empty\n"); exit(1);}
	if(lentarget<1) {fprintf(stderr,"Error[hux_substring]: target string is empty\n"); exit(1);}

	else {
		for(i=0;i<lensource;i++) {
			/* does current element of source match expected element of target? */
			if(source[i]==target[matchtot] || (casesensitive==0&&tolower(source[i])==tolower(target[matchtot]))) {
				matchtot++;
				if(matchtot==lentarget) {start=i-lentarget+1; return(start);}
			}
			/* if not, perhaps the current element matches the start of target? */
			else if (source[i]==target[0] || (casesensitive==0&&tolower(source[i])==tolower(target[0]))) {
				matchtot=1;
				if(matchtot==lentarget) {start=i-lentarget+1; return(start);}
			}
			/* otherwise, reset the number of matched elements to zero */
			else matchtot=0;
		}
	}
	return(start); /* return element number of source array at which first instance of target was first found */
}
