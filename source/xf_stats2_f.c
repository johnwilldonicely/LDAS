/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate summary statistics on an array of numbers (floating-point)
	Option to use high-precision calculation for large datasets (setlarge)
		- here the mean is first calculated by breaking the double-values into integer and fractional parts
		- then the individual differences from the  mean are calculated
		- the sums used for calculating the standard deviation are also similarly adjusted
	NOTE: no check for invalid values (NAN or INF)
USES:
	Getting the mean, stdev, etc. of a data set
DEPENDENCY TREE:
	No dependencies
ARGUMENTS:
	float *data1    : array holding the data
	long nn         : number of elements in the array
	int large       : a flag specifying the method for dealing with large datasets
		              1 = traditional fast computational formula
		              2 = two-pass method, slower but not prone to overflow errors with very large datasets
	float *result_f : array to hold result of calculations
RETURN VALUE:
	zero
SAMPLE CALL:
	xf_stats2_f (data,nn,1,result); mean=result[0];
*/

#include <math.h>

int xf_stats2_f(float *data1, long nn, int setlarge, float *result_f) {

	long ii,jj,kk,sumjj,sumkk,precision=1000000000;
	double aa,bb,cc,dd,ee,ff,nnd,min,max;
	double sum1,sum2,sum3,sumsquares,mean,var,sd,sem,skew;

	for(ii=0;ii<8;ii++) result_f[ii]=0.0;
	if(nn<1) return(0);

	nnd=(double)nn;
	sum1=sumsquares=var=0.0;
	mean=min=max= data1[0];

	/* TRADITIONAL COMPUTATIONAL CALCULATION OF VARIANCE - FASTER */
	if(setlarge==1 && nn>1) {
		sum1=0.0;
		for(ii=0;ii<nn;ii++) {
			dd= (double)data1[ii];
			sum1+= dd;
			sumsquares+= dd*dd; // NOTE: for large data1sets this value could exceed maximum
			if(dd<min)min= dd;
			if(dd>max)max= dd;
		}
		mean= sum1/nnd;
		var= ( sumsquares - ( (sum1*sum1)/nnd )) / (nnd-1.0) ;
	}

	/* TWO-PASS CALCULATION
		- better for very large datasets
		- sums the actual deviations from the mean
		- deviations used for both variance and corrected-mean (mean based on mean-normalized data)
	*/
	if(setlarge==2 && nn>1) {
		sumjj= sumkk= 0;
		sum2= sum3= 0.0;
		for(ii=0;ii<nn;ii++) {
			dd= (double)data1[ii];
			ff= modf(dd,&ee); // get fractional (ff) and integer (ee) parts
			jj= (long)ee; /* integer part */
			kk= (long)ff*precision; /* fractional part, multiplied by precision to a long integer */
			sumjj+= jj;
			sumkk+= kk;
			if(dd<min) min= dd;
			if(dd>max) max= dd;
		}

		/* reconstitute an accurate estimate of the mean, to 10 decimal places */
		mean= (double)((sumjj + sumkk/precision)/nn);

		/* use the mean to calculate differences */
		sum1= sum2= sum3= 0.0;
		for(ii=0;ii<nn;ii++) {
			dd= data1[ii]-mean;
			sum2+= dd*dd;
			sum3+= dd;
		}
		mean+= (sum3/nnd); // adjust the mean
		var= ( sum2 - sum3*sum3 / nnd)/(nnd-1.0);
	}

	/* CALCULATE SD & SEM */
	if(nn>1) { sd= sqrt(var); sem= sd/sqrt(nnd); }
	else var= sd= sem= 0.0;

	/* CALCULATE SKEW */
	if(nn>2 && sd>0.0 ) {
		bb=0.0;
		for(ii=0;ii<nn;ii++) {
			aa= data1[ii]-mean;
			bb+= aa*aa*aa;
		}
		if(bb>0.0) skew = (nn/((nn-1.0)*(nn-2.0))) * (bb/(sd*sd*sd));
		else skew= 0.0;
	}
	else skew= 0.0;

	/* CLEANUP AND RETURN */
	result_f[0]= (float)mean;
	result_f[1]= (float)var;
	result_f[2]= (float)sd;
	result_f[3]= (float)sem;
	result_f[4]= (float)min;
	result_f[5]= (float)max;
	result_f[6]= (float)skew;
	result_f[7]= (float)sum1;
	return(0);
}
