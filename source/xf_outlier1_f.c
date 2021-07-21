/*
<TAGS>signal_processing transform</TAGS>

DESCRIPTION:
	- remove outliers (lower and/or higher) from a dataset (convert to NAN)
	- NAN and INF values are left unchanged
	- just like the double version of this function but the input is 32-bit float (the calculations are all 64-bit)

DEPENDENCIES:
	int xf_compare1_d(const void *a, const void *b)

ARGUMENTS:
	float *data    : input: data array
	long nn        : input: length of data
	float setlow   : input: low-percentile cutoff (0-100, or -1 to skip)
	float sethigh  : input: high-percentile cutoff (0-100, or -1 to skip)
	char *message) : character array holding error message, if any, for return value of -1 (see below)

RETURN VALUE:
	- success: number of trimmed values
	- failure: -, and message array will hold description of error

SAMPLE CALL:
	x= xf_outlier1_f(data,nn,5.0,95.0,message);
	if(x!=0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int xf_compare1_d(const void *a, const void *b);

long xf_outlier1_f(float *data, long nn, float setlow, float sethigh, char *message) {

	char *thisfunc="xf_outlier1_d\0";
	long ii,jj,kk,n2;
	double aa,bb,lowval,highval, *temp=NULL;

	if(setlow==0.00 && sethigh==100.0) return(0);
	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,nn); return(-1); }
	if(setlow<0.0||setlow>100.0) { sprintf(message,"%s [ERROR]: invalid low-percentile (%g), must be 0-100",thisfunc,setlow); return(-1); }
	if(sethigh<0.0||sethigh>100.0) { sprintf(message,"%s [ERROR]: invalid high-percentile (%g), must be 0-100",thisfunc,sethigh); return(-1); }
	if(setlow>sethigh) { sprintf(message,"%s [ERROR]: low-percentile (%g) is greater than high-percentile (%g)",thisfunc,setlow,sethigh); return(-1); }

	/* BUILD A SORTED ARRAY */
	/* build a temporary array - omit non-finite numbers */
	temp= malloc((nn+1)*sizeof(*temp));
	if(temp==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(-1); }
	for(ii=n2=0;ii<nn;ii++) if(isfinite(data[ii])) temp[n2++]= data[ii];
	if(n2<1) return(0); // no error, just don't do anything

	/* SORT THE TEMPORARY ARRAY */
	qsort(temp,n2,sizeof(double),xf_compare1_d);

	/* GET VALUES REPRESENTING THE LOW AND HIGH LIMITS - THIS CODE DERIVED FROM xf-percentile2_f */
	if(setlow==0.0) lowval=temp[0];
	else if(setlow==100.0) lowval=temp[(n2-1)];
	else {
		aa= n2*(setlow/100);
		jj= (long)aa;
		if(aa!=(double)jj) lowval= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
		else lowval= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	}
	if(sethigh==0.0) highval=temp[0];
	else if(sethigh==100.0) highval=temp[(n2-1)];
	else {
		aa= n2*(sethigh/100);
		jj= (long)aa;
		if(aa!=(double)jj) highval= temp[jj]; /* if the percentile limit does not fall exactly on an item, cutoff is unchanged */
		else highval= (temp[jj]+temp[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */
	}

	/* SET OUTLIERS IN THE ORIGINAL DATASET TO NAN */
	for(ii=kk=0;ii<nn;ii++) if(data[ii]<lowval || data[ii]>highval) {data[ii]= NAN; kk++;}

	/* CLEANUP AND RETURN NUMBER OF OUTLIERS SET TO NAN  */
	if(temp!=NULL) free(temp);
	return(kk);
}
