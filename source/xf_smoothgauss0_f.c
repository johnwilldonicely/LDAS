/***********************************************************************************************
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Smooth a float array of size "arraysize" using a Gaussian kernel
	- will ignore NaN and Inf values in input array (just passes them back)
	- this version only modifies a single sample in the array

ARGUMENTS:
	float *original   : pointer to data to be smoothed (memory must be pre-allocated)
	size_t arraysize  : number of elements in original array
	size_t index      : index to the sample to be smoothed
	int smooth        : half-size of smoothing window - full size = 2*smooth + 1
	double *result    : address of variable to hold the result (passed as &result)
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_smoothgauss0_f(float *original,size_t arraysize,size_t index,int smooth,float *result) {

	long i,j,k,winwidth;
	size_t ii,jj,kk,start,end;
	double wtsum,*wt=NULL;
	double aa,bb;

	/* make sure index s in-range */
	if(index<0||index>=arraysize) return(-1);
	/* if no smoothing was requested, result is original value */
	if(smooth<1) { *result= original[index]; return(0); }
	/* calculate size of smoothing window */
	winwidth = smooth+smooth+1;
	/* initialise Gaussian kernel */
	wt= malloc((winwidth+1)*sizeof(*wt)); if(wt==NULL) return(-2);
	/* create Gaussian kernel */
	k=0; wtsum=0.0;
	for(i=-smooth;i<=smooth;i++) wt[k++]= (double)exp(-i*i/smooth/2.0);
	/* pre-define start of local smoothing window - set kk (start position in Gaussian kernel) */
	if(index>=smooth) { start= index-smooth; kk= 0; }
	else { start= 0; kk= smooth-index ; }
	/* pre-define end of local smoothing window - saves time */
	if(index<(arraysize-smooth)) end= index+smooth;
	else end= arraysize-1;

	/* APPLY THE GAUSSIAN KERNEL  */
	aa=bb=0.0; 					// initialize counters for sum of weighted values and sum of weights
	for(jj=start;jj<=end;jj++) {			// loop: add up weighted values around reference point
		if(isfinite(original[jj]) ) {
			aa+= original[jj]*wt[kk];	// summed weighted values contributing to original[i]
			bb+= wt[kk];			// summed weights for this smoothing window - ideally 1.0 at end
		}
		kk++;
	}
	if(bb>0.0) *result= (float)(aa/bb);  // if any weights were added, new value = weighted mean corrected by sum of weights - to account for missing bins
	else *result= original[index];       // otherwise use the original value

	/* free memory and return */
	free(wt);
	return(0);
}
