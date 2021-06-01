/*
<TAGS>file dt.matrix</TAGS>

DESCRIPTION:
	Read a text matrix of values from a table with a header-line
		- based on roughly xf_matrixread3_d
		- assumes input is tab-delimited
		- blank lines are ignored  - i.e. strlen(line)=1
	Stores the header line (first non-blank row)
		- this is used to determine the number of columns
		- all rows must be the same width
	All subsequent non-blank lines are stored as double-precision floating point values (64-bit float)
		- "missing" values and non-numeric values are stored as NAN

USES:
	- easy reading of an entire table of values, including the column-names

DEPENDENCIES:
	char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
	long *xf_lineparse2(char *line, char *delimiters, long *nwords);

ARGUMENTS:
	char *infile    - input: name of input file, or "stdin"
	long *ncols     - output: number of columns on each row ( pass as address to long )
	long *nrows     - output: number of rows ( pass as address to long )
	char **header   - output: pointer to header-line ( pass as address to char*, calling function should free)
	char *message   - output[256]: error message, if any (calling function must pre-allocate)

RETURN VALUE:
	- on success: pointer to matrix of numbers
		- header* points to the header-line - main() should free this at the end
		- nrows and ncols will be updated
	- on error: NULL
		- message[256] describes the error

SAMPLE CALL:
	FILE *fpin= fopen(infile,"r"))==0);
	char *header,message[256];
	long nrows,ncols,ii,jj;
	double *data1=NULL,*pdata;

	data1= xf_readtable1_d(fpin,"\t",&ncols,&nrows,&header,message);
	if(data1==NULL) { fprintf(stderr,"\n---Error: %s\n\n",message); exit(1); }

	printf("%s",header);
	for(ii=0;ii<nrows;ii++) {
		pdata= data1+(ii*ncols);
		printf("%g",pdata[0]); for(jj=1;jj<ncols;jj++) printf("\t%g",pdata[jj]); printf("\n");
	}


	fclose(fpin);
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line, char *delimiters, long *nwords);
/* external functions end */

double *xf_readtable1_d(char *infile, char *delimiters, long *ncols, long *nrows, char **header, char *message) {

	FILE *fpin=NULL;
	char *thisfunc= "xf_matrixread3_d\0";
	char *line=NULL,*templine=NULL;
	int prevblank=0,foundheader=0;
	long *start=NULL,ii,jj,kk,nn=0,nwords,nlines=0,nlinesmatrix=0,maxlinelen=0;
	double aa,*matrix=NULL;
	size_t sizeofmatrix;

	sizeofmatrix= sizeof(*matrix);
	*ncols=	*nrows= 0;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {sprintf(message," %s: file \"%s\" not found\n\n",thisfunc,infile);goto ERROR;}

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {sprintf(message," %s: memory allocation error",thisfunc);goto ERROR;}
		/* get total line count - helps with locating source of errors */
		nlines++;
		/* skip blank lines */
		if(strlen(line)<2) continue;

		/* read the header if it hasn't been found yet */
		if(foundheader==0) {
			foundheader= 1;
			/* save header as a pointer to an un-parsed copy of the line - upon parsing, nwords will indicate the expected number of columns */
			templine= malloc((strlen(line)+1));
			strcpy(templine,line);
			(*header)= templine;
			/* now parse the line to determine the number of columns in the table */
			start= xf_lineparse2(line,delimiters,&nwords);
			if(nwords==0) {sprintf(message,"%s: no words in header (line %ld)",thisfunc,nlines);*nrows=*ncols=-1; goto ERROR;}
			*ncols= nwords;
			continue;
		}

		/* anything else is data to be stored in matrix */
		start= xf_lineparse2(line,delimiters,&nwords);
		if(nwords!=*ncols) {sprintf(message,"%s: expecting %ld columns but found %ld on line %ld",thisfunc,*ncols,nwords,nlines);goto ERROR;}
		/* update variables */
		nlinesmatrix++;
		*ncols= nwords;
		*nrows= *nrows+1;
		/* store this row in the matrix */
 		matrix= realloc(matrix,(nn+nwords)*sizeofmatrix);
		if(matrix==NULL) {sprintf(message,"%s: memory allocation error storing matrix",thisfunc);goto ERROR;}
		for(ii=0;ii<nwords;ii++) {
			if(sscanf((line+start[ii]),"%lf",&aa)==1 && isfinite(aa)) matrix[nn]=aa;
			else matrix[nn]= NAN;
			nn++;
		}
	}
	if(matrix==NULL) {*nrows=0; *ncols=0;}
	goto FINISH;

ERROR:
	if(matrix!=NULL) free(matrix);
	*nrows=-1;
	*ncols=-1;
FINISH:
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(line!=NULL) free(line);
	if(start!=NULL) free(start);
	return(matrix);
}
