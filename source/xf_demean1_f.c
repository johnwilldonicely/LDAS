/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Apply a sliding boxcar-average normalization to a data series - local mean is subtracted from each datum
	Acts as a high-pass filter
	Uses a single-sample-step sliding window, so output is same length as input
	Data at the beginning and end of the input is adjsted by a fraction of the window size not less than 1/2

	NOTE: if input contains invalid values, NAN or INF, output will be invalid
		- in such cases it is recommended to interpolate the input before passing to this function

USES:
	Signal analysis

DEPENDENCIES:
	None

ARGUMENTS:

	float *input   : pointer to array holding amplitude time series to be normalized
	long nn        : length of the input & output arrays
	long nwin1     : length of averaging window, +1 if even
				- this value will be increased by 1 if not an odd number, or decreased if this exceeds the data length
	char *message  : feedback returned to the calling function, which should allocate memory for this array (256 characters)

RETURN VALUE:
	0 on success, input[] will be overwritten
	-1 on error
	message array will hold explanatory text (if any)

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int xf_demean1_f(float *input, long nn, long nwin1, char *message) {

	char *thisfunc="xf_demean1_f\0";
	float *output=NULL;
	size_t ii,jj,mm,nwin2,start,stop,oddnwin1=0;
	double sum,scale1;

	/* CHECK FOR INVALID INPUT */
	if(nn<3) { sprintf(message,"%s [ERROR]: input length %ld must not be less than 3",thisfunc,nn); return(-1); }
	if(nwin1>nn) { sprintf(message,"%s [ERROR]: window size %ld exceeds input length %ld",thisfunc,nwin1,nn); return(-1); }
	/* ALLOCATE TEMPORARY ARRAY */
	output= malloc(nn*sizeof(*input));
	if(output==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }
 	/* CHECK IF ORIGINAL NWIN1 IS ODD */
	if(nwin1%2==0) oddnwin1=0; else oddnwin1=1;
	/* DETERMINE WORKING WINDOW SIZE AND HALF-WINDOW SIZE */
	/* if window size = entire input and nn is an even number, nwin2 must 1 less than half of nwin1   */
	if (nwin1==nn && (nn%2)==0) { nwin2= (nwin1/2) -1; }
	/* otherwise nwin2 is rounded down from half of nwin1 */
	else { nwin2= (size_t)( (double)nwin1/2.0 ); }
	/* make sure nwin1 is an odd number - if original nwin1 was even, this will increase the value by 1 */
	nwin1= nwin2+nwin2+1;
	/* CALCULATE THE SCALING FACTOR */
	scale1=1.0/(double)nwin1;
	//TEST:	fprintf(stderr,"nn=%ld nwin1=%ld nwin2=%ld\n",nn,nwin1,nwin2);

	/* AVERAGE FOR THE FIRST PORTION OF THE DATA */
	/* INCLUDES INITIALIZINGING THE RUNNING SUM  */
	start= 0;
	stop= nwin2+1;
	mm= nwin2+1;
	/* pre-calculate the sum for the first window */
	sum= 0.0;
	for(jj=start;jj<stop;jj++) { sum+= input[jj]; }
	output[start]= input[start]-(sum/mm);
	/* re-calculate the sum for each window up to and including the window centred on halfwin (trailing edge = input[0])*/
	start++;
	for(ii=start;ii<stop;ii++) {
		mm++;
		sum+= input[ii+nwin2];
		output[ii]= (float) ( (double)input[ii] - (sum/(double)(mm)) );
	}
	/* AVERAGE FOR THE MIDDLE PORTION OF THE DATA */
	start= nwin2+1;
	stop= nn-nwin2;
	jj= (start-nwin2-1);
	// calculate the sum for each window
	for(ii=start;ii<stop;ii++) {
		sum-= input[jj++]; /* subtract the number just behind the trailing edge of the window */
		sum+= input[ii+nwin2]; /* add the number at the leading edge of the window */
		output[ii]= (float) ( (double)input[ii] - (sum*scale1) );
	}
	/* AVERAGE FOR THE FINAL PORTION OF THE DATA */
	start= nn-nwin2;
	stop= nn;
	jj= (start-nwin2-1);
	mm= nwin1;
	// calculate the sum for each window
	for(ii=start;ii<stop;ii++) {
		mm--;
		sum-= input[jj++]; /* subtract the number just behind the trailing edge of the window */
		output[ii]= (float) ( (double)input[ii] - (sum/(double)(mm)) );
	}

	/* COPY BACK */
	for(ii=0;ii<nn;ii++) input[ii]= output[ii];

	/* WRAP UP */
	if(oddnwin1==0) sprintf(message,"%s (calculated boxcar average for %ld input points, adjusted window size %ld)",thisfunc,nn,nwin1);
	if(oddnwin1==1) sprintf(message,"%s (calculated boxcar average for %ld input points, window size %ld)",thisfunc,nn,nwin1);
	if(output!=NULL) free(output);
	return(0);
}
