/*
<TAGS>signal_processing detect</TAGS>

DESCRIPTION:
	Define start-stop sample-pairs (SSP) defining periods (events) when data falls within a range
	This is slightly different from detectevents1&2, which are geared towards peak and edge thresholds and define a time for the peak as well
	- must meet a duration criterion defined by tstamp
	- data must fall out-of-range before another event can be detected
	- this function allocates memory for the results
	- NOTE: for each start-stop pair...
		start = first sample meeting the criteria
		stop  = first sample NOT meeting the criteria thereafter (ie the sample after the last sample in the block)
		hence if start=stop, duration of event is zero

	UPDATE: 20.July.2016: fix adjustment to stop if last sample is good - DO NOT adjust the actual timestamp of the last sample!

USES:
	- define epochs when velociity meets a criteria

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *tstamp  : input timestamps used to asess duration of events, and to define start-stop pairs
	float *data   : input data
	long n1       : size of data array
	float min     : minimum data value for inclusion (nan = no limit = FLT_MIN)
	float max     : minimum data value for inclusion (nan = no limit = FLT_MAX)
	long mindur   : minimum duration of event (0 = no limit - should be in tstamp units)
	long **estart : address for results array (event start-samples) - calling function must free()
	long **estop  : address for results array (event stop-samples) - calling function must free()
	char *message : arrray to hold message in the event of an error - calling function must allocate memory

RETURN VALUE:
	number of detected events, >=0 on success
	-1 on error
	result arrays estart and estop will have been assigned memory and will hold event start and stop samples

SAMPLE CALL:
	# detect events where data is less than 5 for at least 10seconds, and timestamps reflected data sampled at 20k

	long kk,*estart=NULL,*estop=NULL;
	double min=nan, max=5, duration=10, samprate=20000;

	kk=(long)(duration*samprate);

	nevents= xf_detectevents2_f(tstamp,data,n1,min,max,kk,&estart,&estop,message);
	if(nevents<0) {fprintf(stderr,"\n--- Error: %s\n\n",message); exit(1);}

	if(estart!=NULL) free(estart);
	if(estop!=NULL) free(estop)

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

long xf_detectevents3_lf(long *tstamp,float *data,long n1,float min,float max,long mindur,long **estart,long **estop,char *message) {

	char *thisfunc="xf_detectevents3_lf\0";
	int sizeoflong=sizeof(long);
	long ii,jj,kk,start,stop,A,Z,tempdur,ssptot;
	long *tempstart=NULL,*temppeak=NULL,*tempstop=NULL;

	/* argument-check */
	if(n1<=0) { sprintf(message,"%s: no input data",thisfunc); return(-1); }

	/* if min or max are NAN, set to FLT_MIN or FLT_MAX respectively (i.e. no limit) */
	if(! isfinite(min)) min=FLT_MIN;
	if(! isfinite(max)) max=FLT_MAX;

	/* initialize start-stop-pair counter */
	ssptot=0;

	/* find the events */
	for(ii=0;ii<n1;ii++) {
		if(data[ii]>=min && data[ii]<=max) {
			/* seek forwards to find the end of the event and update stop */
			start=stop=ii;
			while(data[stop]>=min && data[stop]<=max && stop<n1) stop++;

			/* temporarily store the start-stop pair as A and Z */
			if(stop<n1) {
				A=tstamp[start];
				Z=tstamp[stop];
			}
			/* if last sample was good, hypothetical end of event is one sample past the last timestamp */
			else {
				A=tstamp[start];
				Z=tstamp[n1-1]+1;
			}

			/* calculate event tempdur based on timestamps for start & stop */
			tempdur= (Z-A);

			/* if event meets tempdur criterion, save it */
			if( tempdur >= mindur ) {
				tempstart= (long *)realloc(tempstart,((ssptot+1)*sizeoflong));
				tempstop= (long *)realloc(tempstop,((ssptot+1)*sizeoflong));
				if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
				if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
				tempstart[ssptot]= A;
				tempstop[ssptot]= Z;
				ssptot++;
			}
			/* reset ii so another event cannot be detected within the current one */
			ii=stop;
	}}

	(*estart)=tempstart;
	(*estop)=tempstop;

	return(ssptot);
}
