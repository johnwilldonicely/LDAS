/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:

	Create a complex Morlet wavelet

	Based on MATLAB functions energyvec.m and morelet_wav.m  by Ole Jensen, 1997-1998

	Original notes:
		% Create a Morlet wavelet 'y' with frequency resolution 'f' and temporal
		% resolution 't'. The wavelet will be normalized so the total energy is 1.
		% The 'width' defines the temporal and frequency resolution for the given
		% centre frequency 'f' by determining the number of cycles of the wavelet
		% itself (see Tallon-Baudry et al., J. Neurosci. 15, 722-734 (1997) or
		% Event-Related Potentials: A Methods Handbook, Handy (editor), MIT Press,
		% (2005))

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double freq   : central frequency of wavelet
	double srate  : sampling freq. of signal to be transformed with the wavelet
	int width     : parameter which defines the mother wavelet. This is then scaled and translated to filter for different frequencies, >= 5 is suggested, see Tallon-Baudry et al., J. Neurosci. 15, 722-734 (1997))
	size_t *nwav  : address to a variable to hold the resultant size (elements) of the wavelet (modified by this function)


RETURN VALUE:
	pointer to the wavelet, type = complex float, size=nwav
	NULL on failure

SAMPLE CALL:

	#include <math.h>
	#include <stdio.h>
	#include <complex.h>
	complex float *xf_morletwavelet2_f(double f, double srate, int width, size_t *nwavelet);
	int main() {
		double freq=8.0;
		double sr=24000;
		int width=7;
		size_t ii,nwav;

		complex float *wavelet = xf_morletwavelet2_f(freq,srate,width,&nwav);

		for(ii=0;ii<nwav;ii++) printf("%d	%g	%g\n",ii,creal(wavelet[ii]),cimag(wavelet[ii]));
		free(wavelet);
	}

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

complex float *xf_morletwavelet2_f(double freq, double srate, int ncycles, size_t *nwav)  {

	size_t ii,nn;
	double dcycles,dt,sf,st,aa,bb,cc,dd,tt;
	double *t_value;
	complex float *wavelet;

	nn=*nwav;
	dcycles=(double)ncycles;
	dt = 1.0/srate;
	sf = freq / dcycles;
	st = 1.0 / (2.0 * M_PI * sf);
	nn = (size_t) (2.0*((dcycles/2.0) * st)/dt);

	/* allocate memory for the wavelet time-values */
	t_value= (double *) malloc(nn*sizeof(double));
	if(t_value==NULL) return(NULL);

	/* allocate memory for a complex variable array to hold the wavelet */
	wavelet = (complex float *) malloc(nn*sizeof(complex float));
	if(wavelet==NULL) return(NULL);

	/* construct the time-values for the morlet series */
	for(ii=0;ii<nn;ii++) {
		t_value[ii] = ( -1.0 * (dcycles / 2.0) * st ) + (dt * ii) ;
	}

	/* create the complex wavelet */
	aa= 1.0/ sqrt((st * sqrt(M_PI)));
	bb= 2.0 * (st*st);
	cc= 2.0 * M_PI * freq;

	for(ii=0;ii<nn;ii++) {
		tt = t_value[ii];
		dd = aa * exp(-1.0*(tt*tt) / bb );
		/* wavelet = real + I*imaginary  */
		wavelet[ii]= (float)(dd*(cos(cc*tt))) + I*(float)(dd*(sin(cc*tt)));
	}

	/* update number of elements in wavelet (number of real/imaginary pairs */
	*nwav=nn;

	free(t_value);
	return(wavelet);
}
