/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate the Paired 2-sample Student's t-value

USES:
	Testing statistical differences between two groups of data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data1: array holding the X-data
	double *data2: array holding the Y-data
	long n: number of data1/data2 pairs
	int varcalc: a flag specifying the method for calculating the variance
		1 = traditional fast computational formula
		2 = two-pass method, slower but not prone to overflow errors with very large data1sets
	double *result_d: array to hold result of calculations ( t= result_d[0] )

RETURN VALUE:
	 0: success
	-1: fewer than 2 values in one or both of the input data arrays
	-2: fewer than 2 valid values in one or both of the input data arrays
	-3: memory allocation error

SAMPLE CALL:
	xf_ttest3_d(data1,data2,n,1,result_d);
	t = result_d[0];
	df = result_d[1];
	good_data = result_d[2];
	mean_difference = result_d[3];

TO DO: rather than make a temporary array, if varcalc method is "1" we can do the calculations on the fly
TO DO: to facilitate this, introduce a new (unique) variable reflects the number of valid points
TO DO: make filtering for non_finite numbers an option - could save a step
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int xf_ttest3_d(double *data1, double *data2, long n, int varcalc, double *result_d) {

	int z;
	long i,j,n1=0,n2=0;
	double *tempdata1=NULL,*tempdata2=NULL;
	double aa,bb,dd,df,pooledvar,sd,t;
	double nn,nn2,sum1,ss1,meandiff,mean1,mean2,var1=0;

	/* INITIALIZE THE RESULTS ARRAY TO NAN */
	for(i=0;i<8;i++) result_d[i]=NAN;
	if(n<2) return(-1);

	/* BUILD TEMPORARY ARRAYS FROM DIFFERENCES BETWEEN VALID NUMERICAL PAIRS */
	if((tempdata1=(double *)realloc(tempdata1,n*sizeof(double)))==NULL) return(-3);
	mean1=mean2=0.0;
	for(i=j=0;i<n;i++) {
		z=0;
		if(isfinite(data1[i])) {
			mean1+=data1[i];
			n1++;
			z++;
		}
		if(isfinite(data2[i])) {
			mean2+=data2[i];
			n2++;
			z++;
		}

		if(z==2) tempdata1[j++]= data2[i]-data1[i];
	}
	mean1=mean1/(double)n1;
	mean2=mean2/(double)n2;

	n=j;


	/* IF THERE ARE NOT AT LEAST 2 VALID DATA POINTS IN EACH ARRAY, RETURN AN ERROR FLAG */
	if(n<2) { free(tempdata1); return(-2); }

	nn=(double)n;
	sum1=ss1=meandiff=0.0;

	// TRADITIONAL COMPUTATIONAL CALCULATION OF VARIANCE - FASTER
	if(varcalc==1) {
		meandiff=ss1=sum1=0.0;
		for(i=0;i<n;i++) {
			dd=tempdata1[i];
			sum1+=dd;
			ss1+=dd*dd; // NOTE: for large data1sets this value could exceed maximum
		}
		meandiff= sum1/nn;
		var1= ( ss1 - ( (sum1*sum1)/nn )) / (nn-1.0) ;
	}
	// TWO-PASS CALCULATION OF VARIANCE - SUMS THE ACTUAL DEVIATIONS FROM THE MEAN, COMPENSATES FOR ROUNDING ERROR - BETTER FOR VERY LARGE DATASETS
	if(varcalc==2) {
		meandiff=ss1=sum1=0.0;
		for(i=0;i<n;i++) meandiff += tempdata1[i];
		meandiff /= nn;
		for(i=0;i<n;i++) {
			dd=tempdata1[i]-meandiff;
			ss1 += dd*dd;
			sum1 += dd;
		}
		var1= ( ss1 - sum1*sum1 / nn)/(nn-1.0);
	}

	/* CALCULATE THE DEGREES OF FREEDOM df */
	df = (double)(n-1);

	/* CALCULATE THE STANDARD DEVIATION (SD) */
	sd= sqrt(var1);

	/* CALCULATE THE T-STATISTIC */
	t= meandiff / (sd / sqrt(nn)) ;

	/* COPY RESULTS OF CALCULATIONS TO THE RESULTS ARRAY */
	result_d[0]=t;
	result_d[1]=df;
	result_d[2]=nn; // valid data points - data 1 && 2
	result_d[3]=nn; // replication for consistency with ttest2 function
	result_d[4]=mean1;
	result_d[5]=mean2;
	result_d[6]=meandiff;
	result_d[7]=var1;
	result_d[8]=sd;

	/* FREE MEMORY AND RETURN THE REVISED SAMPLE-SIZE */
	free(tempdata1);
	return(0);
}
