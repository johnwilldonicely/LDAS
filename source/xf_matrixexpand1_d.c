/*
<TAGS>matrix transform</TAGS>

DESCRIPTION:
	Expand an array of numbers representing a 2D matrix
	Rows and columns are duplicated to achieve the user-specified height and width
	NAN values will be ignored, but INF will affect the results
	Input array is unaltered
	NOTE: this function requires additional memory equivalent to at least the input array size

USES:
	- resampling an image or matrix to increase resolution
	- modifying different matrices so they are all the same size

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *matrix1 : input array
	long nx1        : width of input
	long ny1        : height of input
	long nx2        : desired width
	long ny2        : desired height
	char *message   : array to hold error message


RETURN VALUE:
	Pointer to modified matrix - NULL on error

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double *xf_matrixexpand1_d(double *matrix1, long nx1, long ny1, long nx2, long ny2, char *message) {

	char *thisfunc="xf_matrixexpand1_d\0";
	long ii,jj,kk,x1,x2,y1,y2,n1,n2;
	double *matrix2=NULL,*tempdata=NULL,binsize;

	if(nx2<nx1) { sprintf(message,"%s [ERROR]: nx2 (%ld) must be >= nx1 (%ld)",thisfunc,nx2,nx1); return(NULL); }
	if(ny2<ny1) { sprintf(message,"%s [ERROR]: ny2 (%ld) must be >= ny1 (%ld)",thisfunc,ny2,ny1); return(NULL); }
	if(nx2==nx1 && ny2==ny1)  { sprintf(message,"%s [Warning]: input and output matrices are the same size (no changes made)",thisfunc,ny1,ny2); return(matrix1); }

	/* FIND LARGEST DIMENSION REQUIRED FOR TEMPORARY ARRAY */
	kk=nx1;
	if(nx2>kk) kk=nx2;
	if(ny1>kk) kk=ny1;
	if(ny2>kk) kk=ny2;

	/* CALCULATE THE TOTAL SIZE OF ORIGINAL AND DESTINATION ARRAYS (N1 AND N2, RESPECTIVELY) */
	n1= nx1*ny1;
	n2= kk*kk;

	/* ALLOCATE MEMORY */
	tempdata=(double *)realloc(tempdata,(kk*sizeof(double)));
	if(tempdata==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }
	matrix1=(double *)realloc(matrix1,(n2*sizeof(double)));
	if(matrix1==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }
	matrix2=(double *)realloc(matrix2,(n2*sizeof(double)));
	if(matrix2==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }

	/* INITIALIZE THE DESTINATION ARRAY */
	for(ii=0;ii<n2;ii++) matrix2[ii]=NAN;

 	/* EXPAND EACH ROW */
	binsize=(double)nx1/(double)nx2;
	for(y1=0;y1<ny1;y1++) {
		/* set index j to the beginning of current row */
		jj=y1*nx1;
		/* copy row to tempdata */
		for(x1=0;x1<nx1;x1++) tempdata[x1]=matrix1[jj+x1];
		/* expand */
		for(ii=(nx2-1);ii>=0;ii--) tempdata[ii]= tempdata[((long)((double)ii*binsize))];
		/* reset index j to the beginning of current row */
		jj=y1*nx2;
		/* copy tempdata to the corresponding row in matrix2 */
		for(x2=0;x2<nx2;x2++) matrix2[jj+x2]=tempdata[x2];
	}

	/* EXPAND EACH COLUMN - use the new range of columns */
	binsize=(double)ny1/(double)ny2;
	for(x2=0;x2<nx2;x2++) {
		/* copy column to tempdata */
		for(y1=0;y1<ny1;y1++) tempdata[y1]=matrix2[y1*nx2+x2];
		/* expand */
		for(ii=(ny2-1);ii>=0;ii--) tempdata[ii]= tempdata[((long)((double)ii*binsize))];
		/* copy tempdata to the corresponding column in matrix2 */
		for(y2=0;y2<ny2;y2++) matrix2[y2*nx2+x2]=tempdata[y2];
	}

	/* COPY DATA TO MATRI1 SO WE CAN FREE MATRIX2 */
	for(ii=0;ii<n2;ii++) matrix1[ii]=matrix2[ii];

	free(tempdata);
	free(matrix2);
	return(matrix1);
}
