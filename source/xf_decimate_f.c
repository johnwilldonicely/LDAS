/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	Keep every "nth" point in an input (float)
	- can accept fractional decimation factors

USES:
	achieve exact downsampling of an input

DEPENDENCIES: none

ARGUMENTS:
	float *data    : input, array
	long ndata     : input, length of input array
	double winsize : the size of the decimation window (eg. 10.5)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	number of points in decimated reults or -1 on error

SAMPLE CALL:
	# to downsample a 10s input sampled at 500 Hz to 400 Hz
	#	- n= 10*500 = 5000
	#	- downsample factor is 500/400= 1.25

	n= xf_decimate_f(data,n,1.25,message);
	if(n<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	# note that n should be 4000

*/

#include <math.h>
#include <stdio.h>

long xf_decimate_f(float *data, long ndata, double winsize,  char *message) {

	char *thisfunc="xf_decimate_f\0";
	long ii,jj;
	double limit=0.0;

	if(winsize>ndata) { sprintf(message,"%s [ERROR]: winsize (%f) exceeds data-length (%ld)",thisfunc,winsize,ndata); return(-1); }

	for(ii=jj=0;ii<ndata;ii++) {
		if((double)ii>=limit){
			data[jj]=data[ii];
			limit+=winsize;
			jj++;
		}
	}
	return(jj);
}
