/*
<TAGS>math signal_processing</TAGS>

DESCRIPTION:

	Convolute two complex input signals
	See xf_conv1_f for general principals

USES:
	Filtering sig1 using DSP parameters in sig2

DEPENDENCY TREE:

	No dependencies

ARGUMENTS:

	complex float *sig1 : pointer to input array sig1 (X)
	size_t n1           : length of sig1
	complex float *sig2 : pointer to input signal#1 (X)
	size_t n2           : length of sig2
	char message[]      : pointer to array to hold messages

RETURN VALUE:

	pointer to a results array with n1+(n2-1) elements
	NULL on failure

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

complex float *xf_conv2_f(complex float *sig1, size_t n1, complex float *sig2, size_t n2,char message[] ) {

	size_t ii,jj,kk,nresult;
	complex float *result=NULL;

	/* dertermine memory required for result  */
	nresult= n1 + (n2-1);

	/* allocate memory - calloc() will initialize to zero */
	result=(complex float *)calloc(nresult,sizeof(complex float));
	if(result==NULL) { sprintf(message,"xf_conv1_f: memory allocation error");return(NULL); }

	/* convolve */
	for(ii=0;ii<nresult;ii++) {
		for(jj=0;jj<n2;jj++) {
			kk=ii-jj;
			if(kk>=0 && kk<n1) result[ii] += sig2[jj] * sig1[kk] ;
		}
	}

	sprintf(message," xf_conv2_f: success");
	return (result);
}
