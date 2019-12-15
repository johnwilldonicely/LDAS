/*
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Perform linear interpolation across invalid data points
	- modifies the original array
	- expects NAN or INF to be used to designate invalid values
	- blocks of invalid numbers at top and bottom of record are filled with first and last valid datum, respectively

USES:
	Fill in gaps in an array of data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data : array already holding the data
	long ndata : number of elements in the data array

RETURN VALUE:
	total points interpolated across
	-1: no valid data found

SAMPLE CALL:
*/

#include <math.h>
#include <stdio.h>

long xf_interp3_f(float *data, long ndata) {

	long ii,jj,kk,count=0;
	double step;

	/* return if ndata is non-sensical */
	if(ndata<=0) return(-1);
	/* fill beginning of array with first valid datum */
	for(ii=0;ii<ndata;ii++) if(isfinite(data[ii])) break;
	/* return if no valid data found */
	if(ii==ndata) return(-1);
	/* back-fill if first valid datum is not data[0] */
	for(jj=0;jj<ii;jj++) { data[jj]=data[ii]; count++; }

	for(ii=ii;ii<ndata;ii++) {
		/* if data is good, keep an index to it for interpolation */
		if(isfinite(data[ii])) { jj=ii; continue; }
		else {
			/* find next good value if there is one */
			while(!isfinite(data[ii]) && ii<ndata) ii++;
			/* only continue if the end of the aray has not been reached */
			if(ii<ndata) {
				count+= ((ii-jj)-1);
				/* caluculate the amount to incriment data during interpolation */
				step= ((double)data[ii]-(double)data[jj])/(double)(ii-jj);
				/* interpolate, fill the values between the invalid bookends (ii & jj) */
				for(kk=jj+1;kk<ii;kk++) data[kk]= (float)((double)data[kk-1]+step);
				/* make sure j is reset to the current point, because at the end of this loop ii++ */
				jj=ii;
			}
		}
	}

	/* fill the end of the array with the last valid datum */
	if(jj<(ndata-1)) for(ii=jj+1;ii<ndata;ii++) { data[ii]=data[jj]; count++; }

	return(count);
}
