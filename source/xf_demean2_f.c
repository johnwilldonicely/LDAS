/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	- for a point in a series, return the point minus the mean of a window centred on the point
	- acts as a high-pass filter
	- can be called iteratively to de-mean an entire series

USES:
	- eliminate long-term trends in the data
	- adaptive "de-meaning", where the window varies according to the position n the array

DEPENDENCIES:
	None

ARGUMENTS:
	float *input   : input: pointer to array of values
	long nn        : length of input
	long halfwin   : half-length of averaging window, must be >0
	long index     : index to position in input on which the window is centred
	char *message  : feedback returned to the calling function, which should allocate memory for this array (256 characters)

RETURN VALUE:
	success: de-meaned value
	error: NAN
	message array will hold explanatory text (if any)
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

float xf_demean2_f(float *input, long nn, long halfwin, long index, char *message) {

	char *thisfunc="xf_demean2_f\0";
	long ii,jj,kk,mm;
	double sum;

	/* CHECK FOR INVALID INPUT */
	if(nn<3) { sprintf(message,"%s [ERROR]: input length %ld must not be less than 3",thisfunc,nn); return(NAN); }
	if(halfwin<1) { sprintf(message,"%s [ERROR]: half-window size %ld is invalid - must be >0",thisfunc,halfwin); return(NAN); }
	if(halfwin>(nn/2)) { sprintf(message,"%s [ERROR]: half-window size %ld exceeds input length %ld",thisfunc,halfwin,nn); return(NAN); }
	if(index>=nn) { sprintf(message,"%s [ERROR]: index %ld exceeds input length %ld",thisfunc,index,nn); return(NAN); }
	if(!isfinite(input[index]])) { sprintf(message,"%s [ERROR]: index %ld points to a non-finite number - consider interpolating the input",thisfunc,index); return(NAN); }

	/* SET THE BOUNDARIES FOR THE AVERAGING WINDOW : jj=left, kk=right */
	jj= index-halfwin; if(jj<0) jj= 0;
	kk= index+halfwin; if(kk>=nn) kk= nn-1;;

	/* CALCULATE THE MEAN IN THE WINDOW */
	mm= 0;      // counter for the number of values contributing to the mean
	sum= 0.0;   // initialise the sum
	for(ii=jj;ii<=kk;ii++) {
		if(isfinite(input[ii])) { // exclude NAN or INF values
			sum+= input[ii];
			mm++;
	}}

	/* RETURN THE INDEX MINUS THE MEAN, OR NAN IF NO DATA IN WINDOW WAS VALID  */
	if(mm>0) {
		return( (float)( input[index] - sum/(double)mm) )
	}
	else {
		sprintf(message,"%s [ERROR]: no valid data in window at index %ld",thisfunc,index);
		return(NAN);
	}

}
