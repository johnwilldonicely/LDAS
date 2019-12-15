/*
<TAGS>stats signal_processing</TAGS>
DESCRIPTION:

	Calculate the amplitude of a frequency component in a signal, using the Goertzel algorithm
	- this is essentially a DFT for the specified frequency
	- real-valued input is assumed
	- a window slides along the data one sample at a time
	- a Hann taper is applied to each window
	- performed as if zero-padding were applied to each end of the input
		- note that no padding is actually required

	http://en.wikipedia.org/wiki/Goertzel_algorithm

	Note: data should be interpolated to remove non-numberic values, NAN or INF

	Note: that rather than the magnitude of the signal, this function calculates the amplitude
		- hence the results reflect the original amplitude of the input signal
		- each power magnitude is adjusted by (0.5/nwin) and the sqare-root is taken



USES:
	- Fast detection of the energy envelope of a signal for phase-amplitude coupling
	- Detection of a frequency in a signal


DEPENDENCY TREE:
	No dependencies


ARGUMENTS:
	float *input :       data array to be filtered, fixed sample rate is assumed
	float *power :       data array to hold results (calling function must reserve memory)
	size_t nn :          size of input and power arrays (number of samples)
	float sample_freq :  sample frequency (Hz)
	float freq :         frequency of interest (Hz)
	size_t nwin :        size of the window used to integrate (typically 5*sample_freq/freq - i.e. 5 wavelengths)
	char *message :      message indicating success or reason for failure


RETURN VALUE:
	success: 0
	failure: -1

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int xf_power_goertzel1_f(float *input, float *power, size_t nn, float sample_freq, float freq, size_t nwin, char *message) {

	char *thisfunc="xf_power_goertzel1_d";
	long x,y,z;
	size_t ii,jj,kk,ll,mm,stop,halfwin;
	double *taper,aa,bb,cc,p1,p2,coef,scaling;

	sprintf(message,"%s (success)",thisfunc);

	scaling= 0.5 / (double)nwin;
	halfwin= (size_t)(nwin/2);
	coef= 2.0 * cosf(2.0 * M_PI * ((double)freq/(double)sample_freq) );

	//TEST: for(ii=0;ii<nn;ii++) fprintf(stderr,"%ld	%g	%g\n",ii,input[ii],power[ii]); return(-1);

	if(nn<4) {
		sprintf(message,"xf_power_goertzel1_f (no filtering - number of input samples is less than 4)");
		return(-1);
	}

	/* CREATE A HANN TAPER */
	if((taper=(double *)malloc((nwin)*sizeof(double)))==NULL) {
		sprintf(message,"xf_power_goertzel1_f (memory allocation problem)");
		return(-1);
	}
	aa=(2.0*M_PI)/(nwin-1.0);
	for(ii=0;ii<nwin;ii++) taper[ii] = 0.5 * (1.-cosf(ii*aa));

	/*
	1. CALCULATE THE POWER FOR THE BEGINNING OF THE ARRAY
	- note that we begin with hypothetically negative positions
	- consequently we cannot use size_t counters for most calculations but long integers instead
	- this should be ok as numbers at the beginning of the file should fall within the range of long integers anyway
	- zero is substituted for values of input[<0]
	*/
	x=(long)nwin*(-1);
	for (x=x; x<0; x++)  { // x= (-nwin) to zero
		p1=p2=0.0;
		z=x+nwin;
		for(kk=0,y=x;y<z;y++) {
			if(y>=0) bb= ((double)input[y]*taper[kk++])+(coef*p1)-p2;
			else bb= (coef*p1)-p2;
			p2= p1;
			p1= bb;
		}
		z=x+halfwin;
		if(z>=0) power[z] = sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
	}
	/*
	2. CALCULATE THE POWER FOR THE MIDDLE OF THE ARRAY
	- note that power is calculated over a window beginning at position ii
	- consequently the result applies to position ii+(nwin/2)
	*/
	stop= (nn-nwin)-1;
	for (ii=0; ii<stop; ii++)  {
		p1=p2=0.0;
		mm=ii+nwin;
		for(kk=0,jj=ii;jj<mm;jj++) {
			bb= ((double)input[jj]*taper[kk++])+(coef*p1)-p2;
			p2= p1;
			p1= bb;
		}
		power[ii+halfwin]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
	}
	/*
	3. CALCULATE THE POWER FOR THE END OF THE ARRAY
	- note that the windows will begin to extend beyond nn
	- zero is substituted for values of input[>=nn]
	*/
	for (ii=stop; ii<nn; ii++)  {
		p1=p2=0.0;
		mm=ii+nwin;
		for(kk=0,jj=ii;jj<mm;jj++) {
			if(jj<nn) bb= ((double)input[jj]*taper[kk++])+(coef*p1)-p2;
			else bb= (coef*p1)-p2;
			p2= p1;
			p1= bb;
		}
		jj=ii+halfwin;
		if(jj<nn) power[jj]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
	}
	free(taper);


	return(0);
}
