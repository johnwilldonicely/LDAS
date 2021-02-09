/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate the Pearson's correlation and return details including F and p statistics
	- this version for double-precision floating-point values of x and y
	Fills a "results" array with details from a Pearson's correlation
	Allows definition of an arbitrary "invalid" value to be ignored
	Assumes all data is stored in memory
	For faster processing with less analysis detail, use xf_correlate_simple_d

USES:
	Detailed correlation statistics on parallel arrays of data

DEPENDENCIES:
	float xf_prob_F(float F,int df1,int df2)

ARGUMENTS:
	double *x      : input, x-data array
	double *y      : input, y-data array
	long nn        : input, number of elements in x & y
	double setinv  : input, user-specified invalid value (typically NAN, but may be other)
	double *result : output, pre-allocated array to hold results - must allow at least 18 elements
	char *message  : output, pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error

	result array will hold statistics
	char array will hold message (if any)

	if r==0:
		check message (minimum correlation) - likely a vertical or horizontal line
		probability of "nan" will confirm this

	if F==99:
		indicates that r was nearly exxactly -1 or +1: F is arbitrarily assigned a large value

SAMPLE CALL:
	x=  xf_correlate_d(x,y,n,-1,result,message);
	if(x==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	else if (result[1]==0) { fprintf(stderr,"*** %s/%s\n\n",thisprog,message);}
*/


#include<stdlib.h>
#include<stdio.h>
#include<math.h>

int xf_correlate_d(double *x, double *y, long nn, double setinv, double *result, char *message) {

	/* define external dependency */
	float xf_prob_F(float F,int df1,int df2);

	/* define internal variables */
	char *thisfunc="xf_correlate_d\0";
	long  ii,dfa,dfb,N_valid=0;
	double tempx,tempy, prob,b0,b1;
	double SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0,MEANx=0.0,MEANy=0.0;
	double SSx=0.0,SSy=0.0,SDx=0.0,SDy=0.0,SSreg=0.0,SSres=0.0,SPxy=0.0;
	double r=0.0,r2=0.0,r2adj=0.0,F=0.0;

	for(ii=0;ii<=18;ii++) result[ii]=0.0;

	/********************************************************************************/
	/* SCAN THE INPUT DATA */
	/********************************************************************************/
	/* option1: invalid value specified is a finite value - look for non-matches */
	if(isfinite(setinv)) {
		for(ii=0;ii<nn;ii++) {
			tempx= x[ii];
			tempy= y[ii];
			if(tempx!=setinv && tempy!=setinv) {
				N_valid++;
				SUMx += tempx;
				SUMy += tempy;
				SUMx2 += tempx*tempx;
				SUMy2 += tempy*tempy;
				SUMxy += tempx*tempy;
	}}}
	/* option2: invalid value specified is non-finite (NAN or INF) - look for finite values  */
	else {
		for(ii=0;ii<nn;ii++) {
			tempx= x[ii];
			tempy= y[ii];
			if(isfinite(tempx) && isfinite(tempy)) {
				N_valid++;
				SUMx += tempx;
				SUMy += tempy;
				SUMx2 += tempx*tempx;
				SUMy2 += tempy*tempy;
				SUMxy += tempx*tempy;
	}}}

	/********************************************************************************/
	/* MAKE SURE THERE WERE AT LEAST 4 VALID DATA POINTS   */
	/********************************************************************************/
	if(N_valid<4) { sprintf(message,"%s [ERROR: less than 4 valid data points - no correlation possible]",thisfunc); return(-1); }

	/********************************************************************************/
	/* CALCULATE BASIC STATISTICS  */
	/********************************************************************************/
	MEANx = SUMx/N_valid;
	MEANy = SUMy/N_valid;
	SSx   = SUMx2-((SUMx*SUMx)/N_valid);
	SSy   = SUMy2-((SUMy*SUMy)/N_valid);
	SPxy  = SUMxy-((SUMx*SUMy)/N_valid);
	SDx   = sqrt(SSx/(N_valid-1.0));
	SDy   = sqrt(SSy/(N_valid-1.0));
	dfa   = 1;
	dfb   = N_valid-2;

	/********************************************************************************/
	/* CALCULATE R AND REGRESSION COEFFICIENTS b1 (SLOPE) AND b0 (INTERCEPT)  */
	/* - apply corrections for vertical or horizontal lines */
	/********************************************************************************/
	/* if data describes horizontal line... */
	if(SSy==0.0) {
		r=0;
		b1=0.0;   // slope
		b0=MEANy; // y-intercept
		sprintf(message,"%s [WARNING: data defines a horizontal line]\n",thisfunc);
	}
	/* if data describes vertical line (or single point) ... */
	else if(SSx==0.0) {
		r=0;
		b1=NAN;
		b0=NAN;
		sprintf(message,"%s [WARNING: data defines a vertical]\n",thisfunc);
	}
	/* otherwise, if there is variability in both x and y... */
	else {
		r = SPxy/(sqrt(SSx)*sqrt(SSy));
		b1= SPxy/SSx;
		b0= (SUMy/nn)-(b1*(SUMx/nn));
	}

	/* now calculate the remaining principal statistics */
	r2    = r*r;
	r2adj = 1.0-((1.0-r2)*(double)(N_valid-1)/(double)(N_valid-2));
	F     = (r2*dfb)/(1-r2);


	/********************************************************************************/
	/* CALCULATE PROBABILITY */
	/********************************************************************************/
	/* do not pass extreme values to the probability function - just assume it's a perfect correlation */
	if(r>=.999999||r<=-.999999) {
		F=999999.0;
		prob=0.0;
	}
	/* make sure the F-value is non-zero, there are valid degrees-of-freedom,and there is variance in x */
	else {
		if(F>0 && dfa>0 && dfb>0 && SDx!=0) {
			/* call the external probability function */
			prob = xf_prob_F(F,dfa,dfb);
		}
		else {
			prob=NAN;
		}
	}

	/********************************************************************************/
	/* FILL THE RESULTS ARRAY  */
	/********************************************************************************/
	result[0] = (double) N_valid;
	result[1] = (double) r;	/* Pearson's r */
	result[2] = (double) r2; /* r-squared */
	result[3] = (double) r2adj; /* adjusted r-squared */
	result[4] = (double) F;
	result[5] = (double) dfa;
	result[6] = (double) dfb;
	result[7] = (double) prob; /* p-value (probability) */
	result[8] = (double) MEANx;
	result[9] = (double) MEANy;
	result[10] = (double) SSx;
	result[11] = (double) SSy;
	result[12] = (double) SPxy;
	result[13] = (double) SDx;
	result[14] = (double) SDy;
	result[15] = (double) b1; /* slope */
	result[16] = (double) b0; /* intercept */
	result[17] = (double) N_valid;

	return(0);
}
