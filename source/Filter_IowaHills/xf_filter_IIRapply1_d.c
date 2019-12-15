/*
DESCRIPTION: 
	- Implement an IIR filter using second-order sections (biquads)
	- From: http://www.iowahills.com/A7ExampleCodePage.html (Sept 20, 2014)
	- derived from Iowa Hills code RunIIRBiquadForm1()
		- just use the form 1 biquad code
		- inline the biquad calculations (no need for another function call)
	- Form-1 Biquad
		- this uses 2 sets of shift registers, RegX on the input side and RegY on the output side.

NOTES FROM ORIGINAL IOWA HILLS CODE
	Remember that these filters have delay, so you need to run the code for M points
	longer than the number of data points to be filtered in order to get the entire
	signal through the filter. A reasonable value for M is twice the group delay value.
	e.g. 
	double Signal[1100], FilteredSignal[1100];
	for(j=0; j<1000; j++)Signal[j] = (double)random(2000)/1000.0 - 1.0;
	RunIIRBiquadForm1(Signal, FilteredSignal, 1008);

	In the code below, we simply set all the array sizes to 100 so that the
	code will work with any size filter. These filters can't have more
	than about 40 sections. The alternative would be to use malloc for all the arrays.

USES: 
	Filtering data to remove unwanted signals

DEPENDENCY TREE: 
	No dependencies
	
ARGUMENTS: 
	double *Input
	double *Output
	long NumSigPts : the length of the input array
	double *coefs : the array of coefficients to apply (6x NumSections)
	int NumSections : the number of biquad sections, typically PoleCount/2 for even PoleCounts
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result array will hold statistics
	char array will hold message (if any)
	
SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#define REG_SIZE 100
int xf_filter_IIRapply1_d(double *Input, double *Output, long NumSigPts, double *coefs, int NumSections, char *message) {

	char *thisfunc="xf_filter_IIRapply1_d\0"; 
	long ii,kk;
	double *a0,*a1,*a2,*b0,*b1,*b2; // 2nd order IIR coefficients
	double aa,bb,CenterTap;
	double RegX1[REG_SIZE], RegX2[REG_SIZE], RegY1[REG_SIZE], RegY2[REG_SIZE]; 

	
	/* INITIALIZE THE  USING THE 2ND ORDER IIR COEFFICIENTS USING THE *COEFS INPUT (SIZE=6*NUMSECTIONS) */
	a0=(double *) malloc(NumSections*sizeof(double)); if(a0==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	a1=(double *) malloc(NumSections*sizeof(double)); if(a1==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	a2=(double *) malloc(NumSections*sizeof(double)); if(a2==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	b0=(double *) malloc(NumSections*sizeof(double)); if(b0==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	b1=(double *) malloc(NumSections*sizeof(double)); if(b1==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	b2=(double *) malloc(NumSections*sizeof(double)); if(b2==NULL) {sprintf(message,"%s (memory allocation error)",thisfunc); return(-1);}
	for(kk=0;kk<NumSections;kk++) {
		a0[kk]=coefs[6*kk+0];
		a1[kk]=coefs[6*kk+1];
		a2[kk]=coefs[6*kk+2];
		b0[kk]=coefs[6*kk+3];
		b1[kk]=coefs[6*kk+5];
		b2[kk]=coefs[6*kk+5];
	}

	/* INITIALISE THE SHIFT REGISTERS. */
	for(ii=0; ii<REG_SIZE; ii++) RegX1[ii] = RegX2[ii] = RegY1[ii] = RegY2[ii] = 0.0;

	/* APPLY THE FILTER */
	for(ii=0; ii<NumSigPts; ii++) {
		/* calculate the output and initialize the register using the first section */
		aa= Input[ii];
		kk=0; 
		CenterTap = aa * b0[kk] + b1[kk] * RegX1[kk] + b2[kk] * RegX2[kk];
		bb = a0[kk] * CenterTap - a1[kk] * RegY1[kk] - a2[kk] * RegY2[kk];
		RegX2[kk] = RegX1[kk];
		RegX1[kk] = aa;
		RegY2[kk] = RegY1[kk];
		RegY1[kk] = bb;
		/* if there are additional sections, continue */
		for(kk=1; kk<NumSections; kk++) {	
			aa=bb; /* output from last iteration becomes input for this one */
			CenterTap = aa * b0[kk] + b1[kk] * RegX1[kk] + b2[kk] * RegX2[kk];
			bb = a0[kk] * CenterTap - a1[kk] * RegY1[kk] - a2[kk] * RegY2[kk];
			RegX2[kk] = RegX1[kk];
			RegX1[kk] = aa;
			RegY2[kk] = RegY1[kk];
			RegY1[kk] = bb;
		}
		/*set the output */
		Output[ii] = bb;
	}

	/* FREE THE MEMORY FOR THE 2ND ORDER IIR COEFFICIENTS AND RETURN */
	free(a0);free(a1);free(a2);free(b0);free(b1);free(b2);
	return(0);

}

