/*
<TAGS>string file</TAGS>

DESCRIPTION:

	Modify a line, parsing it into whitespace-delimited words, and modify an array of indices to each word

	The variable "nwords" will be modified to reflect the number of words on the line
	The "line" array will be modified, each white-space delimiter replaced by a NULL character

	NOTE:
		multiple whitespace is treated as a single whitespace, so "empty"
		words in a line will affect the index to all subsequent columns.
		Example: if the input file looks like this...
				a	b	c
				a		c

		... then word#2 will be "b" on line#1, but "c" on line#2

	NOTE:
		Leading white-space will insert a NULL at the beginning of the line,
		but start[0] will point to the first non-whitespace character on the line.
		This can be handy in space-delimited files where leading white-space signifies an empty field
		This can be detected using strlen(line), as opposed to strlen((line+start[0]))


	NOTE (4.November.2016)
		Text within quotes will be treated as a single word
		single-quotes within double-quotes (or vice-versa) will not trigger the start/end of quoted text
		Hence "don't exit" will be treated as a single word, the internal space and single-quote ignored
		- additional updates: new variable names, better use of memory


USES:
	provides a set of indices  to each column in a line
	advantage over strtok: finds all words in a line at once, COULD be modified to treat each delimiter as a separate word
	disadvantages over strtok: not as fast

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *line
		- input, pointer to a character array
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

	start= xf_lineparse1(line,&nwords);

	for(i=0;i<nwords;i++) printf("%s\n",line+start[i]);

	if(start!=NULL) free(start);
	exit(0);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long *xf_lineparse1(char *line,long *nwords) {


	char tempchar;
	int prevwhite,inquote1,inquote2;
	long ii,jj;
	long *start=NULL;
	size_t nchars;

	/* IF THIS IS A BLANK LINE (CARRIAGE RETURN ONLY), THERE ARE NO WORDS, SO RETURN NULL */
	nchars=strlen(line);
	if(nchars==0||line[0]=='\n') {*nwords=0;return(NULL);}

	/* reallocate memory for start array */

	*nwords=0;
	prevwhite=1;
	inquote1=inquote2=0;

	for(ii=0;ii<nchars;ii++) {

		/* store a temporary version of the current character */
		tempchar=line[ii];

		/* if this is not part of a currently open quote, test for the end of the word */
		if(inquote1==0 && inquote2==0) {
			/* if this is a white-space character, reset it to NULL */
			if(tempchar==' '||tempchar=='\t'||tempchar=='\n'||tempchar=='\r') {
				line[ii]='\0';
				prevwhite=1;
			}
			/* if this is NOT white-space and previous character was, then this is the start of a new word */
			else if(prevwhite==1) {
				start=(long *)realloc(start,(((*nwords)+1)*sizeof(long)));
				if(start==NULL)  {*nwords=-1;return(NULL);}
				start[*nwords]=ii ;
				*nwords=*nwords+1;
				prevwhite=0;
			}
		}

		/* determine if the current character falls within a quote (use tempchar, not potentially-modified line[ii]) */
		if(tempchar=='\'' && inquote2==0) {if(inquote1==0) inquote1=1; else inquote1=0;}
		if(tempchar=='"'  && inquote1==0) {if(inquote2==0) inquote2=1; else inquote2=0;}

	}

	/* return a pointer to the start array */
	return(start);
}
