/*
<TAGS>matrix filter</TAGS>

DESCRIPTION:
	Resample a series of numbers representing a 2D matrix, so
	there are a user-defined number of rows and columns

	Rows and columns are split (new dimensions > old dimensions)
	or binned & averaged (new dimensions >= old dimansions), accordingly

	Alters input array
	NAN values will be ignored, but INF will affect the results

	[JRH] 23 November 2015

USES:
	- resampling an image or matrix to reduce or increase resolution
	- modifying different matrices so they are all the same size

DEPENDENCY TREE:
	xf_bin1a_d
	xf_expand1_d

ARGUMENTS:
	double *martix1: input, array of numbers to be resampled - matrix format
	long nx1: original matrix width
	long ny1: original matrix height
	long nx2: original matrix width
	long ny2: original matrix height

RETURN VALUE:
	Pointer to modified matrix - NULL on error

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);
double *xf_expand1_d(double *data , long nn, long setn, char *message);


double *xf_matrixresample1_d(double *matrix1, long nx1, long ny1, long nx2, long ny2) {

	char message[256];
	long i,j,k,x1,x2,y1,y2,n1,n2;
	double *matrix2=NULL,*tempdata=NULL,aa;
	size_t sizeofdouble=sizeof(double),iii,jjj,zzz;

	double *xfinternal_resample1(double *tempdata , long tempn, long setn);

	if(nx1==nx2 && ny1==ny2) return(matrix1);

	/* find largest dimension required for temporary array */
	k=nx1;
	if(nx2>k) k=nx2;
	if(ny1>k) k=ny1;
	if(ny2>k) k=ny2;

	/* calculate the total size of original and destination arrays (n1 and n2, respectively) */
	n1= nx1*ny1;
	n2= k*k;

	/* allocate memory */
	tempdata=(double *)realloc(tempdata,((k)*sizeof(double)));
	if(tempdata==NULL) return(NULL);
	matrix2=(double *)realloc(matrix2,((n2)*sizeof(double)));
	if(matrix2==NULL) return(NULL);

	/* initialize the destination array */
	for(i=0;i<n2;i++) matrix2[i]=NAN;


 	/* RESAMPLE EACH ROW */
	for(y1=0;y1<ny1;y1++) {

		/* set index j to the beginning of current row */
		j=y1*nx1;
		/* copy row to tempdata and resample */
		for(x1=0;x1<nx1;x1++) tempdata[x1]=matrix1[j+x1];

		if(nx1<=nx2) {
			tempdata= xf_expand1_d(tempdata,nx1,nx2,message);
			if(tempdata==NULL) return(NULL);
		}
		else {
			zzz=0;
			iii=(size_t)nx1;
			aa= xf_bin1a_d(tempdata,&iii,&zzz,nx2,message);
			if(aa<=0.0) return(NULL);
		}

		/* set index j to the beginning of current row */
		j=y1*nx2;
		/* copy tempdata to a corresponding row in matrix2 */
		for(x2=0;x2<nx2;x2++) matrix2[j+x2]=tempdata[x2];

	}

	/* RESAMPLE EACH COLUMN - use the new range of columns */
	for(x2=0;x2<nx2;x2++) {

		/* copy column to tempdata */
		for(y1=0;y1<ny1;y1++) tempdata[y1]=matrix2[y1*nx2+x2];

		/* resample temporary column */
		if(ny1<=ny2) {
			tempdata= xf_expand1_d(tempdata,ny1,ny2,message);
			if(tempdata==NULL) return(NULL);
		}
		else {
			zzz=0;
			jjj=(size_t)ny1;
			aa= xf_bin1a_d(tempdata,&jjj,&zzz,ny2,message);
			if(aa<=0.0) return(NULL);
		}

		/* copy temporary column back into a column in matrix2 */
		for(y2=0;y2<ny2;y2++) matrix2[y2*nx2+x2]=tempdata[y2];
	}

	free(tempdata);
	return(matrix2);
}
