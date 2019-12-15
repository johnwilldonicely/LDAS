/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Invalidate sections of good data which fail a minimum length requirement
	This version is designed to deal with multi-channel input data
	float version

USES:
	- conditioning step before interpolating
	- apply a general QC to "intermittent" data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data0      input array martix, row=sample, column=channel
	size_t nn:        length of data0 array (number of rows, i.e. multi-channel samples)
	size_t nchan:     number of channels (columns)
	size_t mingood:   minimum number of sequential valid points required to pass filter
	float setbad:     bad-value to look for (e.g SHRT_MAX)
	char message[]:	  message indicating success or reason for failure

RETURN VALUE:
	0 on success, -1 on fail

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int xf_filter_mingood2_f(float *data0, size_t nn, size_t nchan, size_t mingood, float setbad, char *message) {

	char *thisfunc="xf_filter_mingood2_f\0";
	float tempdat;
	size_t ii,jj,kk,mm,ngood=0;

	sprintf(message,"%s (incomplete))",thisfunc);

	if(nn<mingood) {
		sprintf(message,"%s (criterion (%ld) must be <= data length (%ld))",thisfunc,mingood,nn);
		return(-1);
	}

	/* SCAN FOR LENGTHS OF GOOD DATA  */
	for(ii=0;ii<nn;ii++) {
		if(data0[ii*nchan]==setbad) {
			if(ngood>=mingood) ngood=0;
			for(jj=ii-ngood;jj<=ii;jj++) { mm=jj*nchan; for(kk=0;kk<nchan;kk++) { data0[mm+kk]=setbad; }}
			ngood=0;
		}
		else ngood++;
	}
	if(ngood>0 && ngood<mingood) {
		for(jj=(nn-ngood);jj<nn;jj++) { mm=jj*nchan; for(kk=0;kk<nchan;kk++) { data0[mm+kk]=setbad; }}
	}

	/* WRAP UP */
	sprintf(message,"%s (successfully filtered %ld points)",thisfunc,nn);
	return(0);

}
