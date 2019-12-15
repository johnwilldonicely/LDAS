/*
<TAGS>stats</TAGS>
DESCRIPTION:
	Build a histogram (double *histx, double *histy) from an array of data (long *data)

USES:
	Typically used in conjunction with xf_wint1_l, which can produce the data values necessary for the histogram

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *data    : input array used to generate histogram
	int nn        : size of the array
	double *histx : result-array, x-axis values for histogram - NOTE! must be initialised by calling function
	double *histy : result-array, y-values for histogram - NOTE! must be initialised by calling function
	long bintot   : total number of bins in the histogram
	long min      : define bottom of the data range - NOTE than min itself will also be included
	long max      : define top of the data range - NOTE than max itself will also be included
	int format    : the format of the histogram - 1=counts, 2=proportion (peak=1), 3=probability (sum of values=1)

RETURN VALUE:
	Successs: the final sum of values in the modified histy array.
		- This may be less than the original value of nn, depending on the min and max settings
		- Modified histx and histy arrays, which will hold the histogram values
		- NOTE: values in histx[] represent the middle of each bin, not the start

SAMPLE CALL:
	long ii, g1=1, g2=2, winsize=1000, nintervals, bintot=100, min=-500, max=500;
	long time[1001],group[1001],*intervals=NULL;
	double histx[100],histy[100];
	// time and group data must be stored in memory first
	....
	// find the intervals between events of class g1 and g2
	intervals= xf_wint1_l(time,group,n,g1,g2,winsize,&nintervals);
	// now generate values for the histogram
	xf-hist1_l(intervals,nintervals,histx,histy,bintot,min,max,3);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long xf_hist1_l(long *data, long nn, double *histx, double *histy, long bintot, long min, long max, int format) {

	int outofrange=0;
	long ii,jj,kk,mm;
	long range,*tempy=NULL;
	double aa,binwidth,plotedge,inv_binwidth,histmax;

	/* CALCULATE BIN-WIDTH and INVERSE-BINWIDTH */
	range= max-min;
	binwidth= range/(double)bintot;
	plotedge= (binwidth/2.0)+min; // used to transform bin-number to mid-bin value for output

	/* ALLOCATE TEMPORARY MEMORY FOR COUNTS */
	tempy=(long *)realloc(tempy,(bintot*sizeof(long)));
	if(tempy==NULL) return(-1);
	for(ii=0;ii<bintot;ii++) tempy[ii]=0;

	/* FILL X-VALUES FOR HISTOGRAM BASED ON BINWIDTH */
	for(ii=0;ii<bintot;ii++) histx[ii]=(double)ii*binwidth+plotedge;

	/* FILL BINS WITH COUNT VALUES - SKIP OUT-OF-RANGE VALUES */
	outofrange=0;
	for(ii=0;ii<nn;ii++) {
		if(data[ii]>=min && data[ii]<=max) {
			jj= (long)( (data[ii]-min) / binwidth );
			if(jj==bintot) jj--;
			tempy[jj]++;
		}
		else outofrange++;
	}

	/* CALCULATE NUMBER OF HISTOGRAM VALUES - DEDUCT OUT-OF-RANGE VALUES */
	mm= nn-outofrange;

	/* OPTION 1: COUNTS (JUST CONVERT TO DOUBLE) */
	if(format==1) {
		for(ii=0;ii<bintot;ii++) histy[ii]=(double)tempy[ii];
	}
	/* OPTION 2: 0-1 RANGE (MAX=1.0) */
	if(format==2) {
		kk=tempy[0];
		for(ii=0;ii<bintot;ii++) if(tempy[ii]>kk) kk=tempy[ii];
		histmax=(double)kk;
		for(ii=0;ii<bintot;ii++) histy[ii]=(double)tempy[ii]/histmax;
	}
	/* OPTION 3: PROBABILITY (SUM=1.0) */
	if(format==3) {
		aa=(double)mm;
		for(ii=0;ii<bintot;ii++) histy[ii]=(double)tempy[ii]/aa;
	}

	free(tempy);
	return(mm); /* number of in-range values */
}
