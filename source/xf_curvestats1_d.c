/*
<TAGS>stats</TAGS>

John Huxter: 19 December 2011-2015

DESCRIPTION: Calculate statistics on a curve (single time-series, assumes fixed interval)
	COM:
		- Gerrard,et al., The Journal of Neuroscience (2008). 28(31):7883.7890
		- centre of mass (mathematical mean of observations)
		- this version uses a shifted copy of the curve (all-positive values) to avoid shifting mass to negative values when "y" is negative
		- the result is that for some curves the COM may not be exactly where you think  it should be
		- example: for a single sine-cycle, the upturn at the end will shift the COM to the right of the middle
	MEDIAN:
		- mid-point of summed observations
		- also uses the positive-shifted dataset, as per COM
	TEMPORAL BIAS:
		- Skaggs & McNaughton (1996). Science 271(5257): 1870-1873
		- normalized as per Gerrard et. al 2008
		- a measure normalized of asymmetry in the curve

USES:
	- characterization of a time-series

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *curvey : input holding y-values
	long nn : length of curvey
	double interval: interval between successive values of curvey
	double *result_d : pre-allocated array to hold results - should allow at least 16 elements
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result array will hold statistics
	char array will hold message (if any)

	result_d[0]= xymin;  x-value at the trough
	result_d[1]= ymin;   y-value at the trough
	result_d[2]= xymax;  x-value at the peak
	result_d[3]= ymax;   y-value at the peak
	result_d[4]= median; x-value dividing summed y-values in half
	result_d[5]= com;    centre of mass - i.e. the mean observation

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int xf_curvestats1_d(double *curvey, size_t nn, double interval, double *result_d, char *message) {

	char *thisfunc="xf_curvestats1_d\0";
	size_t ii,jj,kj;
	double *tempy=NULL;
	double x1,x2,y1,y2,dx,dy,aa,bb,cc,dd,bbneg=0.0,bbpos=0.0,slope,yintercept,xintercept;
	double xmin,xymin,xymax,ymin,ymax,aucneg=0.0,aucpos=0.0,auc=0.0,com,median=0.0,bias=0.0;

	if(nn<=0) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); return(-1); }
	// TEST: for(i=0;i<nn;i++) printf("	x=%g	y=%g\n",curvex[i],curvey[i]);
	for(ii=0;ii<16;ii++) result_d[ii]=0.0;

	/* CALCULATE MINIMA & MAXIMA AND SUM NEGATIVE AND POSITIVE SIDES OF HISTOGRAM FOR BIAS SCORE */
	xymin=xymax=0.0;
	ymin=ymax=curvey[0];
	for(ii=1;ii<nn;ii++) {
		x2=ii*interval; // current value of x
		y2=curvey[ii]; // current value of y
		if(x2<0) bbneg+=y2; // summed negative values in histogram - in this case will always be zero
		if(x2>=0) bbpos+=y2; // summed positive values in histogram
		if(y2<ymin) {ymin=y2;xymin=x2;} // x and y value at y-minimum - ymin used to adjust data
		if(y2>ymax) {ymax=y2;xymax=x2;} // x and y value at y-maximum
	}

	// ADJUST A TEMPORARY LOCAL COPY OF THE HISTOGRAM FOR MEDIAN AND COM CALCULATION
	tempy=(double *)realloc(tempy,(nn+1)*sizeof(double));
	if(tempy==NULL) { sprintf(message,"%s: memory allocation error",thisfunc); return(-1); }
	aa=bb=0.0;
	for(ii=0;ii<nn;ii++) {
		x2=ii*interval;
		tempy[ii]=curvey[ii]-ymin;
		aa+=x2*tempy[ii]; // = total value of observations = numerator for COM calculation
		bb+=tempy[ii];    // = total observations = denominator for COM calculation
	}

	/* FIND THE MEDIAN, CENTRE-OF-MASS AND TEMPORAL BIAS USING ADJUSTED DATA */
	/* if there is any variation from the mean... */
	if(bb>0) {
		cc=bb/2.0; // this is the halfway-point to total observations
		dd=0.0;    // this is the running counter for observations
		for(ii=0;ii<nn;ii++) {
			x2=ii*interval; // the current x-value
			dd+=tempy[ii]; // a repeat of the bb calculation - sum the observations
			if(dd==cc) {median=x2;break;}
			else if(dd>cc) {
				if(ii==0) median=interval/2.0;
				else median=(x2-(interval/2.0));
				break;
		}}
		com= (aa/bb);
		bias= (bbpos-bbneg)/(bbpos+bbneg);
	}
	/* otherwise if there is no variance in the y-values... */
	else {
		median= ((float)nn/2.0)*interval;
		com= median;
	}

	/* RETURN THE RESULTS */
	result_d[0]=xymin;  // x-value at the minimum
	result_d[1]=ymin;   // y-value at the minimum
	result_d[2]=xymax;  // x-value at the maximum
	result_d[3]=ymax;   // y-value at the maximum
	result_d[4]=median; // x-value dividing summed y-values in half
	result_d[5]=com;    // centre of mass (COM) score - i.e. the mean observation

	free(tempy);
	return(0);
}
