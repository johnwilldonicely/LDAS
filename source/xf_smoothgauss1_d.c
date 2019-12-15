/***********************************************************************************************
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Smooth a double float array of size "arraysize" using a Gaussian kernel
	Will ignore NaN and Inf values in input array (just passes them back)
ARGUMENTS:
	float *original	: pointer to data to be smoothed (memory must be pre-allocated)
	size_t arraysize   : number of elements in original array
	int smooth      : half-size of smoothing window - full size = 2*smooth + 1

New version - May 13 2011: will not adjust NaN or Inf entries
New version - Nov 6 2012:  eliminate error reporting in-function and thisfunc variable
New version - June 30 2013: uses size_t for array length
************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_smoothgauss1_d(double *original,size_t arraysize,int smooth) {

	if(smooth<1) return(-1);
	long i,j,k,winwidth;
	size_t ii,jj,kk,start,end;
	double wtsum=0.0,std=(float)smooth,*wt=NULL,*tempd1=NULL;
	double aa,bb;

	/* calculate size of smoothing window */
	winwidth = smooth+smooth+1;

	/* initialise Gaussian kernel and temporary array */
	wt= (double *) malloc((winwidth+1)*sizeof(double));
	tempd1= (double *) malloc((arraysize+1)*sizeof(double));
	if(wt==NULL || tempd1==NULL) return(-1);

	/* create Gaussian kernel */
	k=0; wtsum=0.0;
	for(i=-smooth;i<=smooth;i++) {
		wt[k] = (double)exp(-i*i/std/2.);	// calculate weight for each bin of the kernel
		wtsum += wt[k];	// calculate sum of weights
		k++;			// count the number of bins (should be winwidth at the end)
	}
	for(i=0;i<winwidth;i++) wt[i]/=wtsum;	// normalise so sum of weights = 1.0

	/* store smoothed array in temporary array - avoids shifting data forward as the kernel is applied */
	for(ii=0;ii<arraysize;ii++) {

		// pre-define start of local smoothing window - set kk (start position in Gaussian kernel)
		if(ii>=smooth) { start = ii-smooth; kk=0; }
		else { start=0; kk=smooth-ii ; }
		// pre-define end of local smoothing window - saves time
		if(ii<(arraysize-smooth)) end = ii+smooth;
		else end=arraysize-1;

		aa=bb=0.0; 		// initialize counters for sum of weighted values and sum of weights
		for(jj=start;jj<=end;jj++) {		// loop: add up weighted values around reference point
			if(isfinite(original[jj]) ) {
				aa += original[jj]*wt[kk];	// summed weighted values contributing to original[i]
				bb += wt[kk];				// summed weights for this smoothing window - ideally 1.0 at end
			}
			kk++;
		}
		if(bb>0.0) tempd1[ii] = aa/bb; // if any weights were added, new value = weighted mean corrected by sum of weights - to account for missing bins
		else tempd1[ii] = original[ii];
	}
	for(i=0;i<arraysize;i++) original[i] = tempd1[i]; /* copy smoothed array back to original */

	free(wt);
	free(tempd1);
	return(0);
}
