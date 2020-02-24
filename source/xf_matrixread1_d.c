/*
<TAGS>file dt.matrix</TAGS>

DESCRIPTION:
		Read one or more matrices from an input
		Each matrix should be separated by a blank line or a line whos first character is "#"
		Data is stored as a continuous array of double-precision floating point numbers
		Automatically detects the number of rows and columns in the input
			- the first matrix is taken as the model - discrepencies result in errors
		The number of matrices and the number of rows and columns are stored

USES:
	storing 2-d blocks of ascii data into memory as a 1d array

DEPENDENCY TREE:

	No dependencies

ARGUMENTS:
	long *nmatrices - output variable, how many matrices were read
	long *ncols     - output variable, how many columns are on each row of each matrix
	long *nrows     - output variable, how many rows are in each matrix
	char *message   - holds error messages - should be defined by caliing funtion as message[256]
	FILE *fpin      - pointer to input stream (typically a file or stdin) - updated by the function

RETURN VALUE:
	- A pointer to an array holding the matrix, or multiple matrices packed together
	- NULL if no line was read
	- note that nmatrices, ncols and nrows will also be updated

SAMPLE CALL:
	data1= xf_matrixread1_d(&nmatrices1,&ncols1,&nrows1,message,fpin);
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* internal functions start */
char *xfinternal_lineread1(char *line,long *maxlinelen,FILE *fpin);
long *xfinternal_lineparse1(char *line,long *nwords);
/* internal functions end */

double *xf_matrixread1_d(long *nmatrices, long *ncols, long *nrows, char *message, FILE *fpin) {

	char *thisfunc= "xf_matrixread1_d\0";
	char *line=NULL;
	int prevblank=1;
	long i,j,k,n=0,nwords,nrowstemp=0,nlines=0,maxlinelen=0;
	long *start=NULL;
	double aa,*matrix=NULL;
	size_t sizeofdouble=sizeof(double);

	*ncols=0; *nrows=0; *nmatrices=0;

	while((line=xfinternal_lineread1(line,&maxlinelen,fpin))!=NULL) {

		nlines++;
		if(maxlinelen==-1)  {matrix=NULL; sprintf(message," %s: memory allocation error reading line %ld",thisfunc,nlines); goto FINISH;}

		start= xfinternal_lineparse1(line,&nwords);

		if(line[0]!='#' && nwords>0) {

			matrix=(double *)realloc(matrix,(n+nwords)*sizeofdouble);
			if(matrix==NULL) {matrix=NULL; sprintf(message,"%s: memory allocation error storing matrix",thisfunc); goto FINISH;}
			for(i=0,j=n;i<nwords;i++) {
				if(sscanf((line+start[i]),"%lf",&aa)==1 && isfinite(aa)) matrix[j]=aa;
				else matrix[j]=NAN;
				j++;
			}
			if(nrowstemp==0 && *nmatrices==0) *ncols=nwords;
			else if(nwords!=*ncols) {matrix=NULL; sprintf(message,"%s: unequal number of columns detected at line %ld",thisfunc,nlines); goto FINISH;}
			n+=nwords; prevblank=0; nrowstemp++;
		}

		else {
			if(prevblank==0) {
				if(*nmatrices==0) *nrows=nrowstemp;
				else if(nrowstemp!=(*nrows)) {matrix=NULL; sprintf(message,"%s: unequal number of rows detected at line %ld",thisfunc,nlines); goto FINISH;}
				*nmatrices=*nmatrices+1;
			}
			prevblank=1; nrowstemp=0;
	}}


	if(prevblank==0) {
		if(*nmatrices==0) *nrows=nrowstemp;
		else if(nrowstemp!=(*nrows)) {matrix=NULL; sprintf(message,"%s: unequal number of rows detected at line %ld",thisfunc,nlines); goto FINISH;}
		*nmatrices=*nmatrices+1;
	}

	if(*nmatrices<1){matrix=NULL; sprintf(message,"%s: no matrices in file",thisfunc); goto FINISH;}

FINISH:
	if(line!=NULL) free(line);
	if(start!=NULL) free(start);
	return(matrix);
}



/******************************************************************************************************
INTERNAL FUNCTION: DERIVED FROM XF_LINEREAD1:
	A safer version of fgets to read a line of unknown length from an ASCII file
	Characters are read in buffers of length XF_LINEREAD1_BUFFSIZE (typically 10,000)
		- If buffer length is less than the length of the line being read, multiple calls to fgets are made
		- the function checks if the buffer contains a newline - if so further calls to fgets are halted
	Memory reserved for "line" is expanded as required
	Blank lines are correctly returned
	Lines without a terminating newline are correctly returned
	Input must be NULL-terminated
******************************************************************************************************/
#define XF_LINEREAD1_BUFFSIZE 10000
char *xfinternal_lineread1(char *line, long *maxlinelen, FILE *fpin) {
	char buffer[XF_LINEREAD1_BUFFSIZE];
	size_t sizeofchar=sizeof(char);
	long nchars;
	/* IF NO MEMORY IS ALLOCATED FOR "line" DO IT NOW */
	if(line==NULL) {
		if(*maxlinelen<1) *maxlinelen=XF_LINEREAD1_BUFFSIZE;
		line=(char *)realloc(line,(*maxlinelen)*sizeofchar);
		if(line==NULL)  {*maxlinelen=-1; return(NULL);}
	}
	/* INITIALIZE LINE AND RESET NUMBER OF CHARACTERS */
	nchars=0; line[0]='\0';
	/* GET MULTIPLE BUFFFERS AND CONCATENATE THEM */
	while(fgets(buffer,XF_LINEREAD1_BUFFSIZE,fpin)!=NULL) {
		/* calculate how many characters were read, not including the terminating '\0' */
		nchars+=strlen(buffer);
		/* if more characters have been read that there is currently storage for, allocate more memory */
		if(nchars>*maxlinelen) {
			*maxlinelen=nchars;
			line=(char *)realloc(line,((nchars+1)*sizeofchar));
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



/******************************************************************************************************
INTERNAL FUNCTION: DERIVED FROM _LINEPARSE1:
	* Modify a line, parsing it into whitespace-delimited words, and modify an array of start-indices to each word
	* The variable "nwords" will be modified to reflect the number of words on the line
	* The "line" array will be modified, each white-space delimiter replaced by a NULL character

	* NOTE: 	multiple whitespace is treated as a single whitespace, so "empty"
				words in a line will affect the index to all subsequent columns.
******************************************************************************************************/
long *xfinternal_lineparse1(char *line,long *nwords) {
	int i,j,prevwhite;
	long *start=NULL;
	size_t nchars;
	/* IF THIS IS A BLANK LINE (CARRIAGE RETURN ONLY), THERE ARE NO WORDS, SO RETURN NULL */
	nchars=strlen(line);
	if(nchars==0||line[0]=='\n') {*nwords=0;return(NULL);}
	/* reallocate memory for start array */
	start=(long *)realloc(start,(nchars*sizeof(long)));
	if(start==NULL)  {return(NULL);}
	*nwords=0;
	prevwhite=1;
	for(i=0;i<nchars;i++) {
		/* if this is a white-space character, reset it to NULL */
		if( line[i]==' '||line[i]=='\t'||line[i]=='\n'||line[i]=='\r') {
			line[i]='\0';
			prevwhite=1;
		}
		/* if this is NOT white-space and previous character was, then this is the start of a new word */
		else if(prevwhite==1) {
			start[*nwords]=i ;
			*nwords=*nwords+1;
			prevwhite=0;
		}
	}
	for(i=*nwords;i<nchars;i++) start[i]=-1;
	return(start);
}
