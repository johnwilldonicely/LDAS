/*
<TAGS>math</TAGS>
DESCRIPTION:
	- Calculate the minimum & maximum windowed-slope in a series

USES:
	- Charactorization of evoked responses, synaptic responses, etc

DEPENDENCIES:
	- No dependencies

ARGUMENTS:
	float *data     : input holding data
	long n1         : size of data array
	long winsize    : size of window (samples) across-which to estimate slope
	double interval : units spanned by each sample, = 1/samplerate
	int test	: test to apply, to ignore a window or break
		0= none
		-1= ignore negative slopes
		1= ignore positive slopes
		-2= stop seeking on first positive slope
		2= stop seeking on first positive slope
		-3= stop seeking when slope starts getting more negative
		3= stop seeking when slope starts getting more positive
	double min	: the smallest (most negative) slope
	double max      : the largest (most positive) slope
	char *message : arrray to hold message in the event of an error

RETURN VALUE:
	success: number of slope estimates taken
	fail: -1, message[] will hold information
	min and max will be updated
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

long xf_geom_slope2_f(float *data,long n1,long winsize,double interval,int test,double *min,double *max,char *message) {

	char *thisfunc="xf_geom_slope2_f\0";
	long ii,jj,kk,n2;
	double aa,bb,cc,tempmin,tempmax,winsize2,slope,prevslope=0.0,delta;

	tempmin= NAN;
	tempmax= NAN;

	/* make sure at least one window is possible */
	if(winsize>n1) { sprintf(message,"%s [WARNING]: window-size (%ld) is too large for data length (%ld)",thisfunc,winsize,n1); return(-1); }

	/* decrement winsize to reflect x-distance spanned and accurately define the end-sample of the window  */
	winsize--;
	/* convert x-distance to actual units */
	winsize2= winsize*interval;
	/* set min and max to possible extremes */
	tempmin= DBL_MAX;
	tempmax= DBL_MIN;
	/* initialize the window-counter */
	n2= 0;

	/* NOW CRAWL THROUGH THE DATA TO FIND THE MIN AND MAX SLOPE */
	for(ii=0;ii<n1;ii++) {
		/* define last sample in window - remember winsize was decremented */
		jj= ii+winsize;
		/* make sure we are still within the data bounds */
		if(jj>=n1) break;
		aa= data[ii];
		bb= data[jj];
		/* if data is good, check min/max and increment window-counter (n2) */
		if(isfinite(aa) && isfinite(bb)) {
			slope= (bb-aa) / winsize2;
			//TEST:	printf("%g	%g\n",aa,slope); continue;
			/* check if slope passes test conditions */
			if(test!=0) {
				if(test==-1 && slope<0) continue;
				else if(test==1 && slope>0) continue;
				else if(test==-2 && slope<0) break;
				else if(test==2 && slope>0) break;
				/* otherwise, test for change in slope */
				else {
					/* only test if we're on to the second window */
					if(n2>0) {
						/* break if slope is becomming more negative (-3) or more positive (+3)  */
						delta= slope-prevslope;
						if(test==-3 && delta<0.0) break;
						if(test==3 && delta>0.0) break;
					}
					/* save current slope as previous */
					prevslope= slope;
				}
			}
			/* check if slope is the minimum or maximum */
			if(slope>tempmax) tempmax= slope;
			if(slope<tempmin) tempmin= slope;
			n2++;
		}
	}

	/* if window size is ok but no good aa/bb pairs were found... */
	if(n2==0) { sprintf(message,"%s [WARNING]: no valid sample-pairs for slope calculation",thisfunc); return(-1); }

	/* assign values to min and max and return the number of estimates used */
	*min= tempmin;
	*max= tempmax;
	return(n2);
}
