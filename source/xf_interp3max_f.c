/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Perform conditional linear interpolation across invalid data points
	- this version also allows user to specify the maximum number of values to interpret across
	- modifies the original array
	- expects NAN or INF to be used to designate invalid values
	- blocks of invalid numbers at top and bottom of record are filled with first and last valid datum, respectively

USES:
	Fill in gaps in an array of data, but not gaps which are too large

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data : array already holding the data
	long ndata  : number of elements in the data array
	long max    : maximum number of points to interpolate across, -1 = no limit

RETURN VALUE:
	total points interpolated across
	-1: no valid data found

SAMPLE CALL:


*/

#include <math.h>
#include <stdio.h>
#include <limits.h>

long xf_interp3max_f(float *data, long ndata, long max) {

	long i=0,j=0,k=0,gap,count=0;
	double step;

	/* if max is set to zero, there is no maximum length gap to span */
	if(max<0) max=LONG_MAX;

	/* fill beginning of array with first valid datum */
	for(i=0;i<ndata;i++) if(isfinite(data[i])) break;
	/* return if no valid data found */
	if(i==ndata) return(-1);
	/* back-fill if first valid datum is not data[0] */
	gap=i-1;
	if(i>0 && gap<=max) for(j=0;j<i;j++) { data[j]=data[i]; count++; }

	/* scan the rest of the data  */
	for(i=i;i<ndata;i++) {
		/* if data is good, keep an index to it (j) for interpolation */
		if(isfinite(data[i])) { j=i; continue; }
		else {
			/* find next good value if there is one */
			while(!isfinite(data[i]) && i<ndata) i++;
			/* only continue if the end of the array has not been reached */
			if(i<ndata) {
				gap=(i-j)-1;
				if(gap<=max) {
					count+=gap;
					/* calculate the amount to increment data during interpolation */
					step=(data[i]-data[j])/(float)(i-j);
					/* interpolate, fill the values between the invalid bookends (i & j) */
					for(k=j+1;k<i;k++) data[k]=data[k-1]+step;
				}
				/* make sure j (previous valid point) is reset to the current point */
				j=i;
			}
		}
	}

	/* fill the end of the array with the last valid datum */
	if(j<(ndata-1)) {
		gap= (ndata-j)+1;
		if(gap<=max) for(i=j+1;i<ndata;i++) { data[i]=data[j]; count++; }
	}

	return(count);
}
