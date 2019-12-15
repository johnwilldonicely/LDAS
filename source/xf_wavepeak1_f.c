/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION :
	John Huxter: 16 June 2017

	Finds the channel containing the peak, and the value of it,
	in a multi-channel waveform

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	float *wave    : array of voltage values representing a spike waveform
	int wavelen    : samples in waveform
	int nchan      : if a compound waveform, the number of channels (eg. 4 for a tetrode waveform)
	int sign       : determines if peak should be negative (-1) or positive (+1)
	float *resultf : an array [2] to hold the results


RETURN VALUE:
	the channel containing the peak, or -1 on error

	resultf[0] = the peak value (negative or positive, depending on sign)
*/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

int xf_wavepeak1_f(float *wave, int wavelen, int nchan, int sign, float *resultf ) {

	int i,j,k,x,y,z,x1,x2,x3;
	int peaksamp,chan,spklen;
	float a,b,c,peakval;

	// initialize peak sample-number and value
	peaksamp=-1;
	resultf[0]=NAN;

	// establish length of waveform on each channel
	spklen= wavelen/nchan;

	if(sign==-1) {
		// find minimum voltage (must be <0)
		peakval= FLT_MAX;
		for(i=0;i<wavelen;i++) {
			a=wave[i];
			if(isfinite(a) && a<peakval) {peakval=a; peaksamp=i;}
		}
		// if peakval has not changed, there is no valid data in the waveform
		if(peakval==FLT_MAX) return(-1);
	}

	if(sign==1) {
		// find maximum voltage (must be <0)
		peakval= FLT_MIN;
		for(i=0;i<wavelen;i++) {
			a=wave[i];
			if(isfinite(a) && a>peakval) {peakval=a; peaksamp=i;}
		}
		// if peakval has not changed, there is no valid data in the waveform
		if(peakval==FLT_MIN) return(-1);
	}

	// store the peak value
	resultf[0]= peakval;
	// return the channel containing the peak
	return(peaksamp/spklen);
}
