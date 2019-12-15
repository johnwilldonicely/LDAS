/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION
	Remove linear trends from a data series

USES

ARGUMENTS
	float *y   : pointer to array holding input time-series, constant intervals assumed
	size_t nn  : number of elements in y[]
	double *result_d : array to hold results, initialized by calling function
		result_d[0]= intercept
		result_d[1]= slope
		result_d[2]= Pearson's r

RETURN VALUE
	0 on success
	1 on failure

*/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>

int xf_detrend1_f(float *y, size_t nn, double *result_d) {


//???STILL NEED TO FIX IF SLOPE =0

	size_t ii;
	double r=NAN,slope=NAN,intercept=NAN,aa,bb;
	double SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0;
	double SSx=0.0,SSy=0.0,SPxy=0.0;

	if(nn<2) return(1);

	for(ii=0;ii<nn;ii++) {
		aa=(double)ii;
		bb=(double)y[ii];
		SUMx += aa;
		SUMy += bb;
		SUMx2 += aa*aa;
		SUMy2 += bb*bb;
		SUMxy += aa*bb;
	}
	SSx = SUMx2-((SUMx*SUMx)/nn);
	SSy = SUMy2-((SUMy*SUMy)/nn);
	SPxy = SUMxy-((SUMx*SUMy)/nn);

	/* DETREND THE DATA, PROVIDED THERE IS VARIABILITY IN THE TIME SERIES */
	if(SSy!=0.0) {
		r = SPxy/(sqrt(SSx)*sqrt(SSy));
		slope= SPxy/SSx;
		intercept= (SUMy/nn)-(slope*(SUMx/nn));
		for(ii=0;ii<nn;ii++) {
			y[ii] -= (intercept + slope*(float)ii);
		}
	}
	/* OTHERWISE (OR IF SLOPE IS EXACTLY ZERO) SUBTRACT THE MEAN */
	else {
		r=0.0;
		slope=0.0;
		intercept=y[0];
		aa=SUMy/(float)nn;
		for(ii=0;ii<nn;ii++) {
			y[ii] -= aa;
		}
	}


	/* COPY COEFFICIENTS TO RESULTS ARRAY */
	result_d[0]=intercept;
	result_d[1]=slope;
	result_d[2]=r;

	/* FINISH */
	return(0);
}
