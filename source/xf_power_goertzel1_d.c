/*
<TAGS>stats signal_processing</TAGS>
DESCRIPTION:
	Calculate the amplitude of a frequency component in a signal, using the Goertzel algorithm
	Return a power estimate for a single window
	- this is essentially a DFT for the specified frequency
	- real-valued input is assumed
	- data should be interpolated to remove non-numeric values, NAN or INF

	http://en.wikipedia.org/wiki/Goertzel_algorithm
	http://netwerkt.wordpress.com/2011/08/25/goertzel-filter/

	Note that rather than the magnitude of the signal, this function calculates the amplitude
		- hence the results reflect the original amplitude of the input signal
		- each power magnitude is adjusted by (2.0/nwin) and the square-root is taken


USES:
	- Fast detection of a frequency in a signal


DEPENDENCY TREE:
	No dependencies


ARGUMENTS:
	double *input :      data array to be filtered, fixed sample rate is assumed
	size_t nn :          size of the data array (number of samples)
	double sample_freq : sample frequency (Hz)
	double freq :        frequency of interest (Hz)


RETURN VALUE:
	success: 0
	failure: -1

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double xf_power_goertzel1_d(double *input, size_t nn, double sample_freq, double freq) {

	size_t ii;
	double aa,p1,p2,coef,scale;

	coef= 2.0 * cosf(2.0 * M_PI * (freq/sample_freq) );
	p1=p2=0.0;
	scale = 2.0/(double)nn;

	for (ii=0; ii<nn; ii++)  {
		aa= input[ii]+(coef*p1)-p2;
		p2= p1;
		p1= aa;
	}

	return( scale * sqrt( (p2*p2) + (p1*p1) - (p1*p2*coef) ) );
}
