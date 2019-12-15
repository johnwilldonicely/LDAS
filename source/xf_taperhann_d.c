/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	Create a Hann taper window function (array)

USES:
	Tapering for use with FFT

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long setn     : size of taper window (number of elements, must be >=3)
	int setnorm   : normalize taper to the mean:
				0=NO (traditional, applying taper reduces amplitude by half)
				1=YES (used by LDAS, preserves amplitude)
	float setpow  : power to raise the taper
				1 is standard
				higher values increase slope
				0 sets all values to "1" - i.e. no taper
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	- pointer to taper 0 on success, NULL on error
	- char array will hold message (if any)
	- calling function must free memory for taper

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double *xf_taperhann_d(long setn, int setnorm, float setpow, char *message) {

	char *thisfunc="xf_taperhann_d\0";
	long ii,nmin=3;
	double dd,sum,mean,*taper=NULL;

	/* check validity of arguments */
	if(setn<nmin) { sprintf(message,"%s [ERROR]: set taper length (%ld) is less than minimum (%ld)",thisfunc,setn,nmin); return(NULL); }
	if(setnorm!=0&&setnorm!=1) { sprintf(message,"%s [ERROR]: mean-normalization (%d) must be set to 0 or 1",thisfunc,setnorm); return(NULL); }

	/* allocate memory for the taper - must be freed by calling function */
	taper= (double*)calloc(setn,sizeof(double));
	if(taper==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }

	/* calculate the taper, and the sum in case normalization to the mean is required */
	/* raising to a power increases the slope (effectively the order) of the taper */
	dd= (2.0*M_PI)/(setn-1.0);
	sum=mean=0.0;
	for(ii=0;ii<setn;ii++) {
		taper[ii] = pow( 0.5*(1.-cosf(ii*dd)) , setpow );
		sum+=taper[ii];
	}

	/* normalize to the mean preserves amplitude while pushing down the tails of the taper */
	if(setnorm==1) {
		mean= sum/(double)setn;
		for(ii=0;ii<setn;ii++) taper[ii] /= mean;
	}

	return(taper);
}
