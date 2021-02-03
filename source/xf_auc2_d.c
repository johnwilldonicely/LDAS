/*
<TAGS>stats</TAGS>
17.January.2017 (JRH)

DESCRIPTION:
	- Calculate the area under the curve using the trapezoid method
		- https://en.wikipedia.org/wiki/Integral
		- http://en.wikipedia.org/wiki/Trapezoidal_rule
	- dual input arrays: x (time)  and value (y)
	- suitable for unevenly sampled data
	- requires at least two valid points for meaningfull results
	- options:
		[ref] defines magnitude (y) at each point
		0: reference to zero (standard definition of AUC)
		1: reference to a line joining the ends
			- distance from this line instead of original y-value
USES:
	- measuring the effect-size of a drug response
	- measuring power in a region of an FFT spectrogram
	- calculation of population-spike for slice LTP experiments

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *curvex : input array holding x-values
	double *curvey : input array holding y-values
	long nn        : length of curvex and curvey (number of elements)
	int ref        : y-value reference, 0 or 1 (see definition above)
	double *result : pre-allocated array holding results (must hold 4 elements)
	char *message  : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result[0] = total AUC (positive + negative)
	result[1] = positive AUC
	result[2] = negative AUC


SAMPLE CALL: GET NEGATIVE-ONLY AUC FOR 100 POINTS, REFERENCED TO ZERO
	x= xf_auc2_d(datax,datay,100,0,result,message)
	if(x!=0) fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	else auc=result[2];
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int xf_auc2_d(double *curvex, double *curvey, size_t nn, int ref, double *result ,char *message) {

	char *thisfunc="xf_auc_d\0";
	size_t ii,jj,kk;
	double aa,bb,cc,dd;
	double x1,x2,y1,y2,dx,dy,slope0,intercept0;
	double slope,xintercept,yintercept,aucneg,aucpos;

	if(nn<=0) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); return(-1); }

	/* CALCULATE THE SLOPE AND INTERCEPT OF THE LINE JOINING THE FIRST AND LAST POINTS */
	x1=curvex[0]; // first value of x
	y1=curvey[0]; // first value of y
	x2=curvex[nn-1]; // last value of x
	y2=curvey[nn-1]; // last value of y
	if(y1==y2) slope0= 0; // avoids division-by-zero error
	else slope0= (y2-y1)/(x2-x1);
	intercept0= y1-(x1*slope0);

	/* STARTING FROM THE SECOND POINT, CALCULATE AUC OF THE POLYGON COMPLETED USING THE PREVIOUS POINT */
	aucpos=aucneg=0.0;
	for(ii=1;ii<nn;ii++) {
		x1=curvex[ii-1]; // previous value of x
		y1=curvey[ii-1]; // previous value of y
		x2=curvex[ii];   // current value of x
		y2=curvey[ii];   // current value of y
		/* if ref==1, modify y1 and y2 according to offset from line joining the first and last points */
		if(ref==1) {
			y1-= x1*slope0+intercept0;
			y2-= x2*slope0+intercept0;
		}
		dx=x2-x1; // change in x
		dy=y2-y1; // change in y
		/* if this point and the last lie on the same side of the y-axis... */
		if(y1*y2>=0) {
			/* area is the box formed by dx & y, minus (or plus, if dy is negative) the triangle formed by dx and dy */
			aa= dx*y2 - 0.5*dx*dy;
			if(aa>0) aucpos+=aa;
			if(aa<0) aucneg+=aa;
		}
		/* if points lie on opposite sides of the y-axis, calculate area of two triangles */
		else {
			slope= dy/dx;
			yintercept= y1-slope*x1;
			xintercept= (0.0-yintercept)/slope; // slope cannot be zero for points on opposite sides of the axis
			/* calculate area of first triangle - negative if y1 is negative */
			aa=0.5*(xintercept-x1)*y1;
			if(aa>0) aucpos+=aa;
			if(aa<0) aucneg+=aa;
			/* calculate area of second triangle - negative if y2 is negative */
			aa=0.5*(x2-xintercept)*y2;
			if(aa>0) aucpos+=aa;
			if(aa<0) aucneg+=aa;
		}
	}

	/* FILL RESULTS ARRAY AND RETURN */
	result[0]= aucpos + aucneg;
	result[1]= aucpos;
	result[2]= aucneg;
	return(0);
}
