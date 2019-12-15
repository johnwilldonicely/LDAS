/*
<TAGS>signal_processing detect</TAGS>

DESCRIPTION:
	Detect events in a series when a threshold is exceeded - must meet a duration criterion
	Edges of events (stop-start= length) are defined by the edge-threshold
	Data must drop below edge-threshold before another event can be detected
	No event will be detected until data is found which does NOT exceed threshold
	This function allocates memory for the results

	NOTE: Data should be high-pass filtered and/or z-scored before running this!

	Changes from xf_detectevents1_f
	- upper threshold accepted to allow event rejection
	- output now includes a measure of the peak-time within each event, instead of just the start & stop times

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data   : input holding data
	long n1       : size of data array
	float thresh  : data sample must exceed this value for event detection to begin
	float upper   : data within an event must remain below this multiple of the threshold - exceeding this value invalidates the event
	float edge    ; data must drop below this multiple of the threshold to determine the edge of the event (e.g. 0.5)
	int sign      : sign of detection (-1,0,+1 :  -1=lessthan +1=greaterthan, 0=either)
	long nmin     : minimum number of samples for which the half-threshold must be exceeded (0=no limit)
	long nmax     : maximum number of samples for which the half-threshold must be exceeded (0=no limit)
	long **estart : address for results array (event start-samples) - calling function must free()
	long **epeak  : address for results array (event peak-samples) - calling function must free()
	long **estop  : address for results array (event stop-samples) - calling function must free()
	char *message : arrray to hold message in the event of an error - calling function must allocate memory

RETURN VALUE:
	number of detected events (>=0) on success
	-1 on error
	result array will have been assigned memory and will hold event start and stop samples

SAMPLE CALL:
	# detect events where signal is less than three for between 200 and 2000 samples

		long *estart=NULL,*estop=NULL;

		nevents= xf_detectevents2_f(data,n,3,10,.5,-1,200,2000,&estart,&epeak,&estop,message);
		if(nevents<0) {fprintf(stderr,"\n--- Error: %s\n\n",message); exit(1);}

		if(estart!=NULL) free(estart);
		if(epeak!=NULL) free(epeak);
		if(estop!=NULL) free(estop)

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long xf_detectevents2_f(float *data,long n1,float thresh, float upper, float edge, int sign,long nmin,long nmax,long **estart, long **epeak, long **estop,char *message) {

	char *thisfunc="xf_detectevents1_f\0";
	int detect,sizeoflong=sizeof(long);
	float peakval;
	long i,j,k,start,stop,peak,length,nminus1;
	long *events=NULL,etot;
	long *tempstart=NULL,*temppeak=NULL,*tempstop=NULL;

	if(sign!=-1 && sign!=0 && sign!=1) { sprintf(message,"%s: invalid sign (%d) - must be -1, 0 or 1",thisfunc,sign); return(-1); }
	if(nmin>nmax && nmax!=0) { sprintf(message,"%s: invalid nmin (%ld) - must be <= nmax (%ld)",thisfunc,nmin,nmax); return(-1); }
	if(edge<=0 || edge>1) { sprintf(message,"%s: invalid edge (%g) - must be >0 and <=1",thisfunc,edge); return(-1); }
	if(upper<=1) { sprintf(message,"%s: invalid upper (%g) - must be >=1",thisfunc,upper); return(-1); }

	/* start in "detected" mde with a start value guaranteed to fail the min and max criteria */
	/* ensures that data must go below threshold before detection can begin */
	detect=1;
	start=n1;
	etot=0;
	nminus1=n1-1;
	edge=thresh*edge;
	upper=thresh*upper;

	if(sign==1) {
		for(i=0;i<n1;i++) {
			if(data[i]>=thresh) {

				/* seek backwards to find the beginning of the event */
				start=i; while(data[start]>=edge && start>0) start--; if(start<i) start++;
				/* seek forwards to find the end of the event and update stop */
				stop=i; while(data[stop]>=edge && stop<n1) stop++; if(stop>start) stop--;
				/* define the peak */
				peak=i;
				peakval=data[i];
				for(j=start;j<=stop;j++) { if(data[j]>peakval) {peak=j;peakval=data[j];} }

				/* if event meets length and upper-range criteria, save it */
				length= (stop-start);
				if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) && peakval<=upper) {
					tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
					if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					temppeak= (long *)realloc(temppeak,((etot+1)*sizeoflong));
					if(temppeak==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
					if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstart[etot]= start;
					temppeak[etot]= peak;
					tempstop[etot]= stop;
					etot++;
				}
				/* reset i so another event cannot be detected within the current one */
				i=stop;
		}}}

	if(sign==-1) {
		for(i=0;i<n1;i++) {
			if(data[i]<=thresh) {

				/* seek backwards to find the beginning of the event */
				start=i; while(data[start]<=edge && start>0) start--; if(start<i) start++;
				/* seek forwards to find the end of the event and update stop */
				stop=i; while(data[stop]<=edge && stop<n1) stop++; if(stop>start) stop--;
				/* define the peak */
				peak=i;
				peakval=data[i];
				for(j=start;j<=stop;j++) { if(data[j]<peakval) {peak=j;peakval=data[j];} }

				/* if event meets length and upper-range criteria, save it */
				length= (stop-start);
				if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) && peakval>=upper) {
					tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
					if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					temppeak= (long *)realloc(temppeak,((etot+1)*sizeoflong));
					if(temppeak==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
					if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstart[etot]= start;
					temppeak[etot]= peak;
					tempstop[etot]= stop;
					etot++;
				}
				/* reset i so another event cannot be detected within the current one */
				i=stop;
		}}}

	if(sign==0) {
		thresh=fabsf(thresh);
		edge=fabsf(edge);
		upper=fabsf(upper);
		for(i=0;i<n1;i++) {
			if(fabsf(data[i])>=thresh) {

				/* seek backwards to find the beginning of the event */
				start=i; while(fabsf(data[start])>=edge && start>0) start--; if(start<i) start++;
				/* seek forwards to find the end of the event and update stop */
				stop=i; while(fabsf(data[stop])>=edge && stop<n1) stop++; if(stop>start) stop--;
				/* define the peak */
				peak=i;
				peakval=fabsf(data[i]);
				for(j=start;j<=stop;j++) { if(fabsf(data[j])>peakval) {peak=j;peakval=fabsf(data[j]);} }

				/* if event meets length and upper-range criteria, save it */
				length= (stop-start);
				if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) && peakval<=upper) {
					tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
					if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					temppeak= (long *)realloc(temppeak,((etot+1)*sizeoflong));
					if(temppeak==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
					if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
					tempstart[etot]= start;
					temppeak[etot]= peak;
					tempstop[etot]= stop;
					etot++;
				}
				/* reset i so another event cannot be detected within the current one */
				i=stop;
		}}}

	(*estart)=tempstart;
	(*epeak)=temppeak;
	(*estop)=tempstop;

	return(etot);
}
