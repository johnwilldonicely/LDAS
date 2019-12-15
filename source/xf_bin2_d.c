/*
<TAGS>signal_processing transform</TAGS>
DESCRIPTION:
	Averages data in non-overlapping time-windows
	NAN values do not contribute to the sum
	Alters input array, shortening overall length to (n-windows) worth of average values
	Time for each window corresponds with the beginning of each window
	NAN values will be ignored, but INF will affect the results

USES:
DEPENDENCY TREE:
	No dependencies
ARGUMENTS:
RETURN VALUE:
SAMPLE CALL:
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long int xf_bin2_d(double *time, double *data , long n, double winwidth, int out) {

	int leftover_data,sizeofdouble=sizeof(double);
	long i,j,k,n2,tot;
	double aa,bb,time_start,time_elapsed,sum;

	winwidth-=0.00000001; // reduce size of window slightly - ensures detection of intervals >= winwidth
	tot=n2=0;
	sum=0.0;
	time_start=time[0];
	leftover_data=0;
	aa= 0.0;

	/* BIN DATA */
	if(out==1) { // OUTPUT SUMS
		for(i=0;i<n;i++) {
			aa=time[i]; 		// store current time
			leftover_data=0;	// by default there is no leftover data at the end of the loop
			time_elapsed= aa-time_start;	// store time elapsed since last window
			if(time_elapsed>=winwidth) {
				if(tot==0) sum=NAN;
				leftover_data=1;		// flag potential presence of leftover data
				time[n2]=time_start; 	// overwrite previous time
				data[n2]=sum;			// overwrite previous data with binned sum
				time_start=aa;			// reset start-time
				sum=0.0; 				// reset the running sum
				tot=0;  				// reset the count within the window
				n2++;					// new length of data array
			}
			if(isfinite(data[i])) {sum+=data[i];tot++;} // build running sum and total data-points
	}}
	else if(out==2) { // OUTPUT MEANS
		for(i=0;i<n;i++) {
			aa=time[i]; 		// store current time
			leftover_data=0;	// by default there is no leftover data at the end of the loop
			time_elapsed= aa-time_start;	// store time elapsed since last window
			if(time_elapsed>=winwidth) {
				if(tot==0) sum=NAN;
				leftover_data=1;		// flag potential presence of leftover data
				time[n2]=time_start; 	// overwrite previous time
				data[n2]=sum/tot; 		// overwrite previous data with binned average
				time_start=aa;			// reset start-time
				sum=0.0; 				// reset the running sum
				tot=0;  				// reset the count within the window
				n2++;					// new length of data array
			}
			if(isfinite(data[i])) {sum+=data[i];tot++;} // build running sum and total data-points
	}}
	else return(n); // if an invalid output is chosen, nothing is done

	/* IF THERE WAS LEFTOVER DATA, OUTPUT */
	time_elapsed= aa-time_start;
	if(time_elapsed>0.0 || leftover_data==1) {
		if(tot==0) sum=NAN;
		time[n2]=time_start;
		if(out==1) data[n2]=sum;
		if(out==2) data[n2]=sum/tot;
		n2++;
	}
	return (n2);
}
