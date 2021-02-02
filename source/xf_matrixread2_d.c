/*
<TAGS>file dt.matrix</TAGS>

DESCRIPTION:
		Read one or more matrices from an input
		Each matrix should be separated by a blank line or a line whos first character is "# followed by an identifier number"
		Data is stored as a continuous array of double-precision floating point numbers
		Automatically detects the number of rows and columns in the input
			- the first matrix is taken as the model - discrepancies result in errors
		The number of matrices and the number of rows and columns are stored

		NOTE: 	multiple whitespace is treated as a single whitespace, so "empty"
	       			words in a line will affect the index to all subsequent columns.
USES:
	storing 2-d blocks of ASCII data into memory as a 1d array

DEPENDENCIES:
	char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);

ARGUMENTS:
	char *infile     - name of the input file, or "stdin"
	long idcol	 - for lines starting with "#", the zero-offset column-number holding the id (typically "1")
	double **matrix1 - result, multi-matrix data - freed by calling function
	double **id1     - result, id's for each matrix - freed by calling function
	long *ncols      - result, how many columns are on each row of each matrix
	long *nrows      - result, how many rows are in each matrix
	char *message    - holds error messages - should be defined by calling funtion as message[256]
	FILE *fpin       - pointer to input stream (typically a file or stdin) - updated by the function

RETURN VALUE:
	- the number of matrices detected, or -1 on error
	- the matrix1[] array is filled
	- the id1[] array stores the identifiers for each matrix, presuming they are preceded by a "# <identifier>" line
	- the message[] array will hold any errors

SAMPLE CALL:
	char message[256];
	double *matrix1=NULL, *id1=NULL, *pmatrix;
	long nmatrices,ncols,nrows,ii,jj,kk,mm,idcol=1;
	nmatrices= xf_matrixread1_d(&matrix1,idcol,&id1,&ncols1,&nrows1,message,fpin);

	mm=5;
	pmatrix= matrix1+(mm*ncols*nrows); // select fifth matrix
	printf("# %g\n",id1[mm]); // print the identifier
	for(ii=0;ii<nrows;ii++) {
		for jj=0;jj<ncols;jj++) printf("%g ",pmatrix[ii*ncols+jj]);
		printf("\n");
	}

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse1(char *line,long *nwords);
/* external functions end */

long xf_matrixread2_d(char *infile, long idcol, double **matrix1, double **id1, long *ncols, long *nrows, char *message) {

	char *thisfunc="xf_matrixread2_d\0";
	char *line=NULL;
	int prevblank=1;
	long *start=NULL,ii,jj,kk,nn=0,nwords,nrowstemp=0,nlines=0,maxlinelen=0,nmatrices;
	double *matrix2=NULL,*id2=NULL,tempid,aa;
	size_t sizeofmatrix= sizeof(*matrix2);
	FILE *fpin;

	*ncols=0; *nrows=0; nmatrices=0; tempid=NAN;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) { sprintf(message," %s: file \"%s\" not found",thisfunc,infile); nmatrices=-1; goto FINISH; }

	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {

		nlines++;
		if(maxlinelen==-1)  {matrix2=NULL; sprintf(message," %s: memory allocation error reading line %d",thisfunc,nlines); nmatrices=-1; goto FINISH;}
		start= xf_lineparse1(line,&nwords);

		/* THIS IS DATA, STORE THE VALUES FROM THE LINE */
		if(line[0]!='#' && nwords>0) {
			matrix2= realloc(matrix2,(nn+nwords)*sizeofmatrix);
			if(matrix2==NULL) {matrix2=NULL; sprintf(message,"%s: memory allocation error storing matrix2",thisfunc); nmatrices=-1; goto FINISH;}
			for(ii=0,jj=nn;ii<nwords;ii++) {
				if(sscanf((line+start[ii]),"%lf",&aa)==1 && isfinite(aa)) matrix2[jj]= aa;
				else matrix2[jj]=NAN;
				jj++;
			}
			if(nrowstemp==0 && nmatrices==0) *ncols=nwords;
			else if(nwords!=*ncols) {matrix2=NULL; sprintf(message,"%s: unequal number of columns detected at line %d",thisfunc,nlines); nmatrices=-1; goto FINISH;}
			prevblank=0; // for next iteration, indicate that previous line contained matrix data
			nrowstemp++; // count the number of rows for this matrix
			nn+=nwords;  // count the total number of items in the matrix for next memory allocation iteration
		}
		/* IF IT'S NOT DATA... (THIS COULD BE A BLANK LINE OR A COMMENT - IF A COMMENT, TEMPID HAS JUST BEEN SET)*/
		else {
			if(prevblank==0) {
				if(nmatrices==0) *nrows=nrowstemp;
				else if(nrowstemp!=(*nrows)) {matrix2=NULL; sprintf(message,"%s: unequal number of rows detected at line %d",thisfunc,nlines); nmatrices=-1; goto FINISH;}
				id2= realloc(id2,(nmatrices+1)*sizeof(*id2));
				if(id2==NULL) {id2=NULL; sprintf(message,"%s: memory allocation error storing id2",thisfunc); nmatrices=-1; goto FINISH;}
				id2[nmatrices]= tempid;
				nmatrices++;
			}
			/* if it's a comment-line, store the id (first word after the # symbol) */
			if(line[0]=='#') {
				if(nwords>idcol) tempid= atof(line+start[idcol]);
				else tempid= NAN;
			}
			prevblank=1;
			nrowstemp=0;
		}
	}

	/* IF YOU GET TO THE END OF THE FILE AND THE PREVIOUS LINE WAS NOT BLANK/COMMENT, TREAT THIS AS THE CONCLUSION OF A MATRIX */
	if(prevblank==0) {
		if(nmatrices==0) *nrows=nrowstemp;
		else if(nrowstemp!=(*nrows)) {matrix2=NULL; sprintf(message,"%s: unequal number of rows detected at line %d",thisfunc,nlines); nmatrices=-1; goto FINISH;}
		id2= realloc(id2,(nmatrices+1)*sizeof(*id2));
		if(id2==NULL) {id2=NULL; sprintf(message,"%s: memory allocation error storing id2",thisfunc); nmatrices=-1; goto FINISH;}
		id2[nmatrices]= tempid;
		nmatrices++;
	}

	if(nmatrices<1){matrix2=NULL; sprintf(message,"%s: no matrices in file",thisfunc); nmatrices=-1; goto FINISH;}

FINISH:
	(*matrix1)= matrix2;
	(*id1)= id2;
	if(line!=NULL) free(line);
	if(start!=NULL) free(start);
	return(nmatrices);
}
