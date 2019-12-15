/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION :
	- Butterworth-filter each channel in a multi-channel waveform
	- 1. linear interpolation is used to remove NAN or INF values
		- omitted if all samples in a channel are NAN or INF
	- 2. subtract the channel-mean
	- 3. apply the bidirectional filter
	- 4. normalize each channel to the first sample in that channel

USES:
	Remove unwanted components of a waveform before calculatting peak, width etc.

DEPENDENCIES:
	xf_interp3_f
	xf_filter_bworth1_f

ARGUMENTS:
	float *wave    : array of voltage values representing a multi-channel spike waveform
	int nchan      : the number of channels (eg. 4 for a tetrode waveform)
	int spklen     : samples comprising the spike on each channel
	float setrate  : the sample-rate of the input
	float setlow   : the low-frequency cutoff (0= no low-cut filtering)
	float setlow   : the high-frequency cutoff (0= no high-cut filtering)
	char *message  : pre-allocated array to hold feedback

RETURN VALUE:
	0 if no problems, -1 if there was an error (no valid data)

	*wave will contain the within-channel filtered data

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

long xf_interp3_f(float *data, long ndata);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);

int xf_wavefilt1_f(float *wave, int nchan, int spklen, float setrate, float setlow, float sethigh, char *message) {

	char *thisfunc="xf_wavefilt_f\0";
	int x,y,z,goodchan=0;
	long ii,jj,kk;
	float *pspk;
	double mean;

	/* FILTER THE SPIKE ON EACH CHANNEL */
	for(ii=0;ii<nchan;ii++) {

		/* define pointer to spike */
		pspk= wave+(ii*spklen);

		/* interpolate the spike - if no good data, leave it */
		z= xf_interp3_f(pspk,spklen);
		if(z>=0) goodchan++;
		else continue;

		/* de-mean the spike */
		mean=0.0;
		for(jj=0;jj<spklen;jj++) mean+= pspk[jj];
		mean/=(double)spklen;
		for(jj=0;jj<spklen;jj++) pspk[jj]-=mean;

		/* apply the filter */
		z= xf_filter_bworth1_f(pspk,spklen,setrate,setlow,sethigh,1.4142,message);
		if(z!=0) return(-1);

		/* restore normalization of spike to the first sample */
		for(jj=1;jj<spklen;jj++) pspk[jj]-= pspk[0];
		pspk[0]=0.0;
	}

	/* CHECK THAT AT LEAST ONE GOOD CHANNEL WAS FOUND */
	if(goodchan==0) { sprintf(message,"%s (no valid data on any channel)",thisfunc); return(-1);}

	/* RETURN WITH SUCCESS STATUS */
	return(0);
}
