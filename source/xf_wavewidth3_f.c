/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION :
	Find the width of the peak (at a % of the peak amplitude) on one channel of a multi-channel waveform
	- needs to know the peakchan (use xf_wavepeak2_f for example)
	- scans within that channel for the minimum or maximum, depending on sign
	- NOTE: this function scans the waveform for the peak - it does not take a "time-zero" argument"
	- NOTE: input should be interpolated and filtered (no NAN or INF values)
		- xf_wavefilt1_f() can be used to interpolate and filter
		- note that if a channel is dead (all NANs), interpolation will not fix the problem
		- in this case, that channel should not be passed to this function!
	- NOTE: the value for setwidth would be 0.25 according to Csicsvari et al, 1999
		- however, the the halfwidth (0.5) may be more reliable, because depending on filtering, some waveforms never return to 25%
USES:

DEPENDENCIES:
	None

ARGUMENTS:
	float *wave     : array of voltage values representing a multi-channel spike waveform
	int nchan       : the number of channels (eg. 4 for a tetrode waveform)
	int spklen      : samples in each channel (individual spike waveforms)
	int peakchan    : the channel containing the peak
	int sign        : determines if detected peak should be negative (-1) or positive (+1)
	double setthres : the proportion of peak-amplitude at which to calculate the width (eg. 0.5 = 50%, i.e. half-width)
	float *resultf  : an array [3] to hold the results
	char *message   : pre-allocated array to hold error message

RETURN VALUE:
	0 if no problems, -1 if there was an error (no valid data)

	resultf[0] = width at (setwidth*100)% of the peak value
	resultf[1] = the peak value (negative or positive, depending on sign)
	resultf[2] = setwidth x peak_value (where the width is calculated)
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

int xf_wavewidth3_f(float *wave, int nchan, int spklen, int peakchan, int sign, double setthresh, float *resultf, char *message) {

	char *thisfunc="xf_wavewidth3_f\0";
	long x1,x2,ii,jj,kk,peaksamp;
	float *pwave,peakval,width;
	double aa,bb,cc,y1,y2,thresh;

	/* SET UP A POINTER TO THE PEAK CHANNEL IN THE COMPOUND WAVE */
	pwave= wave+(peakchan*spklen);

	/* CHECK VALIDITY OF ARGUMENTS */
	if(peakchan>=nchan) { sprintf(message,"%s [ERROR]: selected channel (%d) must be < number of channels (%d)",thisfunc,peakchan,nchan); return(-1); }

	/* CHECK INTEGRITY OF SELECTED PEAK CHANNEL */
	for(ii=0;ii<spklen;ii++) if(!isfinite(pwave[ii])) break;
	if(ii<spklen) { sprintf(message,"%s [ERROR]: selected channel (%d) contains non-finite values",thisfunc,peakchan); return(-1); }

	/* INITIALIZE VARIABLES */
	resultf[0]=NAN;
	resultf[1]=NAN;
	resultf[2]=NAN;
	peaksamp=-1;

	/* OPTION1: NEGATIVE PEAK, FIND LEFT(X1) AND RIGHT (X2) THRESHOLD CROSSINGS */
	if(sign==-1) {
		peakval= FLT_MAX;
		/* find the peak value and peak sample-number */
		for(ii=0;ii<spklen;ii++) if(pwave[ii]<peakval) { peakval= pwave[ii]; peaksamp= ii; }
		/* define the threshold */
		thresh = (double)peakval*setthresh;
		/* look backwards (x1) and forwards (x2) for samples exceeding the threhold */
		for(x1=peaksamp;x1>=0;x1--)     if(pwave[x1]>thresh) break;
		for(x2=peaksamp;x2<spklen;x2++) if(pwave[x2]>thresh) break;
	}
	/* OPTION2: POSITIVE PEAK, FIND LEFT(X1) AND RIGHT (X2) THRESHOLD CROSSINGS */
	if(sign==1) {
		peakval= FLT_MIN;
		/* find the peak value and peak sample-number */
		for(ii=0;ii<spklen;ii++) if(pwave[ii]>peakval) { peakval= pwave[ii]; peaksamp= ii; }
		/* define the threshold */
		thresh = (double)peakval*setthresh;
		/* look backwards (x1) and forwards (x2) for samples exceeding the threhold */
		for(x1=peaksamp;x1>=0;x1--)     if(pwave[x1]<thresh) break;
		for(x2=peaksamp;x2<spklen;x2++) if(pwave[x2]<thresh) break;
	}

	/* CALCULATE INTERPOLATED (FRACTIONAL) SAMPLE-NUMBER AT WHICH THRESHOLD WAS CROSSED */
	/* left-of-peak crossing - aa, relative to x1 */
	if(x1<0) aa= 0.0;
	else {
		kk= x1+1;
		if(kk>=spklen) kk=(spklen-1);
		y1= (double)pwave[x1];
		y2= (double)pwave[kk];
		cc=(y1-y2);
		if(cc!=0.0) aa= (double)x1+(y1-thresh)/cc; // true crossing is forward
		else aa=(double)x1;
	}
	/* right-of-peak crossing - aa, relative to x2 */
	if(x2>=spklen) bb= (double)(spklen-1);
	else {
		kk= x2-1;
		if(kk<0) kk=0;
		y1= (double)pwave[x2];
		y2= (double)pwave[kk];
		cc=(y1-y2);
		if(cc!=0.0) bb= (double)x2-(y1-thresh)/cc; // true crossing is backward
		else bb=(double)x2;
	}

	/* calculate width in fractional number of samples */
	width= (float)(bb-aa);
	//TEST:	fprintf(stderr,"	peaksamp=%ld value=%.2f thresh:%.2f x1:%ld x2:%ld aa:%.2f bb:%.2f width:%.2f\n",peaksamp,peakval,thresh,x1,x2,aa,bb,width);
	//TEST:	for(ii=0;ii<spklen;ii++) printf("%ld	%g\n",ii,pwave[ii]);

	resultf[0]=width;
	resultf[1]=peakval;
	resultf[2]=thresh;

	return(0); // status good
}
