/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Perform linear interpolation across invalid data points (double)
	- user specifies the invalid value to interpolate across
	- modifies the original array
	- invalid numbers at the start/end of the input can be filled with first/last valid datum, respectively

USES:
	fill in gaps in an array of data

DEPENDENCIES: none

ARGUMENTS:
	double *data  : input, array
	long ndata    : input, length of input array
	short invalid : invalid value for interpolation (typically FLOAT_MIN or FLOAT_MAX)
	int setfill   : edge-fill invalid values with nearest valid (0=none,1=start,2=end,3=both
	result[3]     : result indices
		result[0] = index to first good value (will be 0 if startsetfill is set)
		result[1] = index to last good value (will be ndata-1 if endsetfill is set)
		result[2] = total interpolated points

RETURN VALUE:
	0: success
	1: fail - no good data

SAMPLE CALL:

*/

#include <math.h>
#include <stdio.h>

int xf_interp4_d(double *data, long ndata, float invalid, int setfill, long *result) {

	long ii,jj,kk,count=0;
	double step;

	/* return if ndata is non-sensical */
	if(ndata<=0) return(1);
	/* find first valid datum */
	for(ii=0;ii<ndata;ii++) if((data[ii])!=invalid) break;
	/* register the value and index of the first (and possibly last) good data point  */
	result[0]=result[1]=ii;
	/* return status=1 if no valid data found */
	if(ii==ndata) return(1);
	/* fill the start of the array with the first valid datum */
	if(setfill==1 || setfill==3) { for(jj=0;jj<ii;jj++) { data[jj]=data[ii]; count++; }}
	else jj= ii; /* either way, jj is initialized to ii - the first valid data point */

	for(ii=ii;ii<ndata;ii++) {
		/* if data is good, keep an index to it (jj) for interpolation */
		if(data[ii]!=invalid) { jj=ii; continue; }
		else {
			/* find next good value if there is one */
			while(data[ii]==invalid && ii<ndata) ii++;
			/* only continue if the end of the array has not been reached */
			if(ii<ndata) {
				count+=((ii-jj)-1);
				/* calculate the amount to increment data during interpolation */
				step= (data[ii]-data[jj])/(double)(ii-jj);
				/* interpolate, setfill the values between the invalid bookends (ii & jj) */
				for(kk=jj+1;kk<ii;kk++) data[kk]= data[kk-1]+step;
				/* make sure jj is reset to the current point, because at the end of this loop ii++ */
				jj=ii;
			}
		}
	}
	/* fill the end of the array with the last valid datum */
	if(jj<(ndata-1) && (setfill==2 || setfill==3)) { for(ii=jj+1;ii<ndata;ii++) { data[ii]=data[jj]; count++; }}

	/* determine the last good datum in the input array */
	result[1]= jj;
	/* save the total number of good values */
	result[2]=count;

	return(0);
}
