/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Calculate a percentile cutoff for an array of float values
	- this version calculates the specified percentile and its inverse
	- example, if user specified "5", then results for the 5% and 95% values are produced

USES:
	Getting the median, finding outliers in a distribution

DEPENDENCIES:
	xf_compare1_d

ARGUMENTS:
	float *data   : array holding the input data
	long nn       : number of elements in the array
	double setper : the percentile cutoff desired (0-100)
	double *per1  : the result (pass as address to variable)
	double *per2  : as above, but the mirror-image percentile (e.g. 95% for 5%, 99% for 1%, and so on)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0  if successful, -1 if fail

SAMPLE CALL:
	z= xf_percentile3_f(data, nn, 2.5, &aa, &bb, message;
	if(z!=0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
	printf("set percentile = %g\n",aa);
	printf("inverse percentile = %g\n",bb);

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* external compare function required by qsort - in this instance, comparing type double */
int xf_compare1_d(const void *a, const void *b);

int xf_percentile3_f(float *data, long nn, double setper, double *per1, double *per2, char *message) {

	char *thisfunc="xf_percentile3_f\0";
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

	/* get the percentile value */
	if(setper==0) bb= temp[0];
	else {
		aa= n2*(setper/100); /* find the position in the array corresponding to the percentile */
		jj= (long)aa;  /* convert this to an integer element-number */
		if(aa!=(double)jj) bb= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
		else bb= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	}
	*per1= bb;

	/* now get the inverse-percentile value - e.g. if the 5th percentile was requested, this will be the 95th percentile */
	if(setper==100) bb= temp[(n2-1)];
	else {
		aa= n2*((100-setper)/100); /* find the position in the array corresponding to the percentile */
		jj= (long)aa;  /* convert this to an integer element-number */
		if(aa!=(double)jj) bb= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
		else bb= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	}
	*per2= bb;
	//TEST:fprintf(stderr,"min=%g  max=%g\nsetper=%g\nper1=%g  per2=%g\n",temp[0],temp[(n2-1)],setper,*per1,*per2);

END:
	if(temp!=NULL) free(temp);
	return(0);
}
