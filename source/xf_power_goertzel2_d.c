/*
<TAGS>stats signal_processing</TAGS>
DESCRIPTION:
	Calculate the amplitude of a frequency component in a signal, using the Goertzel algorithm
	Output a power estimate for every input data point
	- this is essentially a DFT for the specified frequency
	- real-valued input is assumed
	- data should be interpolated to remove non-numberic values, NAN or INF
	- performed as if zero-padding were applied to each end of the input
	- note that no padding is actually required

	http://en.wikipedia.org/wiki/Goertzel_algorithm

	Note that rather than the magnitude of the signal, this function calculates the amplitude
		- hence the results reflect the original amplitude of the input signal
		- each power magnitude is adjusted by (2.0/nwin) and the square-root is taken


USES:
	- Fast detection of the energy envelope of a signal for phase-amplitude coupling
	- Detection of a frequency in a signal


DEPENDENCY TREE:
	No dependencies


ARGUMENTS:
	double *input :      data array to be filtered, fixed sample rate is assumed
	double *power :      data array to hold results (calling function must reserve memory)
	size_t nn :          size of input and power arrays (number of samples)
	double sample_freq : sample frequency (Hz)
	double freq :        frequency of interest (Hz)
	size_t nwin :        size of the window used to integrate (typically 5/freq - i.e. 5 wavelengths)
	int dotaper :        apply a Hann taper (0=no, 1=yes)
	char *message :      message indicating success or reason for failure


RETURN VALUE:
	success: 0
	failure: -1

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int xf_power_goertzel2_d(double *input, double *power, size_t nn, double sample_freq, double freq, size_t nwin, int dotaper, char *message) {

	char thisfunc="xf_power_goertzel2_d";
	long x,y,z;
	size_t ii,jj,kk,ll,mm,stop,halfwin;
	double *taper,aa,bb,cc,p1,p2,coef,scaling;

	sprintf(message,"%s (success)",thisfunc);

	scaling= 2.0 / (double)nwin;
	halfwin= (size_t)(nwin/2);
	coef= 2.0 * cosf(2.0 * M_PI * (freq/sample_freq) );

	//TEST: for(ii=0;ii<nn;ii++) fprintf(stderr,"%ld	%g	%g\n",ii,input[ii],power[ii]); return(-1);

	if(nn<4) {
		sprintf(message,"%s (no filtering - number of input samples is less than 4)",thisfunc);
		return(-1);
	}

	if(dotaper==1) {

		/* CREATE A MODFIED HANN TAPER */
		if((taper=(double *)malloc((nwin)*sizeof(double)))==NULL) {
			sprintf(message,"%s (memory allocation problem)",thisfunc);
			return(-1);
		}
		aa=(2.0*M_PI)/(nwin-1.0);
		for(ii=0;ii<nwin;ii++) taper[ii] = 1.0 - cosf(ii*aa) ;

		/*
		CALCULATE THE POWER FOR THE MIDDLE OF THE ARRAY
		- note that power is calculated over a window beginning at position ii
		- consequently the result applies to position ii+(nwin/2)
		*/
		stop= (nn-nwin)-1;
		for (ii=0; ii<stop; ii++)  {
			p1=p2=0.0;
			mm=ii+nwin;
			for(kk=0,jj=ii;jj<mm;jj++) {
				bb= (input[jj]*taper[kk++])+(coef*p1)-p2;
				p2= p1;
				p1= bb;
			}
			power[ii+halfwin]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
		}
		/*
		2. CALCULATE THE POWER FOR THE BEGINNING OF THE ARRAY
		- note that we begin with hypothetically negative positions
		- consequently we cannot use size_t counters for most calculations but long integers instead
		- this should be ok as numbers at the beginning of the file should fall within the range of long integers anyway
		- zero is substituted for values of input[<0]
		*/
		x=(long)nwin*(-1);
		for (x=x; x<0; x++)  {
			p1=p2=0.0;
			z=x+nwin;
			for(kk=0,y=x;y<z;y++) {
				if(y>=0) bb= (input[y]*taper[kk++])+(coef*p1)-p2;
				else bb= (coef*p1)-p2;
				p2= p1;
				p1= bb;
			}
			z=x+halfwin;
			if(z>=0) power[z] = sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
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
				if(jj<nn) bb= (input[jj]*taper[kk++])+(coef*p1)-p2;
				else bb= (coef*p1)-p2;
				p2= p1;
				p1= bb;
			}
			jj=ii+halfwin;
			if(jj<nn) power[jj]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
		}
		free(taper);
	}


	/* OTHERWISE DO THE EXACT SAME TING BUT WITHOUT TAPERING */
	else if(dotaper!=1)  {

		stop= (nn-nwin)+1;
		for (ii=0; ii<stop; ii++)  {
			p1=p2=0.0;
			mm=ii+nwin;
			for(jj=ii;jj<mm;jj++) {
				bb= input[jj]+(coef*p1)-p2;
				p2= p1;
				p1= bb;
			}
			power[ii+halfwin]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
		}
 		x=(long)nwin*(-1);
		for (x=x; x<0; x++)  {
 			p1=p2=0.0;
 			z=x+nwin;
 			for(y=x;y<z;y++) {
 				if(y>=0) bb= input[y]+(coef*p1)-p2;
				else bb= (coef*p1)-p2;
 				p2= p1;
 				p1= bb;
 			}
			z=x+halfwin;
 			if(z>=0) power[z] = sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
 		}
		for (ii=stop; ii<nn; ii++)  {
			p1=p2=0.0;
			mm=ii+nwin;
			for(jj=ii;jj<mm;jj++) {
				if(jj<nn) bb= input[jj]+(coef*p1)-p2;
				else bb= (coef*p1)-p2;
				p2= p1;
				p1= bb;
			}
			jj=ii+halfwin;
			if(jj<nn) power[jj]= sqrt(((p2*p2) + (p1*p1) - (p1*p2*coef)) * scaling);
		}

	}

	return(0);
}
