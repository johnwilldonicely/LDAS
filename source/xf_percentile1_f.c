/*
<TAGS>stats</TAGS>
AUTHORS
	John Huxter, 4 February 2011
	updated 11 August 2011: now uses the GNU qsort function
	updated 4 September 2012: remove reference to FUNC_NAME
	updated 6 October 2012 - eliminate use of "message" string to store error messages
	updated 22 October 2017 - now will ignore non-finite numbers (INF or NAN)

DESCRIPTION:
	Calculate percentile cutoffs for a distribution of double values

USES:
	Getting the median, finding outliers in a distribution

DEPENDENCIES:
	xf_compare1_d

ARGUMENTS:
	float *data   : array holding the input data
	long nn       : number of elements in the array
	double *result: a minimum 16-element array to hold the percentile results
	   result[0]  : 1st percentile
	   result[1]  : 2.5th percentile
	   result[2]  : 5th percentile
	   result[3]  : 10th percentile
	   result[4]  : 25th percentile
	   result[5]  : 50th percentile (median)
	   result[6]  : 75th percentile
	   result[7]  : 90th percentile
	   result[8]  : 95th percentile
	   result[9]  : 97.5th percentile
	   result[10] : 99th percentile

RETURN VALUE:
	0  if successful, -1 if fail

SAMPLE CALL:
	if ( xf_percentile1_d(data,n,result) >=0) median=result[5];
	else {
	 fprintf(stderr,"--- Error: Memory allocation error\n"); exit(1);
 	}
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* external compare function required by qsort - in this instance, comparing type double */
int xf_compare1_d(const void *a, const void *b);

int xf_percentile1_f(float *data, long nn, double *result) {

	long int ii,jj,kk,n2;
	double aa,bb,cc, *temp=NULL;
	double perclist[11]={.01,.025,.05,.1,.25,.5,.75,.90,.95,.975,.99};

	/* initialize results */
	for(ii=0;ii<11;ii++) result[ii]=0.0;

	/* build a temporary array - omit non-finite numbers */
	temp= malloc((nn+1)*sizeof(*temp));
	if(temp==NULL) return(-1);
	for(ii=n2=0;ii<nn;ii++) if(isfinite(data[ii])) temp[n2++]= (double)data[ii];

	/* sort the temporary array */
	qsort(temp,n2,sizeof(double),xf_compare1_d);

	/* build the percentile list */
	for(ii=0;ii<11;ii++) {
		aa= n2*perclist[ii];  /* find the position in the array corresponding to the current percentile */
		jj= (long)aa;	      /* convert this to an integer element-number */
		if(aa!=(double)jj) result[ii]= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
		else result[ii]= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	}

	free(temp);
	return(0);
}
