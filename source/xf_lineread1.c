/*
<TAGS>string file</TAGS>

DESCRIPTION:

	A safer version of fgets to read a line of unknown length from an ASCII file
	For files with lines ~ 4000 characters wide, read time is about 80% the speed of fgets alone
	Uses fgets and concatenates the results to the variable "line"
	Characters are read in buffers of length XF_LINEREAD1_BUFFSIZE (typically 1000)
		- If buffer length is less than the length of the line being read, multiple calls to fgets are made
		- the function checks if the buffer contains a newline - if so further calls to fgets are halted

	Memory reserved for "line" is expanded as required
	Blank lines are correctly returned
	Lines without a terminating newline are correctly returned
	Input must be NULL-terminated

	Last update: 18 February 2013 [JRH]

USES:

	Situations where fgets is inappropriate because maximum line length is unknown
	Example - reading large tables or matrices of data

DEPENDENCY TREE:

	No dependencies

ARGUMENTS:

	char *line
		- pointer to a character array
		- new memory will be allocated and will be overwritten
	long *maxlinelen
		- variable holding the most recent value for maxmimum line length
		- updated with the actual length of the line, if this exceeds the initial value
		- the calling function should pass the address (&) of the variable, function will read as a pointer (*)
		- if maxlinelen is set to "-1", this indicates a memory allocation error occurred
	FILE *fpin
		- pointer to input stream (typically a file or stdin)
		- updated by the function

RETURN VALUE:

	A pointer to a character array holding the new line - NULL if no line was read

TEST PROGRAM:

		// save function as xf_lineread1.c
		// save as "xe-test.c" in same folder as xf_lineread1.c
		// compile as follows: gcc -O3 -std=gnu99 xe-test.c xf_lineread1.c -o xe-test)

		#include <stdio.h>
		#include <stdlib.h>

		char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);

		int main (int argc, char *argv[]) {
			char *line=NULL,*pline;
			long maxlinelen=0;
			FILE *fpin;
			if(argc<2) { fprintf(stderr,"USAGE: xe-test [filename] \n"); exit(0); }
			fpin=fopen(argv[1],"r");
			if(fpin==0) { fprintf(stderr,"File read error\n"); exit(1); }

			while((pline= xf_lineread1(line,&maxlinelen,fpin) )!=NULL) {
				if(maxlinelen<0) { fprintf(stderr,"Memory allocation error\n"); exit(1); }
				else printf("%s",pline);
			}
			fclose(fpin);
			fprintf(stderr,"\nlongest line: %d characters\n",maxlinelen);
			if(line!=NULL) free(line);
		}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define XF_LINEREAD1_BUFFSIZE 1000

char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin) {

	char buffer[XF_LINEREAD1_BUFFSIZE];
	size_t sizeofchar= sizeof(char);
	long nchars;

	/* IF NO MEMORY IS ALLOCATED FOR "line" DO IT NOW */
	if(line==NULL) {
		if(*maxlinelen<1) *maxlinelen=XF_LINEREAD1_BUFFSIZE;
		line= realloc(line,(*maxlinelen+1));
		if(line==NULL)  {*maxlinelen=-1; return(NULL);}
	}

	/* INITIALIZE LINE AND RESET NUMBER OF CHARACTERS */
	nchars=0; line[0]='\0';

	/* GET MULTIPLE BUFFFERS AND CONCATENATE THEM */
	while(fgets(buffer,XF_LINEREAD1_BUFFSIZE,fpin)!=NULL) {
		/* calculate how many characters were read, not including the terminating '\0' */
		nchars+=strlen(buffer);
		/* if more characters have been read that there is currently storage for, allocate more memory */
		/* note that nchars does not include the terminating null character which must be accommodated, so reallocation must also happen if nchars=maxlinelen */
		if(nchars>(*maxlinelen)) {
			*maxlinelen=nchars;
			line= realloc(line,(*maxlinelen+1));
			if(line==NULL) {*maxlinelen=-1; return(NULL);}
		}
		/* append current buffer to line */
		strcat(line,buffer);
		/* if the current buffer contains a newline, break */
		if(strstr(buffer,"\n")) break;
	}

	/* IF NOTHING WAS READ INTO THE BUFFER, RETURN A NULL, TERMINATING THE WHILE LOOP CALLING THIS FUNCTION  */
	if(nchars==0) {	line[nchars]='\0'; return(NULL) ; }
	/* OTHERWISE RETURN THE CURRENT LINE, MAKING SURE IT IS NULL-TERMINATED */
	else { line[nchars]='\0'; return(line);	}

}
