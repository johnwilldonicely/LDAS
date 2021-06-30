/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate a percentile cutoff for an array of float values

USES:
	Getting the median, finding outliers in a distribution

DEPENDENCIES:
	xf_compare1_f

ARGUMENTS:
	float *data   : array holding the input data
	long nn       : number of elements in the array
	double setper : the percentile cutoff desired (0-1)
	double *per1  : the result (pass as address to variable)
	double *per2  : as above, but the mirror-image percentile (e.g. 95% for 5%, 99% for 1%, and so on)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0  if successful, -1 if fail

SAMPLE CALL:
	if ( xf_percentile1_d(data,n,result) >=0) median=result[5];
	else { fprintf(stderr,"** Memory allocation error\n"); exit(1); }
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* external compare function required by qsort - in this instance, comparing type double */
int xf_compare1_d(const void *a, const void *b);

int xf_percentile2_f(float *data, long nn, double setper, double *per1, double *per2, char *message) {

	char *thisfunc="xf_percentile2_f\0";
	long int ii,jj,kk,n2;
	double aa,bb, *temp=NULL;

	/* check validity of arguments */
	if(nn==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,nn); return(-1); }
	if(setper<0.0||setper>100.0) { sprintf(message,"%s [ERROR]: invalid percentile (%g), must be 0-100",thisfunc,setper); return(-1); }

	/* build a temporary array - omit non-finite numbers */
	temp= malloc((nn+1)*sizeof(*temp));
	if(temp==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }
	for(ii=n2=0;ii<nn;ii++) if(isfinite(data[ii])) temp[n2++]= data[ii];

	/* sort the temporary array */
	qsort(temp,n2,sizeof(double),xf_compare1_d);

	/* get the ercentile value */
	aa= n2*(setper/100); /* find the position in the array corresponding to the percentile */
	jj= (long)aa;  /* convert this to an integer element-number */
	if(aa!=(double)jj) bb= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
	else bb= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	*per1= bb;

	/* now get the inverse-percentile value - e.g. if the 5th percentile was requested, this will be the 95th percentile */
	aa= n2*((100-setper)/100); /* find the position in the array corresponding to the percentile */
	jj= (long)aa;  /* convert this to an integer element-number */
	if(aa!=(double)jj) bb= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
	else bb= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	*per2= bb;

END:
	if(temp!=NULL) free(temp);
	return(0);
}
