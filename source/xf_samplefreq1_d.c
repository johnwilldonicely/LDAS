/*
<TAGS>signal_processing time</TAGS>
AUTHORS
	John R. Huxter, 4 April 2015

DESCRIPTION:
	Calculate sample frequency for a series of timestamps ( 1 / median_sample_interval )

USES:


DEPENDENCIES:
	xf_compare1_d

ARGUMENTS:
	double *time: array holding the time-stamps
	long n1: number of elements in the time array - must be >1
	char *message: string to hold error message on fail

RETURN VALUE:
	sample frequency, 0 if fail

SAMPLE CALL:
	samplefreq= xf_samplefreq1_d(time,n1,message);
	if(samplefreq==0) {
		fprintf(stderr,"Error: %s \n",message);
		exit(1);
	}
*/

#include <stdlib.h>
#include <stdio.h>

/* external compare function required by qsort - in this instance, comparing type double */
int xf_compare1_d(const void *a, const void *b);

double xf_samplefreq1_d(double *time1, long n1, char *message) {

	char *thisfunc="xf_samplefreq1_d\0";
	long ii,jj,n2=1001;
	double a,*inter=NULL,median,sampfreq;

	if(n1<2) {sprintf(message,"%s [ERROR]: cannot calculate interval for less than two time-stamps",thisfunc);return(-1.0);}

	/* array for intervals is initialized for 1001 elements, unless this exceeds the length of the time array */
	if(n2>n1) n2=n1;

	/* make an array of the intervals for the first 1000 (or n1-1) timestamps */
	if((inter=(double *)realloc(inter,(n2*sizeof(double))))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1.0);}
	for(ii=1;ii<n2;ii++) inter[(ii-1)]=time1[ii]-time1[(ii-1)];

	/* decrement n2 because there is one less interval than samples contributing to the intervals */
	n2--;

	/* sort the interval array */
	qsort(inter,n2,sizeof(double),xf_compare1_d);

	a=n2*0.5;  /* find the position in the sorted array corresponding to the mid-point */
	jj=(long)a;	/* convert this to an integer element-number */
	if(a!=(double)jj) median=inter[jj]; /* if the percentile limit does not fall exactly on an item, cutoff	is unchanged */
	else median=(inter[jj]+inter[jj-1])/2.0; /* otherwise if the cutoff is exactly an item number, take the avergage of this point and the previous item */

	if(median!=0.0) sampfreq=1.0/median;
	else {
		sprintf(message,"%s [ERROR]: no variance in first %ld time-stamps",thisfunc,(n2+1));
		sampfreq=0.0;
	}

	/* free memory and return the result */
	free(inter);
	return(sampfreq);
}
