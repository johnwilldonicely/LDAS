/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate the Pearson's correlation and returns r
	- this version for integer values of x and y
	Fills a "results" array with  slope, and intercept
	Non-numeric values, Nan and Inf should be removed before calling this function
	Should only be used if there are 4 or more data points

USES:
	Simple correlation statistics on parallel arrays of data

DEPENDENCIES:
	None

ARGUMENTS:
	int *x         : input, x-data array
	int *y         : input, y-data array
	long nn        : input, number of elements in x & y
	double *result : output, pre-allocated array to hold results - must allow at least 3 elements

RETURN VALUE:
	Pearson's r (or NAN or 0 if there was a problem)
	if r==0, it may be because the data represented a vertical or horizontal line

	result array will hold slope and intercept

SAMPLE CALL:
	r=  xf_correlate_simple_i(x,y,n,result);
*/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>

double xf_correlate_simple_l(long *x, long *y, long nn, double *result_d) {
	long  i;
	long tempx,tempy,sumx=0.0,sumy=0.0,sumx2=0.0,sumy2=0.0,sumxy=0.0;
	double aa,bb;
	double SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0;
	double SSx=0.0,SSy=0.0,SPxy=0.0, r=0.0;

	for(i=0;i<nn;i++) {
		tempx= x[ii];
		tempy= y[ii];
		sumx+= tempx;
		sumy+= tempy;
		sumx2+= tempx*tempx;
		sumy2+= tempy*tempy;
		sumxy+= tempx*tempy;
	}
	SUMx= (double)sumx;
	SUMy= (double)sumy;
	SUMx2= (double)sumx2;
	SUMy2= (double)sumy2;
	SUMxy= (double)sumxy;
	SSx= SUMx2-((SUMx*SUMx)/nn);
	SSy= SUMy2-((SUMy*SUMy)/nn);
	SPxy= SUMxy-((SUMx*SUMy)/nn);

	/* if data describes horizontal line... */
	if(SSy==0.0) {
		r=0.0;
		result_d[0]=bb;
		result_d[1]=0.0;
	}
	/* if data describes vertical line (or single point) ... */
	else if(SSx==0.0) {
		r=0.0;
		result_d[0]=NAN;
		result_d[1]=NAN;
	}
	/* otherwise, if there is variability in both x and y... */
	else {
		r = SPxy/(sqrt(SSx)*sqrt(SSy));
		result_d[1]= SPxy/SSx;	/* slope */
		result_d[0]= (SUMy/nn)-(result_d[1]*(SUMx/nn)); /* intercept */
	}
	return(r);
}
