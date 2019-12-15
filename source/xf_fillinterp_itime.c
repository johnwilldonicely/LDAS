/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:

	Fill target array (valB) with interpolated values from source array (valA),
	based on where the target array times (timeB) fall relative to the source array (timeA)

	Invalid data points only assigned to B where it is out of range of A (beginning & end)

	This function is like hux_fillinterp except
		- times are unsigned long integers
		- a maximum number of invalid samples across which to interpolate is defined
		- totA and totB are long integers
		- invalid is a float

	This function is NOT appropriate for data with large number of invalid samples (eg. theta phase)
	- in this case, use similar routine, hux_fillprev instead


USES:
	Allows mapping of data from the source array onto a different time-series of samples
	E.g. assign interpolated position values to a series of neuronal action potential times

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long unsigned int *timeA	: input timestamps of the source array
	long unsigned int *timeB	: input time-stamps of the target array
	float *valA	: input data values, interpolated versions of which are sent to valB
	float *valB	: output data values
	int totA		: total number of values in the source array
	int totB		: total number of values in the target array
	int invalid 	: value to treat as invalid (typically -1)
	int max_invalid : maximum permissible number of invalid points to allow interpolation accross
	char *type  	: data type, "linear" or "circular" (0-360)

RETURN VALUE:
	The number of interpolated values generated (excluding invalid samples at start and end)

 */

# include <string.h>

int xf_fillinterp_itime (long unsigned int *timeA,long unsigned int *timeB,float *valA,float *valB,long totA,long totB,float invalid,int max_invalid,char *type) {

	long indexA,indexB,first,second,count,n_invalid;
	int circular;
	float a;

	indexA=indexB=count=0;
	if(strcmp(type,"circular")==0) circular=1;
	else circular=0;

	/* DETERMINE FIRST VALID SOURCE RECORD */
	while(valA[indexA]==invalid && indexA<totA) indexA++;
	/* Invalidate all B-records before (and up to exactly the same time as) the first valid A-record */
	/* Avoiding equality of time[indexA] and time[indexB] is important to avoid potential division-by-zero problems below */
	/* Put another way, all B-records must have A-records before AND after them for interpolation to proceed */
	if(timeB[0] <= timeA[indexA]) {
		while(timeB[indexB] <= timeA[indexA] && indexB<totB) {
			valB[indexB] = (float) invalid;
			indexB++;
	}}

	/* FIND VALID A-RECORDS BOUNDING EACH B-RECORD - START INDEXA WITH FIRST VALID A */
	n_invalid=0; first=second=indexA;
	for(indexB=indexB;indexB<totB;indexB++) {
		/* find valid source sample indexA with bigger timestamp than target*/
		while(timeA[second] < timeB[indexB] || valA[second] == invalid) {
			if(valA[second]==invalid) n_invalid++;
			else {
				first=second;
				n_invalid=0;
			}
			second++;
			if(second>=totA) { for(indexB=indexB;indexB<totB;indexB++) valB[indexB] =  invalid; return(count);	}
		}
		if(n_invalid<=max_invalid) {
			/* calc. proportion of inter-source-sample interval elapsed since last source sample*/
			a = ( (timeB[indexB]-timeA[first]) / (double)(timeA[second]-timeA[first]) );
			/* use this time proportion to calculate proportions of other variables elapsed */
			valB[indexB] = valA[first] + ((valA[second]-valA[first]) * a);
			count++;
			/* determine corrected proportion of heading change at target record time */
			if(circular==1 && valB[indexB]!=invalid) {
				if(valB[indexB]<0) valB[indexB] = 360 + valB[indexB];
				if(valB[indexB]>359.999999) valB[indexB] = 360 - valB[indexB];
			}
		}
		else valB[indexB] = (float)invalid;;
	}

	return(count);
}
