/*
<TAGS>string file</TAGS>

DESCRIPTION:

	Modify a line, parsing it into delimited words, and modify an array of indices to each word
	Behaves much like the linux "cut" command
		- unlike strtok, serial delimiters are not treated as a single delimiter
		- hence two consecutive delimiters, or delimiters at the beginning or end of the line, signify a missing word
		- note: using strtok to identify words in a line is faster, but strtok is not appropriate for CSV files

USES:
	provides a set of indices to each column in a line
	suitable for reading particular columns from CSV files or other ASCII files where "blank" fields are possible


DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *line
		- input, pointer to a character array
	char *delimiters
		- input, delimiter characters to consider
	long *nwords
		- address to a long integer to hold the total number of words found

RETURN VALUE:

	Success: A pointer to a long-integer array holding the start locations of each word in line
		- memory is allocated for this array inside the function
		- the variable nwords will be updated to report the number of delimited words on the line
	Blank line: NULL, nwords=0;
	Memory allocation fail: NULL, nwords=-1

TEST PROGRAM:

	#include <stdio.h>
	#include <stdlib.h>
	long *start=NULL,nwords,i;
	char line[13]={"dog cat pig"};

	start= xf_lineparse2(line,"\t ",&nwords);

	for(i=0;i<nwords;i++) printf("%s\n",line+start[i]);
	if(start!=NULL) free(start);
	exit(0);

???TO DO: may be able to improve using search method more like lineparse1 (simple serial scan)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long *xf_lineparse2(char *line,char *delimiters, long *nwords) {

	char *pch;
	long *start=NULL,ii;
	size_t nchars;

	/* IF THIS IS A BLANK LINE (CARRIAGE RETURN ONLY), THERE ARE NO WORDS, SO RETURN NULL */
	nchars=strlen(line);
	if(nchars==0||line[0]=='\n') {*nwords=0;return(NULL);}

	/* REALLOCATE MEMORY FOR START ARRAY */
	start= realloc(start,(nchars*sizeof(*start)));
	if(start==NULL)  {*nwords=-1;return(NULL);}
	start[0]=-1;

	/* FIND THE LOCATIONS OF THE DELIMITERS */
	pch = strpbrk(line,delimiters);
	(*nwords) = 1;
	while(pch != NULL) {
		ii=pch-line;
		start[(*nwords)++]=ii;
		pch= strpbrk(pch+1,delimiters);
	}

	/* RESET DELIMITERS TO '\0' */
	for(ii=1;ii<(*nwords);ii++) line[start[ii]]='\0';

	/* SET START POSITIONS FOR EACH WORD TO ONE PAST THE DELIMITER */
	for(ii=0;ii<(*nwords);ii++) start[ii]+=1;

	/* IF THE LAST WORD CONTAINS A NEWLINE, REPLACE IT WITH A NULL */
	ii=start[(*nwords)-1];
	while(line[ii]!='\0') {
		if(line[ii]=='\n' || line[ii]=='\r') line[ii]='\0';
		ii++;
	}

	/* ??? INVALIDATE ALL THE OTHER START SIGNALS - PROBABLY UNNECESSARY!*/
	for(ii=(*nwords);ii<nchars;ii++) start[ii]=-1;

	/* return the list of start times */
	return(start);
}
