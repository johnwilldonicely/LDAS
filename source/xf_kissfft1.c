/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	Perform a fast Fourier transfer on a time-series

USES:

DEPENDENCY TREE:

ARGUMENTS:

RETURN VALUE:

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kiss_fftr.h"

int xf_kissfft1(double *pdata, double *buff2, double *taper, double *spect, long *parameters, kiss_fftr_cfg *cfgr, kiss_fft_cpx *fft) {

	char *thisfunc="xf_kissfft1\0";
	int sizeofdouble=sizeof(double);
	long i,j,k,tot=0;

	size_t ii,Xn=0,Xp=0;
	double sum,mean,ar,ai,aa,scaling1;
	long setnwin=parameters[0],setmean=parameters[1],setunits=parameters[2],halfnwin=setnwin/2;

	/* define scaling factors - size of data sent to fft */
	scaling1=1.0/(float)setnwin; /* defining this way permits multiplication instead of (slower) division */

	/* determine mean-correction to window, if required */
	if(setmean==1) {
		sum=0; for(i=0;i<setnwin;i++) sum+=pdata[i]; mean=sum*scaling1;
		/* copy real data from pdata to buff2, and apply mean-correction + taper */
		for(i=0;i<setnwin;i++) buff2[i]= (pdata[i]-mean) * taper[i];
	}
	else {
		mean=0.0;
		/* copy real data from pdata to buff2, and apply taper */
		for(i=0;i<setnwin;i++) buff2[i]= pdata[i] * taper[i];
	}

	/* call the fft function **********/
	kiss_fftr(cfgr,buff2,fft);


	/* store the fftreal, fftimag & spectrum:  note that at present this must be done for the entire half-spectrum, not just indexa to indexb */
	if(setunits==0) { /* units= amplitude peak */
		aa=2.0 * scaling1;
		for(i=0;i<halfnwin;i++) { ar= fft[i].r; ai= fft[i].i; spect[i]= aa * sqrtf( ar*ar + ai*ai ); }
	}
	else if(setunits==1) { /* units= amplitude peak (dB) */
		aa=2.0 * scaling1;
		for(i=0;i<halfnwin;i++) { ar= fft[i].r; ai= fft[i].i; spect[i]= 20.0 *log10(1.0 + (aa * sqrtf( ar*ar + ai*ai ))); }
	}
	else if(setunits==2) { /* units= RMS */
		aa=sqrtf(2.0) * scaling1;
		for(i=0;i<halfnwin;i++) { ar= fft[i].r; ai= fft[i].i; spect[i]= aa * sqrtf( ar*ar + ai*ai ); }
	}
	else if(setunits==3) { /* units= RMS-Squared */
		aa= 2.0 * scaling1 * scaling1;
		for(i=0;i<halfnwin;i++) { ar= fft[i].r; ai= fft[i].i; spect[i]= aa * ( ar*ar + ai*ai ); }
	}


	return (0);
}
