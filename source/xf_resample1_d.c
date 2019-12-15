/*
<TAGS>signal_processing</TAGS>
[JRH] 20 Jan 2013

DESCRIPTION:
	Resample a series of numbers so there are a user-defined number of elements [setn]
	If actual n < setn, data is split as required
	If acctual n> setn, data is binned and averaged accordingly

	Alters input array
	NAN values will be ignored, but INF will affect the results

USES:
	- binning an array of number
	- modifying different series of data so they are all the same length
	- making an irregular matrix of data have rowa all the same length

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data: input, array of numbers to be resampled
	long n: input, number of elements in data
	long setn: input, the new number of elements requested for data

RETURN VALUE:
	Pointer to modified data array - NULL if n < setn and memory allocation error occurs

SAMPLE CALL:

TO DO:
	- consider using a Gaussian kernal for resampling
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double *xf_resample1_d(double *data , long n, long setn) {

	int leftover_data,sizeofdouble=sizeof(double);
	long i,j,k,n2,tot;
	float binsize=(float)n/(float)setn,limit;
	double sum;

	/* IF SETN < N, AVERAGE THE DATA IN NON-OVERLAPPING BINS */
	if(setn<n) {

		tot=n2=0;
		sum=0.0;
		limit=(float)(((n2+1.0)*binsize)-1.0);

		for(i=0;i<n;i++) {
			leftover_data=1;	// by default assume there is leftover data at the end of the loop
			if((float)i>limit) {
				if(tot==0) sum=NAN;
				leftover_data=0;		// flag potential presence of leftover data
				data[n2]=sum/tot; 		// overwrite previous data with binned average
				sum=0.0; 				// reset the running sum
				tot=0;  				// reset the count within the window
				n2++;					// new length of data array
				limit=(float)(((n2+1.0)*binsize)-1.0); // readjust limit
			}
			if(!isnan(data[i])) {sum+=data[i];tot++;} // build running sum and total data-points
		}
		/* IF THERE WAS LEFTOVER DATA, OUTPUT THE AVERAGE */
		if(leftover_data==1) {
			if(tot==0) sum=NAN;
			data[n2++]=sum/tot;
		}
	}

	/* IF SETN > N, EXPAND THE DATA */
	else if(setn>n) {
		data=(double *)realloc(data,(setn+1)*sizeof(double));
		if(data!=NULL) {
			for(i=(setn-1);i>=0;i--) data[i]=data[((int)((float)i*binsize))];
		}
	}

	return (data);
}
