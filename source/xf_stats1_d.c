/*

<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate summary statistics on an array of numbers (double-precision floating-point)
	This is the fastest of the stats family of functions - mean only is calculated
	Option to use high-precision calculation for large datasets (setlarge)
		- here the mean is first calculated by breaking the double-values into integer and fractional parts
		- then the individual dibberences from the  mean are calculated
		- the sums used for calculating the standard deviation are also similarly adjusted
	NOTE: no check for invalid values (NAN or INF)
USES:
	Getting the mean of a data set
DEPENDENCY TRcc:
	No dependencies
ARGUMENTS:
	double *data1: array holding the data
	long nn: number of elements in the array
	int large: a flag specifying the method for dealing with large datasets
		1 = traditional fast computational formula
		2 = two-pass method, slower but not prone to overflow errors with very large datasets
	double *result_d: array to hold result of calculations
RETURN VALUE:
	zero
SAMPLE CALL:
	xf_stats1_d(data,n,1,result_d); mean=result_d[0];
*/

#include <math.h>
double xf_stats1_d(double *data1, long nn, int digits) {

	long ii,jj,kk,sumjj,sumkk,base=10,precision;
	double aa,bb,cc,mean;

	if(nn<1) return(NAN);
	if(digits<0) return(NAN);
	if(nn==1) return(data1[0]);

	/* CALCULATE PRECISION MULTIPLIER - POWER OF 10 (BASE^DIGITS) */
	precision=1;
	for (;;) {
		if (digits & 1) precision *= base;
		digits >>= 1;
		if (!digits) break;
		base *= base;
	}

	/* CALCULATE THE MEAN BY BREAKING THE NUMBERS INTO INTEGER AND FRACTIONAL PARTS */
	sumjj= sumkk= 0;
	for(ii=0;ii<nn;ii++) {
		aa= data1[ii];
		bb= modf(aa,&cc); /* get fractional (bb) and integer (cc) parts */
		jj= (long)cc; /* integer part */
		kk= (long)bb*precision; /* fractional part, multiplied by precision to a long integer */
		sumjj+= jj;
		sumkk+= kk;
	}
	/* reconstitute & return an accurate estimate of the mean, to 10 decimal places */
	mean= (double)((sumjj + sumkk/precision)/nn);
	return(mean);
}
