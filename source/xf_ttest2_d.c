/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate the Independent 2-sample Student's t-value

USES:
	Testing statistical differences between two groups of data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data1: array holding the X-data
	double *data2: array holding the Y-data
	long n1: number of elements in data1
	long n2: number of elements in data2
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
	xf_ttest2_d(data1,data2,n1,n2,1,result_d);
	t=result_d[0];

TO DO: rather than make a temporary array, if varcalc method is "1" we can do the calculations on the fly
TO DO: to facilitate this, introduce a new (unique) variable reflects the number of valid points
TO DO: make filtering for non_finite numbers an option - could save a step

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

int xf_ttest2_d(double *data1, double *data2, long n1, long n2, int varcalc, double *result_d) {

	long i,j;
	double *tempdata1=NULL,*tempdata2=NULL;
	double aa,bb,dd,df,pooledvar,pooledsd,t;
	double nn1,nn2,sum1,sum2,ss1,ss2,mean1,mean2,var1=0.0,var2=0.0;

	/* INITIALIZE THE RESULTS ARRAY TO NAN */
	for(i=0;i<12;i++) result_d[i]=NAN;
	if(n1<2 || n2<2) return(-1);

	/* BUILD TEMPORARY ARRAYS FROM ONLY THE VALID NUMERICAL DATA */
	if((tempdata1=(double *)realloc(tempdata1,n1*sizeof(double)))==NULL) return(-3);
	for(i=j=0;i<n1;i++) { if(isfinite(data1[i])) tempdata1[j++]=data1[i]; }
	n1=j;
	if((tempdata2=(double *)realloc(tempdata2,n2*sizeof(double)))==NULL) return(-3);
	for(i=j=0;i<n2;i++) { if(isfinite(data2[i])) tempdata2[j++]=data2[i]; }
	n2=j;

	/* IF THERE ARE NOT AT LEAST 2 VALID DATA POINTS IN EACH ARRAY, RETURN AN ERROR FLAG */
	if(n1<2) { free(tempdata1); return(-2); }
	if(n2<2) { free(tempdata2); return(-2); }

	nn1=(double)n1;
	nn2=(double)n2;
	sum1=sum2=ss1=ss2=mean1=mean2=0.0;

	// TRADITIONAL COMPUTATIONAL CALCULATION OF VARIANCE - FASTER
	if(varcalc==1) {
		mean1=ss1=sum1=0.0;
		for(i=0;i<n1;i++) {
			dd=tempdata1[i];
			sum1+=dd;
			ss1+=dd*dd; // NOTE: for large data1sets this value could exceed maximum
		}
		mean1= sum1/nn1;
		var1= ( ss1 - ( (sum1*sum1)/nn1 )) / (nn1-1.0) ;

		mean2=ss2=sum2=0.0;
		for(i=0;i<n2;i++) {
			dd=tempdata2[i];
			sum2+=dd;
			ss2+=dd*dd; // NOTE: for large data2sets this value could exceed maximum
		}
		mean2= sum2/nn2;
		var2= ( ss2 - ( (sum2*sum2)/nn2 )) / (nn2-1.0) ;

	}
	// TWO-PASS CALCULATION OF VARIANCE - SUMS THE ACTUAL DEVIATIONS FROM THE MEAN, COMPENSATES FOR ROUNDING ERROR - BETTER FOR VERY LARGE DATASETS
	if(varcalc==2) {
		mean1=ss1=sum1=0.0;
		for(i=0;i<n1;i++) mean1 += tempdata1[i];
		mean1 /= nn1;
		for(i=0;i<n1;i++) {
			dd=tempdata1[i]-mean1;
			ss1 += dd*dd;
			sum1 += dd;
		}
		var1= ( ss1 - sum1*sum1 / nn1)/(nn1-1.0);

		mean2=ss2=sum2=0.0;
		for(i=0;i<n2;i++) mean2 += tempdata2[i];
		mean2 /= nn2;
		for(i=0;i<n2;i++) {
			dd=tempdata2[i]-mean2;
			ss2 += dd*dd;
			sum2 += dd;
		}
		var2= ( ss2 - sum2*sum2 / nn2)/(nn2-1.0);

	}

	/* CALCULATE THE DEGREES OF FREEDOM df */
	df = (double)n1+n2-2;

	/* CALCULATE THE STANDARD DEVIATION (SD) AND STANDARD ERROR OF THE mean1 (SEM) */
	pooledvar= ( (n1-1)*var1 + (n2-1)*var2 ) / df;
	pooledsd= sqrt(pooledvar);

	/* CALCULATE THE T-STATISTIC */
	t= (double)(mean2-mean1) / (pooledsd * sqrt((1.0/nn1) + (1.0/nn2)));

	/* COPY RESULTS OF CALCULATIONS TO THE RESULTS ARRAY */
	result_d[0]=t;
	result_d[1]=df;
	result_d[2]=nn1;
	result_d[3]=nn2;
	result_d[4]=mean1;
	result_d[5]=mean2;
	result_d[6]=mean2-mean1;
	result_d[7]=var1;
	result_d[8]=var2;
	result_d[9]=pooledvar;

	/* FREE MEMORY AND RETURN THE REVISED SAMPLE-SIZE */
	free(tempdata1);
	free(tempdata2);
	return(0);
}
