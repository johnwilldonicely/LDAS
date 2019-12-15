/*
<TAGS>signal_processing stats time </TAGS>

DESCRIPTION:
	Calculate the density (counts) defined by start-stop pairs of blocks of samples, using non-overlapping windows
	Assigns memory for the density array
	Very similar to xf_smoothbox1_d, derived from xf_density1_1

USES:
	- convert lost.ssp packet-loss definitions into a packet-loss time-series

DEPENDENCIES:
	None

ARGUMENTS:
	long *start1  : input array of start-times (sample numbers) - assumed to be in ascending order
	long *stop1   : input array of stop-times (sample numbers) - assumed to be in ascending order
	long nn       : total number of events
	long min      : minimum event time: -1= events[0]
	long max      : maximum event time: -1= events[nevents-1]
	double winsize: size of the sliding window (samples, fractional if necessary) in which to calculate density
	long nwin     : result: number of density-windows generated
	char *message : arrray to hold message in the event of an error

RETURN VALUE:
	on success:
		- pointer to the density array (counts, time series)
		- nwin will be updated to reflect size of density array
	on error:
		NULL

SAMPLE CALL:
	density= xf_density2_l(start1,stop1,nn,0,-1,winsize,&nwin,message);
	if(density==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long *xf_density2_l(long *start1,long *stop1,long nn,long min,long max,double winsize,long *nwin,char *message) {

	char *thisfunc="xf_density2_l\0";
	long ii,jj,kk,*density1=NULL,tempnwin,tempstart,tempstop;
	double dur;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn<0) { sprintf(message,"%s: number of start1 (%ld) must be >= 0",thisfunc,nn); return(NULL); }

	/* DEFINE MIN, MAX, DURATION WINSIZE AND NUMBER OF WINDOWS */
	/* assumes start1 values are in ascending order */
	if(min<0) min= start1[0];
	if(max<0) max= stop1[(nn-1)];
	if(max<min) { sprintf(message,"%s: min (%ld) must be < max (%ld)",thisfunc,min,max); return(NULL); }
	dur= (double)((max-min)+1);
	if(winsize>dur) { sprintf(message,"%s: window size %g exceeds events time-span %g",thisfunc,winsize,dur); return(NULL); }
	/* calculate the number of windows */
	if(fmod(dur,winsize)==0) tempnwin= (long)(dur/winsize);
	else tempnwin= (long)(dur/winsize)+1;
	// TEST: fprintf(stderr,"function: min=%ld	max=%ld	dur=%.3f	tempnwin=%ld\n",min,max,dur,tempnwin);

	/* ALLOCATE MEMORY FOR TEMP AND FINAL DENSITY ARRAYS AND PRE-FILL THE TEMP ARRAY */
	density1= calloc(tempnwin,sizeof(*density1));
	if(density1==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(NULL); }

	/* FILL THE DENSITY ARRAY */
	for(ii=0;ii<nn;ii++) {
		tempstart= start1[ii];
		tempstop=  stop1[ii];
		for(jj=tempstart;jj<tempstop;jj++) {
			if(jj<min||jj>max) continue;
			else if(jj==min) density1[0]++;
			else {
				kk= (long)((double)(jj-min)/winsize);
				// CHECK if(kk>=tempnwin) fprintf(stderr,"ERROR!! kk=%ld, but tempnwin=%ld\n",kk,tempnwin);
				density1[kk]++;
	}}}

	*nwin= tempnwin;
	return(density1);
}
