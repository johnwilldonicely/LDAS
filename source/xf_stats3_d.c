/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate summary statistics on an array of numbers (double-precision floating-point)
	Similar to xf_stats2_d, but NAN or INF values are treated as missing values and do not affect results

USES:
	Getting the mean, stdev, etc. of a data set

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data1: array holding the data
	long n1: number of elements in the array
	int setlarge: a flag specifying the method for calculating the mean and variance
		1 = traditional fast computational formula
		2 = two-pass method, slower but not prone to overflow errors with very large datasets
	double *result_d: array to hold result of calculations

RETURN VALUE:
	The number of valid data-points used in calculations (finite numbers, not NAN or INF)
	On error:
		-1: insufficient number of samples
		-2: memory allocation error
		-3: no valid numbers in data

SAMPLE CALL:
	xf_stats3_d(data,n1,1,result); mean=result[0];
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

long xf_stats3_d(double *data1, long n1, int setlarge, double *result_d) {

	long ii,jj,kk,n2,sumjj,sumkk,precision=1000000000;
	double *tempdata=NULL;
	double aa,bb,cc,dd,ee,ff,min,max;
	double nnd,sum1,sum2,sum3,sumsquares,mean,var,sd,sem,skew;

	/* INITIALIZE THE RESULTS ARRAY TO NAN */
	for(ii=0;ii<8;ii++) result_d[ii]=NAN;

	/* MAKE SURE THERE IS SOME DATA! */
	if(n1<1) return(-1);

	/* BUILD TEMPORARY DATA ARRAY FROM ONLY THE VALID NUMERICAL DATA */
	if((tempdata= realloc(tempdata,n1*sizeof(*tempdata)))==NULL) return(-2);
	for(ii=jj=0;ii<n1;ii++) { if(isfinite(data1[ii])) tempdata[jj++]= data1[ii]; }
	/* define valid-data-n - if there is no valid data, return an error message*/
	n2=jj; if(n2<1) { free(tempdata); return(-3); }
	/* make a double-sized version of n2 */
	nnd= (double)n2;

	sum1=sumsquares=var=0.0;
	mean=min=max=tempdata[0];

	/* TRADITIONAL COMPUTATIONAL CALCULATION OF VARIANCE - FASTER */
	if(setlarge==1&&n2>1) {
		sum1=0.0;
		for(ii=0;ii<n2;ii++) {
			dd= tempdata[ii];
			sum1+= dd;
			sumsquares+= dd*dd; // NOTE: for large datasets this value could exceed maximum
			if(dd<min) min= dd;
			if(dd>max) max= dd;
		}
		mean= sum1/nnd;
		var= ( sumsquares - ( (sum1*sum1)/nnd )) / (nnd-1.0) ;
	}

	/* TWO-PASS CALCULATION
	- better for very large datasets
	- sums the actual deviations from the mean
	- deviations used for both variance and corrected-mean (mean based on mean-normalized data)
	*/
	if(setlarge==2 && n2>1) {
		sumjj= sumkk= 0;
		sum2= sum3= 0.0;
		for(ii=0;ii<n2;ii++) {
			dd= tempdata[ii];
			ff= modf(dd,&ee); // get fractional (ff) and integer (ee) parts
			jj= (long)ee; /* integer part */
			kk= (long)ff*precision; /* fractional part, multiplied by precision to a long integer */
			sumjj+= jj;
			sumkk+= kk;
			if(dd<min) min= dd;
			if(dd>max) max= dd;
		}

		/* reconstitute an accurate estimate of the mean, to 10 decimal places */
		mean= (double)((sumjj + sumkk/precision)/n2);

		/* use the mean to calculate differences */
		sum2= sum3= 0.0;
		for(ii=0;ii<n2;ii++) {
			dd= tempdata[ii]-mean;
			sum2+= dd*dd;
			sum3+= dd;
		}
		mean+= (sum3/nnd); // adjust the mean
		var= ( sum2 - sum3*sum3 / nnd)/(nnd-1.0);
	}

	/* CALCULATE THE STANDARD DEVIATION (SD) AND STANDARD ERROR OF THE MEAN (SEM) */
	if(n2>1) { sd= sqrt(var); sem= sd/sqrt(nnd); }
	else var=sd=sem=0.0;

	/* CALCULATE THE SKEW */
	if(n2>2 && sd>0.0 ) {
		bb=0.0;
		for(ii=0;ii<n2;ii++) { aa=tempdata[ii]-mean; bb+=aa*aa*aa; }
		if(bb>0.0) skew= (n2/((n2-1.0)*(n2-2.0))) * (bb/(sd*sd*sd));
		else skew=0.0;
	}
	else skew=0.0;

	/* COPY RESULTS OF CALCULATIONS TO THE RESULTS ARRAY */
	result_d[0]= mean;
	result_d[1]= var;
	result_d[2]= sd;
	result_d[3]= sem;
	result_d[4]= min;
	result_d[5]= max;
	result_d[6]= skew;
	result_d[7]= sum1;

	/* FREE MEMORY AND RETURN THE REVISED SAMPLE-SIZE */
	free(tempdata);
	return(n2);
}
