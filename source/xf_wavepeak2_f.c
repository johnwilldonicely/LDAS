/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION :
	For a multi-channel waveform file, find:
	 	1. The peak channel
			- depending on sign, this is the channel with the smallest or largest value at time zero
		2. The minimum and maximum value at peak-time
			- compares values across channels at time zero
		3. The min/max ratio
			- similarity between min and max, corrected for whether the values are < or > zero

	NOTE: unlike xf_wavepeak1, this function assumes the peak is near a known position in the waveform
	NOTE: recommend use of xf_wavefilt1_f to filter the waveforms before detecting the peak

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *wave    : array of voltage values representing a multi-channel waveform
	long nchan     : the number of channels in the compound-waveform (eg. 4 for a tetrode waveform)
	long spklen    : the number of samples from each channel: length(wave)= nchan*spklen
	long setzero   : the sample-number within each channel corresponding to time-zero (the detected peak)
	int sign       : whether the original spike-detection was for negative (-1) or positive (1) events
	float *resultf : an array to hold the output
	char *message  : a string array to hold a status message

RETURN VALUE:
	- on success: the channel containing the peak (negative or positive, depending on "sign")
	- on failure: -1

	- resultf[0]: at time zero, smallest (sign=-1) or largest (sign=1) value
	- resultf[1]: cross-channel minimum value at time zero (peak)
	- resultf[2]: cross-channel maximum value at time zero (peak)
	- resultf[3]: the difference between the minimum and maximum
	- resultf[4]: the adjusted min:max ratio (0-1)
		- the ratio of the smallest deflection from zero relative to the largest
		- values approaching 1 indicate indentical signals on all channels
		- values approaching -1 are increasingly different, -1 being the maximum difference for values on opposite sides of zero
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include<float.h>

long xf_wavepeak2_f(float *wave, long nchan, long spklen, long setzero, int sign, float *resultf, char *message) {

	char *thisfunc="xf_wavecor1_f\0";
	long ii,jj,kk,mm,nn,wavelen,jitter1,jitter2,start,stop,minsamp,maxsamp,peakchan;
	float aa,bb,ww,min1,max1,peak,diff,ratio;
	double min2,max2;

	/* CHECK INPUT PARAMETERS */
	if(sign!=-1&&sign!=1) {sprintf(message,"%s [ERROR]: sign must be -1 or 1",thisfunc); return(-1);}

	/* DEFINE ZONE AROUND ZERO TO SCAN FOR PEAK */
	jitter1= 1; // this defines the size of the zone each side of zero to scan
	if(jitter1>setzero) jitter1=setzero;
	jitter2= jitter1+1 ;
	if((setzero+jitter2)>spklen) jitter2= spklen-setzero;
	//fprintf(stderr,"jitter1=%ld\tjitter2=%ld\n",jitter1,jitter2);

	/* INITIALIZE VARIABLES */
	wavelen= nchan*spklen;
	minsamp= maxsamp= -1;
	mm= 0;
	min1= FLT_MAX;
	max1= FLT_MIN;
	peak=ratio= NAN;

	/* FIND THE MINIMUM AND MAXIMUM PEAK VALUES */
	for(ii=setzero;ii<wavelen;ii+=spklen) {
		start= ii-jitter1;
		stop=  ii+jitter2;
		for(jj=start;jj<stop;jj++) {
			ww=wave[jj];
			if(isfinite(ww)) {
				if(ww<min1) { min1=ww; minsamp=jj; }
				if(ww>max1) { max1=ww; maxsamp=jj; }
				mm++;
	}}}
	/* make sure some good values are present! */
	if(mm==0) {sprintf(message,"%s [ERROR]: no valid peak-values in waveform",thisfunc); return(-1);}

	/* DETERMINE THE PEAK CHANNEL */
	if(sign==-1) { peak= min1; peakchan= (long)(minsamp/spklen); }
	else         { peak= max1; peakchan= (long)(maxsamp/spklen); }

	/* CALCULATE THE RATIO BETWEEN MIN AND MAX */
	/* make double copies of min and max */
	min2= (double)min1;
	max2=  (double)max1;
	/* avoid exact zero */
	if(min1==0.F) min2= nextafter(0,1);
	if(max1==0.F) max2= nextafter(0,1);
	/* ratio option 1: both > 0 */
	if(min2>0.0 && max2>0.0) ratio= (float)(min2/max2);
	/* ratio option 2: both < 0 */
	else if(min2<0.0 && max2<0.0) ratio= (float)(max2/min2);
	/* ratio option 3: values fall both sides of zero - ratio ranges from 0 to -1  */
	else if(min2<0.0 && max2>0.0) {
		aa=fabs(min2);
		if(max2>aa) ratio= (float)(min2/max2);
		if(aa>max2) ratio= (float)(max2/min2);
		else ratio= -1.F;
	}

	/* FILL THE RESULTS ARRAY AND RETURN THE PEAK CHANNEL  */
	sprintf(message,"%s [OK]",thisfunc);
	resultf[0]=peak;
	resultf[1]=min1;
	resultf[2]=max1;
	resultf[3]=(max1-min1);
	resultf[4]=ratio;

	return(peakchan);
}
