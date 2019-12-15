/***********************************************************************************************
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Smooth a double-precision float array of size "arraysize" using a Gaussian kernel
	New version (May 13 2011) will ignore NaN and Inf values in input array (just passes them back)
ARGUMENTS:
	float *original	: pointer to data to be smoothed (memory must be pre-allocated)
	int arraysize   : number of elements in original array
	int smooth      : half-size of smoothing window - full size = 2*smooth + 1
	char message    : string to hold messages upon return

New version - May 13 2011: will not adjust NaN or Inf entries
New version - Nov 6 2012:  eliminate error reporting in-function and thisfunc variable
************************************************************************************************/
#include<stdlib.h>
#include <math.h>

int xf_smoothgaussd(double *original,long arraysize,int smooth) {

	if(smooth<1) return(-1);
	long int i,j,k,start,end,winwidth;
	double wtsum=0.0,std=(float)smooth,*wt=NULL,*tempd1=NULL;
	double aa,bb;

	/* calculate size of smoothing window */
	winwidth=smooth+smooth+1;

	/* initialise Gaussian kernel and temporary array */
	wt= (double *) malloc((winwidth+1)*sizeof(double));
	tempd1= (double *) malloc((arraysize+1)*sizeof(double));
	if(wt==NULL || tempd1==NULL) return(-1);

	// create Gaussian kernel
	k=0; wtsum=0.0;
	for(i=-smooth;i<=smooth;i++) {
		wt[k] = (double)exp(-i*i/std/2.);	// calculate weight for each bin of the kernel
		wtsum += wt[k];	// calculate sum of weights
		k++;			// count the number of bins (should be winwidth at the end)
	}
	for(i=0;i<winwidth;i++) wt[i]/=wtsum;	// normalise so sum of weights = 1.0

	/* store smoothed array in temporary array - avoids shifting data forward as the kernel is applied */
	for(i=0;i<arraysize;i++) {
		start= i-smooth; 	// pre-define start of local smoothing window - saves time
		end= i+smooth;	// pre-define end of local smoothing window - saves time
		aa=bb=0.0; 			// initialize counters for sum of weighted values and sum of weights
		k=-1;							// keep track of position in smoothing window
		for(j=start;j<=end;j++) {		// loop: add up weighted values around reference point
			k++;
			if(j>=0&&j<arraysize && isfinite(original[j]) ) {
				aa += original[j]*wt[k];	// summed weighted values contributing to original[i]
				bb += wt[k];				// summed weights for this smoothing window - ideally 1.0 at end
			}
		}
		if(bb>0.0) tempd1[i] = aa/bb; // if any weights were added, new value = weighted mean corrected by sum of weights - to account for missing bins
		else tempd1[i] = original[i];
	}
	for(i=0;i<arraysize;i++) original[i] = tempd1[i]; /* copy smoothed array back to original */

	free(wt); free(tempd1);
	return(0);
}
