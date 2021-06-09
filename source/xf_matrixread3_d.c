/*
<TAGS>file dt.matrix</TAGS>

DESCRIPTION:
	Read one matrix from a single- or multi-matrix input
	Multiple matrices should be separated by a blank (\n) or header-line (#)
		- every encountered header-line updates the header array
		- if data has been read at this point, the function returns
		- therefore, the header is set for subsequent data on the next call
	Automatically detects the number of rows and columns in the input
		- row #1 is taken as the default number of columns
		- all rows must be the same width
	Stores the preceding and most recent header line (#)

	NOTE: 	multiple whitespace is treated as a single whitespace, so "empty"
       			words in a line will affect the index to all subsequent columns.

USES:
	- Finding matrices for a given subject or group in a multi-matrix file

DEPENDENCIES:

	char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
	long *xf_lineparse1(char *line,long *nwords);

ARGUMENTS:
	FILE *fpin      - input: pointer to input stream (typically a file or stdin) - updated by the function
	long *ncols     - output: number of columns on each row (pass as address)
	long *nrows     - output: number of rows (pass as address)
	char *header    - output[256]: header for the current matrix
	char *message   - output[256]: header for next matrix, or error message, if any

	NOTE: calling function must allocate at least 256-bytes each for header[] and message[]
	NOTE: header[] and message[256] should be initialized to '\n\0' before first call to this function

RETURN VALUE:
	- on success:
		- A pointer to an array holding the matrix (NULL at end of file)
		- header[] contains the header preceeding the matrix
		- message[] contains the header for the next matrix, if there there is no preceeding blank line
	- on error:
		- ncols and nrows are set to -1
		- message[] describes the error

SAMPLE CALL:
	char header[256],message[256];
	long nrows=0,ncols=0,ii,jj,kk;
	FILE *fpin= fopen(infile,"r"))==0);
	header[0]='\n'  ; header[1]='\0';
	message[0]='\n' ; message[1]='\0';
	while(1) {

		matrix= xf_matrixread3_d(fpin,&width,&height,header,message);

		if(height<0) {fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message);exit(1);}
		if(matrix1==NULL) break;
		printf("%s",header);
		for(ii=0;ii<height;ii++) {
			kk= ii*width;
			for(jj=0;jj<width;jj++) { if(jj>0) printf(" ");	printf("%g",matrix1[kk+jj]); }
			printf("\n");
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

double *xf_matrixread3_d(FILE *fpin, long *ncols, long *nrows, char *header, char *message) {

	char *thisfunc= "xf_matrixread3_d\0";
	char *line=NULL;
	int prevblank=0;
	long *start=NULL,ii,jj,kk,nn=0,nwords,nlines=0,nlinesmatrix=0,maxlinelen=0;
	double aa,*matrix=NULL;
	size_t sizeofmatrix;

	sizeofmatrix= sizeof(*matrix);
	*ncols=	*nrows= 0;

	/* use message[] to initialize header[] */
	/* this represents the memory of the last header from a previous call */
	header= strncpy(header,message,256);

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {sprintf(message," %s: memory allocation error",thisfunc);*nrows=*ncols=-1;goto FINISH;}
		nlines++; // total line count - helps with locating source of errors

		/* DEAL WITH HEADER LINES WHICH DEFINE PARAMETERS FOR THIS OR THE NEXT MATRIX */
		if(line[0]=='#') {
			/* if data has NOT been found, initialize the header now */
			if(nlinesmatrix==0) { header= strncpy(header,line,256); continue; }
			/* if data HAS been found, the header is saved to message[] for the NEXT call */
			else { message= strncpy(message,line,256); break; }
		}

		/* DEAL WITH BLANK LINES - AS ABOVE BUT DO NOT REDEFINE HEADER */
		else if(line[0]=='\n') {
			/* if data has been NOT been found, just carry on */
			if(nlinesmatrix==0) { continue;	}
			/* if data HAS been found, message[] is reset to '\n\0'for the NEXT call */
			else { message[0]='\n'; message[1]='\0'; break; }
		}

		/* DEAL WITH EVERYTHING ELSE - THIS ROW MUST CONTAIN MATRIX VALUES */
		else {
			/* parse the line into words - check for consistent number of columns */
			start= xf_lineparse1(line,&nwords);
			if(nwords!=*ncols && nlinesmatrix>0) {matrix=NULL; sprintf(message,"%s: unequal number of columns detected at line %ld",thisfunc,nlines);*nrows=*ncols=-1;goto FINISH;}
			/* update variables */
			nlinesmatrix++;
			*ncols= nwords;
			*nrows= *nrows+1;
			/* store this row in the matrix */
	 		matrix= realloc(matrix,(nn+nwords)*sizeofmatrix);
			if(matrix==NULL) {sprintf(message,"%s: memory allocation error storing matrix",thisfunc);*nrows=*ncols=-1;goto FINISH;}
			for(ii=0;ii<nwords;ii++) {
				if(sscanf((line+start[ii]),"%lf",&aa)==1 && isfinite(aa)) matrix[nn]=aa;
				else matrix[nn]= NAN;
				nn++;
			}
		}
	}

	if(matrix==NULL) {*nrows=0; *ncols=0;}

FINISH:
	if(line!=NULL) free(line);
	if(start!=NULL) free(start);
	return(matrix);
}
