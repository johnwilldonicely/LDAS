/*
<TAGS>signal_processing transform</TAGS>
DESCRIPTION:
	- a very smimple binning function that returns a pointer to a new binned dataset
	- works from the beginning of the input to the end, using fixed-size bins
	- partial bins at the end of the input will be omitted
	- NANs and INFS will not contribute to the contents
USES:
	- Downsampling data without disrupting the input array

DEPENDENCIES: NONE

ARGUMENTS:
	double *data   : input: pointer to input array
	long n1        : input: number of elements in data array
	long binsize   : input: desired bin-width (samples - can be a fraction)
	int type       : input: desired result-type, 1=SUM, 2=MEAN, 3=MEDIAN
	long *nbins    : output: total bins generated - will be (long)(n1/binsize)
	char *message  : output: array to hold error message

RETURN VALUE:
	- SUCCES: pointer to result
	- FAILURE: NULL, with error described in message array

*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int xf_compare1_d(const void *a, const void *b);
double xf_percentile2_d(double *data, long nn, double setper, char *message);


double *xf_bin_simple_d(double *data1, long n1, long binsize, int type, long *nbins, char *message) {

	char *thisfunc="xf_bin_simple_d\0",message2[256];
	long ii=0,jj,kk,mm,nsums=0,binmax=0,bin=0;
	double aa,bb,cc,sum=0.0,*result=NULL,*pdata=NULL;

	if(n1<1) { sprintf(message,"%s [ERROR]: number of samples (%ld) must be >0",thisfunc,n1); return(NULL); }
	if(binsize<=0) {sprintf(message,"%s [ERROR]: bin size (%ld) must be >0",thisfunc,binsize); return(NULL);}
	if(binsize>n1) {sprintf(message,"%s [ERROR]: bin size (%ld) must <= input length (%ld)",thisfunc,binsize,n1); return(NULL);}
	if(type<1||type>3) {sprintf(message,"%s [ERROR]: invalid binning type (%d) must be 1-3",thisfunc,type); return(NULL);}

	binmax= n1/binsize;
	result= realloc(result,binmax*sizeof(*result));
	if(result==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL);}

	/* set the limit for scanning the input array */
	/* - this ensures the loop will not exceed bounds OR produce results for partial bins */
	mm= binmax*binsize;

	for(ii=0;ii<mm;ii+=binsize) {
		pdata= data1+ii;
		if(type==1) { // bins contain sum
			nsums=0; sum=0.0;
			for(jj=0;jj<binsize;jj++) {
				if(isfinite(pdata[jj])) { sum+= pdata[jj]; nsums++; } // only include valid data
			}
			if(nsums==0) result[bin]= NAN;
			else result[bin]= sum;
		}
		else if(type==2) { // bins contain mean - code almost identiccal to above
			nsums=0; sum=0.0;
			for(jj=0;jj<binsize;jj++) {
				if(isfinite(pdata[jj])) { sum+= pdata[jj]; nsums++; } // only include valid data
			}
			if(nsums==0) result[bin]= NAN;
			else result[bin]= sum/(double)nsums;
		}
		else { // bins contain median - for this, use the percentile function
			aa= xf_percentile2_d(pdata,binsize,50.0,message2);
		}
		bin++;
	}

	*nbins= bin;
	return(result);
}
