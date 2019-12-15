/*
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

	long i,j,k,count=0;
	float step;

	/* fill beginning of array with first valid datum */
	for(i=0;i<ndata;i++) if(isfinite(data[i])) break;
	/* return if no valid data found */
	if(i==ndata) return(-1); 
	/* back-fill if first valid datum is not data[0] */
	if(i>0) for(j=0;j<i;j++) { data[j]=data[i]; count++; } 

	for(i=i;i<ndata;i++) {
		/* if data is good, keep an index to it for interpolation */
		if(isfinite(data[i])) { j=i; continue; } 
		else {
			/* find next good value if there is one */
			while(!isfinite(data[i]) && i<ndata) i++; 
			/* only continue if the end of the aray has not been reached */
			if(i<ndata) { 
				count+=((i-j)-1);
				/* caluculate the amount to incriment data during interpolation */
				step=(data[i]-data[j])/(float)(i-j);
				/* interpolate, fill the values between the invalid bookends (i & j) */
				for(k=j+1;k<i;k++) data[k]=data[k-1]+step; 
				/* make sure j is reset to the current point, because at the end of this loop i++ */
				j=i;
			}
		}
	}
	
	/* fill the end of the array with the last valid datum */
	if(j<(ndata-1)) for(i=j+1;i<ndata;i++) { data[i]=data[j]; count++; }
	
	return(count);
}
