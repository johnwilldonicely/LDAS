/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Apply a Finite Impulse Response (FIR) filter to an array of input
	Separate input and output arrays (input preserved)
USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *input : array holding input
	double *ouput : array (pre-allocated) to hold onput
	long nn : size of input array
	double *input : input holding input
	double *coefs :  the set of FIR filter coefficients, pre-calculated by calling function
	int ncoefs : the number of coefficients
	int shift : correct data for phase-shift? 0=NO, 1=YES, 2=YES+start-up padding (recommended)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	output array will be overwritten
	char array will hold message (if any)

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>

int xf_filter_FIRapply2_f(float *input, float *output, long nn, double *coefs, int ncoefs, int shift, char *message) {

	char *thisfunc="xf_filter_FIRapply2_f\0";
	int top;
	long ii,jj,kk,delay;
	double aa;
	double *reg;

	reg= calloc(ncoefs,sizeof(double));
	if(reg==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }

	/* apply the filter */
	top=0;
	for(ii=0; ii<nn; ii++) {
		reg[top]= (double)input[ii];
		kk= 0;
		aa= 0.0;
		// The coefficient index increases while the reg index decreases.
		for(jj=top;jj>=0;jj--) aa+= coefs[kk++] * reg[jj];
		for(jj=ncoefs-1;jj>top;jj--) aa+= coefs[kk++] * reg[jj];

		output[ii]=(float)aa;
		if(++top >= ncoefs) top= 0;
	}
	/* correct for shift in data and pad end of data with the last sample */
	if(shift==1) {
		delay=(long)((ncoefs-1)/2);
		jj=delay;
		kk=nn-delay;
		for(ii=0; ii<kk; ii++) output[ii]=output[jj++];
		ii--;
		for(jj=kk;jj<nn; jj++) output[jj]=output[ii];
		/* further correction for potential start-up artefact */
		if(shift>1) for(ii=0; ii<delay; ii++) output[ii]=output[delay];
	}

	free(reg);
	return(0);
}
