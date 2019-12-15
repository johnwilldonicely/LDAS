/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:

	Apply a notch (band-stop) filter to an array of numbers
		- this version does not modify original input array
		- consequently, it requires pre-allocation of memory for swap and output

	Makes two passes at the data (forward & reverse), to avoid time-shifting

	NOTE : interpolation should be applied first if needed to remove NANs or INFs
	NOTE : padding  & de-meaning the array can help remove effects from large
	       deflections at either edge of the data and data offset from zero

	Coefficient calculations based on principals in chapter 6 of
	Introductory Digital Signal Processing with Computer Applications (2nd Edition, 1994)
	By Paul A. Lynn and Wolfgang Fuerst

USES:
	Removal of 50 or 60 Hz mains noise from recorded signals

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *X:           array holding input to be filtered, fixed sample rate is assumed
	float *Y:           array reserved for swapping values, memory must be pre-allocated by calling function
	float *Z:           array reserved for filtered output, memory must be pre-allocated by calling function
	size_t nn:          length of X and Z arrays
	float sample_freq:  sample frequency (samples per second)
	float notch_freq:     the frequency to by cut (Hz)
	float notch_width:    width of the stop-band (Hz)
	char message[]:	    message indicating success or reason for failure

RETURN VALUE:
	0 on success, -1 on fail

*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int xf_filter_notch2_f(float *X, float *Y, float *Z, size_t nn, float sample_freq, float notch_freq, float notch_width, char *message) {

	char *thisfunc="xf_filter_notch2_f\0";
	size_t ii;
	double s,p,r,omega,a1,b1,b2;

	sprintf(message,"%s (incomplete))",thisfunc);

	if(nn<4) {
		sprintf(message,"%s (no filtering - number of input samples is less then 4)",thisfunc);
		return(-1);
	}
	if(notch_freq >= (sample_freq/2.0)) {
		sprintf(message,"%s (notch frequency %g is larger than 1/2 the sample frequency %g)",thisfunc,notch_freq,sample_freq);
		return(-1);
	}


	/* DEFINE THE PROPORTION OF THE NYQUIST RANGE REPRESENTED BY THE NOTCH FREQUENCY */
	omega= (double)notch_freq / (sample_freq/2.0);
	/* DEFINE RADIUS OF THE COMPLEX POLE PAIR */
	r= 1.0 - ( M_PI / (double)(sample_freq/notch_width) );
	/* DEFINE COEFFICIENTS : a= non-recursive, b=recursive */
	a1= -2.0 * cos(omega*M_PI);
	b1= -2.0 * r * cos(omega*M_PI);
	b2= r * r * exp(omega*M_PI) * exp(-omega*M_PI);


	/* FORWARD FILTER, COPYING DATA TO ARRAY Y */
	/* initialize: using all coefficients for the non-recursive terms helps reduce edge effects if data is offset from zero */
	ii=0; Y[ii] = X[ii] + a1*X[ii] + X[ii];
	ii=1; Y[ii] = X[ii] + a1*X[ii-1] + X[ii-1] - b1*Y[ii-1];
	for(ii=2;ii<nn;ii++) Y[ii] = X[ii] + a1*X[ii-1] + X[ii-2] - b1*Y[ii-1] - b2*Y[ii-2];

	/* BACKWARD FILTER TO REMOVE PHASE-SHIFT, AND COPY DATA TO OUTPUT ARRAY Z */
	/* initialize: using all coefficients for the non-recursive terms helps reduce edge effects if data is offset from zero */
	ii=nn-1; X[ii] = Y[ii] + a1*Y[ii] + Y[ii];
	ii=nn-2; X[ii] = Y[ii] + a1*Y[ii+1] + Y[ii+1] - b1*X[ii+1];
	while(ii-- >0) { /* ii is size_t, so if it becomes negative any test for i<0 will fail */
		X[ii] = Y[ii] + a1*Y[ii+1] + Y[ii+2] - b1*X[ii+1] - b2*X[ii+2];
	}

	/* WRAP UP */
	sprintf(message,"%s (successfully filtered %ld points)",thisfunc,nn);
	return(0);

}
