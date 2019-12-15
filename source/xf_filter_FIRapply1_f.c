/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Apply a Finite Impulse Response (FIR) filter to an array of data
	Overwrites input array
USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data : input holding data
	long nn : size of data array
	double *coefs :  the set of FIR filter coefficients, pre-calculated by calling function
	int ncoefs : the number of coefficients
	int shift : correct data for phase-shift? 0=NO, 1=YES, 2=YES+start-up padding (recommended)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	data array will be modified
	char array will hold message (if any)

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>

int xf_filter_FIRapply1_f(float *data, long nn, double *coefs, int ncoefs, int shift, char *message) {

	char *thisfunc="xf_filter_FIRapply1_f\0";
	int top;
	long ii,jj,kk,delay;
	double aa;
	double *reg;

	reg= calloc(ncoefs,sizeof(double));
	if(reg==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }

	/* apply the filter */
	top=0;
	for(ii=0; ii<nn; ii++) {
		reg[top]= (double)data[ii];
		kk= 0;
		aa= 0.0;
		// The coefficient index increases while the reg index decreases.
		for(jj=top;jj>=0;jj--) aa+= coefs[kk++] * reg[jj];
		for(jj=ncoefs-1;jj>top;jj--) aa+= coefs[kk++] * reg[jj];

		data[ii]=(float)aa;
		if(++top >= ncoefs) top= 0;
	}

	/* correct for shift in data and pad end of data with the last sample */
	if(shift>0) {
		delay=(long)((ncoefs-1)/2);
		jj=delay;
		kk=nn-delay;
		for(ii=0; ii<kk; ii++) data[ii]=data[jj++];
		ii--;
		for(jj=kk;jj<nn; jj++) data[jj]=data[ii];
		// further correction for potential start-up artefact
		if(shift>1) for(ii=0; ii<delay; ii++) data[ii]=data[delay];
	}

	free(reg);
	return(0);
}
