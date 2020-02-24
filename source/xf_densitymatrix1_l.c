/*
<TAGS>signal_processing stats dt.matrix</TAGS>

DESCRIPTION:
	Determine the cumulative occurance of x/y values rounded to fill a matrix of fixed dimensions
	- note that x/y zero will correspond to the upper left corner of the matrix
	WARNING: do not use for repeated calls assign memory for the same array
		- consider using xf_densitymatrix2_l instead

USES:
	- create a density matrix of dwell-times from a series of x-y position data
	- create an action-potential density matrix from the x/y positions associated with cell firing

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *xdata : input holding x-position values data
	float *ydata : input holding y-position values data
	long nn      : number of x/y pairs in the input
	long setxbintot : desired width of the matrix (must be >0)
	long setybintot : desired height of the matrix (must be >1)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	pointer to matrix of setxbintot x setybintot values, or NULL on error
	char array will hold message (if any)

SAMPLE CALL:
	long *matrix=NULL;
	width=3; height=5;
	matrix= xf_densitymatrix1_f(xdata,ydata,nn,width,height,message);
	if(matrix==NULL)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<(width*height);ii++) {
		printf("%ld\t",matrix[ii]);
		if((ii+1)%width=0) printf("\n");
	}


*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

long *xf_densitymatrix1_l(float *xdata, float *ydata, long nn, long setxbintot, long setybintot, char *message) {

	char *thisfunc="xf_densitymatrix1_l\0";
	long ii,jj,kk,xbin,ybin,matrixsize;
	long *matrix=NULL;
	double xmin,xmax,ymin,ymax,xrange,yrange;
	double xbinwidth,ybinwidth,xbinwidth_inv,ybinwidth_inv;

	if(nn<1) {sprintf(message,"%s [ERROR]: no data in input",thisfunc); return(NULL); }
	if(setxbintot<1) {sprintf(message,"%s [ERROR]: matrix width <1 (setxbintot=%ld)",thisfunc,setxbintot); return(NULL); }
	if(setybintot<1) {sprintf(message,"%s [ERROR]: matrix height <1 (setybintot=%ld)",thisfunc,setybintot); return(NULL); }

	matrixsize=setxbintot*setybintot;
	matrix = (long *) realloc(matrix,(matrixsize*sizeof(long)));
	if(matrix==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }

	/* DETERMINE RANGE */
	xmin=ymin=  FLT_MAX -1.0 ;
	xmax=ymax= -FLT_MAX ;
	for(ii=0;ii<nn;ii++) {
		if(isfinite(xdata[ii]) && isfinite(ydata[ii])) {
			if(xdata[ii]<xmin) xmin=xdata[ii];
			if(xdata[ii]>xmax) xmax=xdata[ii];
			if(ydata[ii]<ymin) ymin=ydata[ii];
			if(ydata[ii]>ymax) ymax=ydata[ii];
		}
	}
	if(xmin==-FLT_MAX) { sprintf(message,"%s [ERROR]: input contains no valid numbers",thisfunc); return(NULL); }

	xrange=xmax-xmin;
	yrange=ymax-ymin;

	/* DETERMINE BIN-WIDTH */
	xbinwidth = xrange/(double)setxbintot;
	ybinwidth = yrange/(double)setybintot;
//fprintf(stderr,"xbinwidth=%g\tybinwidth=%g\n",xbinwidth,ybinwidth);

	/* DETERMINE INVERSE BIN-WIDTH: avoids double float division below - should speed things up a bit */
	/* - also note that we use an ever-so-slightly larger denominator to avoid bin-rollover */
	xbinwidth_inv = 1.0/nextafter(xbinwidth,DBL_MAX);
	ybinwidth_inv = 1.0/nextafter(ybinwidth,DBL_MAX);

	/* INITIALISE MATRIX */
	for(ii=0;ii<matrixsize;ii++) matrix[ii]=0.0;

	/* BUILD THE MATRIX */
	for(ii=0;ii<nn;ii++) {
		xbin= (long)( (xdata[ii]-xmin) * xbinwidth_inv);
		ybin= (long)( (ydata[ii]-ymin) * ybinwidth_inv);
		jj= ybin*setxbintot+xbin;
		matrix[jj]++;
	}

//fprintf(stderr,"xmin=%g	xmax=%g\n",xmin,xmax);
//fprintf(stderr,"ymin=%g	ymax=%g\n",ymin,ymax);
//for(ii=0;ii<matrixsize;ii++) fprintf(stderr,"%ld\n",matrix[ii]);

	return (matrix);
}
