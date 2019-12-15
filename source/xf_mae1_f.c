/*
<TAGS>math stats signal_processing</TAGS>

DESCRIPTION:
	Calculate mean absolute error (MAE) between two series, corrected by the range of series-1
	Less sensitive to ourliers than RMSE
	https://en.wikipedia.org/wiki/Mean_absolute_error
USES:
	Estimate the goodness of fit between two lines
DEPENDENCIES:
	None
ARGUMENTS:
	float *data1 : input array representing values for line #1
	float *data2 : input array representing values for line #2
	long nn : number of elements in each array
	double *result : pre-allocated array to hold results - must allow at least 6 elements
	char *message : pre-allocated array to hold error message
RETURN VALUE:
	MAE on success, INF on horizontal line for NAN on error
	char array will hold message (if any)
SAMPLE CALL:
	mae= xf_mae1_d(data1,data2,nn,message);
	if(!isfinite(mae)) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double xf_mae1_f(float *data1, float *data2, long nn, char *message) {

	char *thisfunc="xf_mae1_f\0";
	long ii,jj,mm,sumbb,sumcc,precision=1000000000;
	double aa,bb,cc,min,max;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn<1) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,nn); return(NAN); }
	/* CHECK INTEGRITY OF DATA */
	for(ii=0;ii<nn;ii++) { if(isfinite(data1[ii]) && isfinite(data2[ii])) break; }
	if(ii>=nn) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); return(NAN); }

	/* INITIALIZE VALUES - INCLUDING MIN & MAX */
	min= max= data2[ii];
	mm=0;
	sumbb= sumcc= 0;

	/* CALCULATING THE MEAN ABSOLUTE ERROR */
	for(ii=0;ii<nn;ii++) {
		if(isfinite(data1[ii]) && isfinite(data2[ii])) {
			aa= data2[ii];
			/* check for min and max */
			if(aa<min) min= aa;
			if(aa>max) max= aa;
			/* calculate the error (absolute difference) for the current point */
			aa= fabs(aa-data1[ii]);
			/* perform sum using long-integer arithmetic */
			cc= modf(aa,&bb); // get integer (bb) and fractional (cc) parts
			sumbb+= (long)bb;
			sumcc+= (long)(cc*precision);
			/* update running count of valid pairs */
			mm++;
	}}

	// fprintf(stderr,"mm=%ld\n",mm);
	// fprintf(stderr,"sumbb=%ld\n",sumbb);
	// fprintf(stderr,"sumcc=%ld\n",sumcc);

	/* reconstruct the sum of the errors */
	aa= (double)sumbb + ((double)sumcc/(double)precision);
	/* get the mean absolute error */
	bb= aa/(double)mm;
	/* return the MAE, corrected by the data range range */
	return( bb/(max-min) );
}
