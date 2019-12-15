/*
<TAGS>math stats</TAGS>
DESCRIPTION:
	Calculate the root-mean-square (RMS) power in an array
	Uses a single-sample-step sliding window, so output is same length as input
	Output values will ramp up at the beginning and ramp down at the end
	More efficient than calculating RMS in 50% overlapping windows, but requires extra memory (length of data)

	NOTE: if input contains invalid values, NAN or INF, output will be invalid
			- in such cases it is recommended to interpolate the input before passing to this function

USES:
	Signal analysis

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	double *input   : pointer to array holding amplitude time series to be converted to RMS power
	double *output  : pointer to array holding the RMS result - calling function must allocate memory
	size_t nn      : length of the input & output arrays
	size_t nwin1   : length of the window in which to calculate RMS power
						- ideally 4-5 times the wavelength of a frequency of interest
						- this value will be increased by 1 if not an odd number, or decreased if this exceeds the data length

	char *message  : feedback returned to the calling function, which should allocate memory for this array (256 characters)

RETURN VALUE:
	0 on success
	-1 on error

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int xf_rms2_d(double *input, double *output, size_t nn, size_t nwin1, char *message) {

	char *thisfunc="xf_rms2_d\0";
	size_t ii,jj,kk,mm,nwin2,start,stop,oddnwin1=0;
	double sum,s1,s2,scale,dmm;

	sprintf(message,"%s (incomplete))",thisfunc);

 	/* CHECK IF ORIGINAL NWIN1 IS ODD */
	if(nwin1%2==0) oddnwin1=0; else oddnwin1=1;

	/* CHECK FOR INVALID INPUT */
	if(nn<3) { sprintf(message,"%s (input length %ld must not be less than 3)",thisfunc,nn); return(-1); }
	if(nwin1>nn) { sprintf(message,"%s (window size %ld exceeds input length %ld)",thisfunc,nwin1,nn); return(-1); }

	/* DETERMINE WORKING WINDOW SIZE AND HALF-WINDOW SIZE */
	/* if window size = entire input and nn is an even number, nwin2 must 1 less than half of nwin1   */
	if (nwin1==nn && (nn%2)==0) { nwin2= (nwin1/2) -1; }
	/* otherwise nwin2 is rounded down from half of nwin1 */
	else { nwin2= (size_t)( (double)nwin1/2.0 ); }
	/* make sure nwin1 is an odd number - if original nwin1 was even, this will increase the value by 1 */
	nwin1= nwin2+nwin2+1;

	/* CALCULATE THE SCALING FACTOR */
	scale=(double)1.0/(double)nwin1;

	//TEST:	fprintf(stderr,"nn=%ld nwin1=%ld nwin2=%ld\n",nn,nwin1,nwin2);


	/* RMS POWER FOR THE FIRST PORTION OF THE DATA */
	/* INCLUDES INITIALIZING THE RUNNING SUM OF SQUARES */
	start=0;
	stop=nwin2+1;
	dmm=(double)nwin2+1;

	/* pre-calculate the sum for the first window */
	sum=0.0; for(jj=start;jj<stop;jj++) { sum+= input[jj]*input[jj]; }
	output[start]=sqrt(sum/dmm);
	/* re-calculate the sum for each window up to and including the window centred on halfwin (trailing edge = input[0])*/
	start++;
	for(ii=start;ii<stop;ii++) {
		s2=input[ii+nwin2];
		sum+=s2*s2; /* add the number at the leading edge of the window */
		if(sum>0.0) output[ii]=sqrt(sum/++dmm);
		else output[ii]=0.0;
	}


	/* RMS POWER FOR THE MIDDLE PORTION OF THE DATA */
	start=nwin2+1;
	stop=nn-nwin2;
	jj=(start-nwin2-1);
	// calculate the sum for each window
	for(ii=start;ii<stop;ii++) {
		s1=input[jj++];
		s2=input[ii+nwin2];
		sum-=s1*s1; /* subtract the number just behind the trailing edge of the window */
		sum+=s2*s2; /* add the number at the leading edge of the window */
		if(sum>0.0) output[ii]=sqrt(sum*scale);
		else output[ii]=0.0;
	}


	/* RMS POWER FOR THE FINAL PORTION OF THE DATA */
	start=nn-nwin2;
	stop=nn;
	jj=(start-nwin2-1);
	dmm=(double)nwin1;
	// calculate the sum for each window
	for(ii=start;ii<stop;ii++) {
		s1=input[jj++];
		sum-=s1*s1; /* subtract the number just behind the trailing edge of the window */
		if(sum>0.0) output[ii]=sqrt(sum/--dmm);
		else output[ii]=0.0;
	}

	/* WRAP UP */
	if(oddnwin1==0) sprintf(message,"%s (calculated RMS power for %ld input points, adjusted window size %ld)",thisfunc,nn,nwin1);
	if(oddnwin1==1) sprintf(message,"%s (calculated RMS power for %ld input points, window size %ld)",thisfunc,nn,nwin1);
	return(0);
}
