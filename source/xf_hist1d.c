/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Build a histogram (double *histx, double *histy) from an array of data data (double *data)

USES:
	Typically used in conjunction with xf_wint1, which can produce the data values necessary for the histogram

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data : array of data data to generate histogram
	int n : size of the array
	double *histx : result-array, x-axis values for histogram - NOTE! must be initialised by calling function
	double *histy : result-array, y-values for histogram - NOTE! must be initialised by calling function
	int bintot : total number of bins in the histogram
	double min : define bottom of the data range - NOTE than min itself will also be included
	double max : define top of the data range - NOTE than max itself will also be included
	int format : the format of the histogram - 1=counts, 2=proportion (peak=1), 3=probability (sum of values=1)

RETURN VALUE:
	A long integer, the final number of data points in the modified histy array.
	This may be less than the original value of n, depending on the min and max settings

	The main result is modification of the histx and histy arrays, which will hold the values
	necessary to generate a histogram plot. Note that the values in the histx array
	will correspond with the middle of each bin, not the start.

SAMPLE CALL:
	int g1=1, g2=2;
	long ii,jj,kk,nn,result_l[32],nintervals;
	float winsize=0.01;
	double time[1001],group[1001],*intervals=NULL;
	// time and group data must be stored in memory first

	// find the intervals between events of class g1 and g2
	intervals= xf_wint1(time,group,n,g1,g2,winsize,result_l);  nintervals=result_l[0];

	// now generate values for the histogram
	xf-histd1(intervals,nintervals,histx,histy,100,-.05,.05,1) // 100ms wide histogram with 100 bins

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long xf_hist1d(double *data, long nn, double *histx, double *histy, int bintot, double min, double max, int format) {

	int outofrange=0;
	long ii,jj,kk;
	double aa,range,binwidth,plotedge,inv_binwidth,histmax;

	/* CALCULATE BIN-WIDTH and INVERSE-BINWIDTH */
	range=max-min;
	binwidth=range/(double)bintot;

	inv_binwidth=1.0/binwidth;
	plotedge=min+binwidth/2.0; // used to transform bin-number to mid-bin value for output


	/* FILL BINS WITH COUNT VALUES - SKIP OUT-OF-RANGE VALUES (POSSIBLE IF SETLOW OR SETHIGH WERE DEFINED) */
	outofrange=0;
	for(ii=0;ii<nn;ii++) {
		if(data[ii]>=min && data[ii]<=max) {
			jj=(int)((data[ii]-min)*inv_binwidth);
			if(jj==bintot) jj--;
			histy[jj]++;
		}
		else outofrange++;
	}
	nn-=outofrange;

	/* FILL X-VALUES FOR HISTOGRAM BASED ON BINWIDTH */
	for(ii=0;ii<bintot;ii++) histx[ii]=(double)ii*binwidth+plotedge;

	/* MODIFY HISTOGRAM Y-VALUES IF FORMAT=2 (PROPORTION) OR 3 (PROBABILITY)*/
	if(format==2) {
		histmax=histy[0]; for(ii=0;ii<bintot;ii++) if(histy[ii]>histmax) histmax=histy[ii];
		aa=1.0/histmax;
		for(ii=0;ii<bintot;ii++) histy[ii]=histy[ii]*aa;
	}
	if(format==3) {
		aa=1.0/(double)(nn);
		for(ii=0;ii<bintot;ii++) histy[ii]=histy[ii]*aa;
	}
	return(nn); /* modified version of n */
}
