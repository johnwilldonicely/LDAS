/*
<TAGS>math signal_processing</TAGS>

DESCRIPTION:

	Convolute two input signals

	Given signal 1 (X) and signal 2 (Y), this algorithm effectively slides a
	reversed version of Y[] along X[], multiplying each Y by the corresponding
	X, provided it exists, and summing the products to create a result.

	Example:

		signal1 (X) consists of seven numbers (0,1,2,3,4,5,6), so nX=7
		signal2 (Y) consists of three numbers (0,1,2), so nY=3

		The first row shows X[]
		Subsequent rows show Y[], reversed & shifted along X[]
		The result for each X is the sum (looking back) of Y*X
		At the end of X[] two additional values can be computed
		Total results therefore = nX+(nY-1) = in this case 7+(3-1) = 9
		However, you could ignore the last nY-1 results
		Missing values denoted by "_"

	_ _ 0 1 2 3 4 5 6 _ _

	2 1 0                  - result[0] = 2x_ + 1x_ + 0x0
	  2 1 0                - result[1] = 2x_ + 1x0 + 0x1
	    2 1 0              - result[2] = 2x0 + 1x1 + 0x2
		  2 1 0            - result[3] = 2x1 + 1x2 + 0x3
	...
	            2 1 0      - result[6] = 2x4 + 1x5 + 0x6
	              2 1 0    - result[7] = 2x5 + 1x6 + 0x_
	                2 1 0  - result[8] = 2x6 + 1x_ + 0x_



USES:

	Filtering sig1 using DSP parameters in sig2

DEPENDENCY TREE:

	No dependencies

ARGUMENTS:

	float *sig1 	: pointer to input array sig1 (X)
	size_t n1		: length of sig1
	float *sig2 	: pointer to input signal#1 (X)
	size_t n2		: length of sig2
	char message[]  : pointer to array to hold messages

RETURN VALUE:

	pointer to a results array with n1+(n2-1) elements
	NULL on failure

SAMPLE CALL:

	#define XDIM 320
	#define HDIM 60

	float *xf_conv1_f(float *sig1, size_t n1, float *sig2, size_t n2,char message[] );

	int main (int argc, char *argv[]) {

		size_t ii;
		float   fX[XDIM], fY[XDIM], fH[HDIM], *result=NULL;

		// DEFINE THE INPUT SIGNAL
		for (ii = 0; ii < XDIM; ii++) fX[ii]=0;
		for (ii = HDIM; ii < XDIM; ii++) fX[ii] = sin(2.0 * M_PI * (float)ii / HDIM) + sin(2.0 * M_PI * (float)ii / 10.0);
		// DEFINE IMPULSE RESPONSE (MAX NO. OF TERMS = HDIM)
		for (ii = 0; ii < HDIM; ii++) fH[ii]=0;
		for (ii = 1; ii <= 10; ii++) fH[ii] = 0.1;

		result= xf_conv1_f(fX,XDIM,fH,HDIM,message);

		for(ii=0;ii<(n1+(n2-1));ii++) printf("%d %f\n",ii,result[ii]);

		free(result);
		exit(0);

	}


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float *xf_conv1_f(float *sig1, size_t n1, float *sig2, size_t n2,char message[] ) {

	size_t ii,jj,kk,nresult;
	float *result=NULL;

	/* dertermine memory required for result  */
	nresult= n1 + (n2-1);

	/* allocate memory - calloc() will initialize to zero */
	result=(float *)calloc(nresult,sizeof(float));
	if(result==NULL) { sprintf(message,"xf_conv1_f: memory allocation error");return(NULL); }

	/* convolve */
	for(ii=0;ii<nresult;ii++) {
		for(jj=0;jj<n2;jj++) {
			kk=ii-jj;
			if(kk>=0 && kk<n1) result[ii] += sig2[jj] * sig1[kk] ;
		}
	}

	sprintf(message," xf_conv1_f: success");
	return (result);
}
