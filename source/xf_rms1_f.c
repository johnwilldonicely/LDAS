/*
<TAGS>math stats</TAGS>
DESCRIPTION:
	- Calculate the root-mean-square (RMS) power in an array
	- This is the square root of the mean of the summed squared-values
	- NAN and INF values will be ignored

USES:
	Signal analysis

DEPENDENCIES:
	No dependencies

ARGUMENTS:

	float *input : pointer to array holding amplitude time series to be converted to RMS power
	long nn       : length of the input
	char *message : feedback returned to the calling function, which should allocate memory for this array (256 characters)

RETURN VALUE:
	RMS value on success
	NAN on error

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double xf_rms1_f(float *input, long nn, char *message) {

	size_t ii,mm=0;
	double sum=0.0,sum2=0.0,aa;
	char *thisfunc="xf_rms1_d\0";

	for(ii=0;ii<nn;ii++) {
		aa= (double)input[ii];
		if(isfinite(aa)) {
			sum2+= aa*aa;
			mm++;
		}
	}

	if(mm==0) {
		sprintf(message,"%s [ERROR]: no valid data in input",thisfunc);
		return(NAN);
	}
	else {
		return(sqrt((sum2/(double)mm)));
	}

}
