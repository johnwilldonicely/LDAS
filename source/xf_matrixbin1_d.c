/*
<TAGS>filter matrix</TAGS>
DESCRIPTION:
	Downsize an input array representing a 2D matrix to a new width and heght
	Alters the input array
	NAN and INF values will be ignored
	This function has no memory overhead

USES:
	- resampling an image or matrix to reduce resolution
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
	0 on sucess, -1 on error

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_matrixbin1_d(double *matrix1, long nx1, long ny1, long nx2, long ny2, char *message) {

	char *thisfunc="xf_matrixbin1_d\0";
	long ii,jj,kk,x1,x2,y1,y2,nsums,xmax,ymax;
	double xbinsize,ybinsize,limit,sum;

	if(nx2>nx1) { sprintf(message,"%s [ERROR]: nx1 (%ld) must be >= nx2 (%ld)",thisfunc,nx1,nx2); return(1); }
	if(ny2>ny1)  { sprintf(message,"%s [ERROR]: ny1 (%ld) must be >= ny2 (%ld)",thisfunc,ny1,ny2); return(1); }
	if(nx2==nx1 && ny2==ny1)  { sprintf(message,"%s [Warning]: input and output matrices are the same size (no changes made)",thisfunc,ny1,ny2); return(0); }

	xmax=nx1-1;
	ymax=ny1-1;
	xbinsize=(double)(nx1)/(double)nx2;
	ybinsize=(double)ny1/(double)ny2;

 	/* RESAMPLE EACH ROW */
	if(nx2<nx1) {
		for(y1=0;y1<ny1;y1++) {
			/* set index jj to the beginning of current row, for reading the input */
			/* kk is the initial pointer to the destination, which will be incremented  */
			jj=kk=y1*nx1;
			/* set initial limit for binning and initialize sum and nsums */
			limit=xbinsize;
			sum=0.0;
			nsums=0;
			/* for each column, build the sum - calculate the mean if limit is exceeded */
			for(x1=0;x1<nx1;x1++) {
				if(x1>=limit) {
					if(nsums>0) matrix1[kk]= sum/nsums;
					else matrix1[kk]=NAN;
					sum=0;
					nsums=0;
					limit+= xbinsize;
					kk++;
				}
				if(isfinite(matrix1[jj+x1])) { sum+= matrix1[jj+x1]; nsums++; }
			}
			/* process the last sample */
			if(nsums>0) matrix1[kk]= sum/nsums;
			else matrix1[kk]=NAN;
		}
		//TEST: for(y1=0;y1<ny1;y1++) {for(x1=0;x1<nx1;x1++) printf("%.3f\t",matrix1[y1*nx1+x1]);printf("\n");} exit(0);
	}

	/* RESAMPLE EACH COLUMN - use the new range of columns */
	if(ny2<ny1) {
		for(x1=0;x1<nx2;x1++) {
			/* set initial index to destination: this will be incremented by nx1 */
			kk=x1;
			/* set initial limit for binning and initialize sum and nsums */
			limit=ybinsize;
			sum=0.0;
			nsums=0;
			/* for each row, build the sum - calculate the mean if limit is exceeded */
			for(y1=0;y1<ny1;y1++) {
				/* set jj to index for input */
				jj=y1*nx1+x1;
				if(y1>=limit) {
					if(nsums>0) matrix1[kk]= sum/nsums;
					else matrix1[kk]=NAN;
					sum=0;
					nsums=0;
					limit+= ybinsize;
					kk+=nx1;
				}
				if(isfinite(matrix1[jj])) { sum+= matrix1[jj]; nsums++; }
			}
			/* process the last sample */
			if(nsums>0) matrix1[kk]= sum/nsums;
			else matrix1[kk]=NAN;
		}
		//TEST: for(y1=0;y1<ny1;y1++) {for(x1=0;x1<nx1;x1++) printf("%.3f\t",matrix1[y1*nx1+x1]);printf("\n");} exit(0);
	}

	/* COMPRESS MATRIX */
	kk=0;
	for(y2=0;y2<ny2;y2++){
		jj=y2*nx1;
		for(x2=0;x2<nx2;x2++) {
			matrix1[kk++]=matrix1[jj+x2];
		}
	}

	/* RETURN THE ERROR CODE */
	return(0);
}
