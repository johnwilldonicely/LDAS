/*
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Clip values in an array exceeding certain limits
	A minimum number of sequencial valid data must be present in order to avoid clipping

USES:
	- restrict the data range in a data series
	- invalidate values exceeding a certain threshold

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data0:     array holding input to be filtered, fixed sample rate is assumed
	size_t nn:        length of data0 array
	size_t nwin:      minimum number of sequential valid points required to pass filter
	float min:        minimum value to keep
	float max:        maximum value to keep
	float newmin:     clipped value to replace numbers less than min
	float newmax:     clipped value to replace numbers more than max
	char message[]:	  message indicating success or reason for failure

RETURN VALUE:
	0 on success, -1 on fail

*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int xf_filter_clip1_f(float *data0, size_t nn, size_t nwin, float min, float max, float newmin, float newmax, char *message) {

	char *thisfunc="xf_filter_clip1_f\0";
	size_t ii,jj,ngood=0;

	sprintf(message,"%s (incomplete))",thisfunc);

	if(nn<nwin) {
		sprintf(message,"%s (window size (%ld) must be <= data length (%ld))",thisfunc,nwin,nn);
		return(-1);
	}

	/* SCAN & CLIP RECORDS - NON-NUMBERS SET TO NEWMIN */
	for(ii=0;ii<nn;ii++) {
		if(data0[ii]<min || !isfinite(data0[ii])) {
			if(ngood>=nwin) ngood=0;
			for(jj=ii-ngood;jj<=ii;jj++) data0[jj]=newmin;
			ngood=0;
		}
		else if(data0[ii]>max) {
			if(ngood>=nwin) ngood=0;
			for(jj=ii-ngood;jj<=ii;jj++) data0[jj]=newmax;
			ngood=0;
		}
		else ngood++;
	}
	if(ngood>0 && ngood<nwin) {
		for(ii=(nn-ngood);ii<nn;ii++) {
			if(data0[ii]<min || !isfinite(data0[ii])) data0[ii]=newmin;
			else if(data0[ii]>max) data0[ii]=newmax;
		}

	}

	/* WRAP UP */
	sprintf(message,"%s (successfully filtered %ld points)",thisfunc,nn);
	return(0);

}
