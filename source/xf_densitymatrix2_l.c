/*
<TAGS>signal_processing stats dt.matrix</TAGS>

DESCRIPTION:
	Determine the cumulative occurance of x/y values rounded to fill a matrix of fixed dimensions
	- note that x/y zero will correspond to the upper left corner of the matrix
	- this version is safe for repeated calls - memory for output assigned by calling function

USES:
	- create a density matrix of dwell-times from a series of x-y position data
	- create an action-potential density matrix from the x/y positions associated with cell firing

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *xdata : input holding x-position values data
	float *ydata : input holding y-position values data
	long nn      : input number of x/y pairs in the input
	long *matrix : output preallocated array able to hold output of size (setxbintot x setybintot)
	long setxbintot : input desired width of the matrix (must be >0)
	long setybintot : input desired height of the matrix (must be >1)
	float *ranges : input pointer to ranges[4] array defining xmin,ymin,xmax,ymax, or NANs for auto-setting each range based on *xdata and *ydata
	char *message : onput pre-allocated array to hold error message

RETURN VALUE:
	status flag (0=OK, -1=ERROR)
	matrix array will be overwritten
	message array will hold message (if any)

SAMPLE CALL:
	long matrix[15];
	width=3; height=5;
	z= xf_densitymatrix1_f(xdata,ydata,nn,matrix,width,height,message);
	if(z=-1)  { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<(width*height);ii++) {
		printf("%ld\t",matrix[ii]);
		if((ii+1)%width=0) printf("\n");
	}


*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

int xf_densitymatrix2_l(float *xdata, float *ydata, long nn, long *matrix, long setxbintot, long setybintot, float *ranges, char *message) {

	char *thisfunc="xf_densitymatrix2_l\0";
	int foundgood;
	float a,b;
	long ii,jj,kk,xbin,ybin,matrixsize;
	double xmin,xmax,ymin,ymax,xrange,yrange;
	double xbinwidth,ybinwidth,xbinwidth_inv,ybinwidth_inv;

	if(nn<1) {sprintf(message,"%s [ERROR]: no data in input",thisfunc); return(-1); }
	if(setxbintot<1) {sprintf(message,"%s [ERROR]: matrix width <1 (setxbintot=%ld)",thisfunc,setxbintot); return(-1); }
	if(setybintot<1) {sprintf(message,"%s [ERROR]: matrix height <1 (setybintot=%ld)",thisfunc,setybintot); return(-1); }

	matrixsize=setxbintot*setybintot;

	/* DETERMINE RANGE */
	xmin=ranges[0];
	ymin=ranges[1];
	xmax=ranges[2];
	ymax=ranges[3];
	if(!isfinite(xmin)) { xmin= nextafter(FLT_MAX,0.0) ;  for(ii=0;ii<nn;ii++) { a=xdata[ii]; if(isfinite(a) && a<xmin) xmin=a; }}
	if(!isfinite(ymin)) { ymin= nextafter(FLT_MAX,0.0) ;  for(ii=0;ii<nn;ii++) { b=ydata[ii]; if(isfinite(b) && b<ymin) ymin=b; }}
	if(!isfinite(xmax)) { xmax= nextafter(-FLT_MAX,0.0) ; for(ii=0;ii<nn;ii++) { a=xdata[ii]; if(isfinite(a) && a>xmax) xmax=a; }}
	if(!isfinite(ymax)) { ymax= nextafter(-FLT_MAX,0.0) ; for(ii=0;ii<nn;ii++) { b=ydata[ii]; if(isfinite(b) && b>ymax) ymax=b; }}
	foundgood=0;
	for(ii=0;ii<nn;ii++) {
		a=xdata[ii];
		b=ydata[ii];
		if(isfinite(a) && a>=xmin && a<=xmax && isfinite(b) && b>=ymin && b<=ymax) { foundgood=1; break; }
	}
	if(foundgood==0) { sprintf(message,"%s [ERROR]: input contains no valid in-range numbers",thisfunc); return(-1); }
	xrange=xmax-xmin;
	yrange=ymax-ymin;

	/* DETERMINE BIN-WIDTH */
	xbinwidth = xrange/(double)setxbintot;
	ybinwidth = yrange/(double)setybintot;

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

	return (0);
}
