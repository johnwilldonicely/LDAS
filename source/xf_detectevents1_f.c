/*
<TAGS>signal_processing detect</TAGS>

DESCRIPTION:
	Detect events in a series when a threshold is exceeded
	For each supra-threshold point, the function looks outward until the data drops below the half-threshold
	Edges of events (stop-start= width) are defined by the edge-threshold
	Data must drop below half-threshold before another event can be detected
	No event will be detected until data is found which does NOT exceed threshold


USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data   : input holding data
	long n1       : size of data array
	float thresh  : data sample must exceed this value for event detection to begin
	float edge    ; percentage of threshold to determine edge of the event (e.g. 0.5)
	int sign      : sign of detection (-1,0,+1 :  -1=lessthan +1=greaterthan, 0=either)
	long nmin     : minimum number of samples for which the half-threshold must be exceeded (0=no limit)
	long nmax     : maximum number of samples for which the half-threshold must be exceeded (0=no limit)
	long **estart : address for results array (event start-samples) - calling function must free()
	long **estop  : address for results array (event stop-samples) - calling function must free()
	char *message : arrray to hold message in the event of an error

RETURN VALUE:
	number of detected events (>=0) on success
	-1 on error
	result array will have been assigned memory and will hold event start and stop samples

SAMPLE CALL:
	# detect events where signal is less than three for between 200 and 2000 samples

		long *estart=NULL,*estop=NULL;

		nevents= xf_detectevents1_f(data,n,3,-1,200,2000,&estart,&estop,message);
		if(nevents<0) {fprintf(stderr,"\n--- Error: %s\n\n",message); exit(1);}

		if(estart!=NULL) free(estart);
		if(estop!=NULL) free(estop)

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long xf_detectevents1_f(float *data,long n1,float thresh, float edge, int sign,long nmin,long nmax,long **estart,long **estop,char *message) {

	char *thisfunc="xf_detectevents1_f\0";
	int detect,sizeoflong=sizeof(long);
	long i,j,k,start,stop,length,nminus1;
	long *events=NULL,etot;
	long *tempstart=NULL,*tempstop=NULL;

	if(sign!=-1 && sign!=0 && sign!=1) { sprintf(message,"%s: sign argument (%d) must be -1, 0 or 1",thisfunc,sign); return(-1); }
	if(nmin>nmax) { sprintf(message,"%s: nmin argument (%ld) must be <= nmax (%ld)",thisfunc,nmin,nmax); return(-1); }
	if(edge<=0 || edge>=1) { sprintf(message,"%s: invalid edge (%g) - must be between 0 and 1",thisfunc,edge); return(-1); }

	/* start in "detected" mde with a start value guaranteed to fail the min and max criteria */
	/* ensures that data must go below threshold before detection can begin */
	detect=1; start=n1;

	etot=0;
	edge=thresh*edge;
	nminus1=n1-1;

	if(sign==1) {
		for(i=0;i<n1;i++) {
			if(data[i]>=thresh) {
				if(detect==0) {start=i; while(data[start]>=edge && start>0) start--; if(start<i) start++;}
				detect=1;
			}
			else {
				if(detect==1) {
					stop=i;
					while(data[stop]>=edge && stop<nminus1) stop++;
					if(stop>start) stop--; // bring stop back to last sample exceeding half-threshold
					length= (stop-start);
					if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) ) {
						tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
						if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
						if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstart[etot]= start;
						tempstop[etot]= stop;
						etot++;
					}
					i=stop;
				}
				detect=0;
	}}}

	if(sign==-1) {
		for(i=0;i<n1;i++) {
			if(data[i]<=thresh) {
				if(detect==0) {start=i; while(data[start]<=edge && start>0) start--;if(start<i) start++;}
				detect=1;
			}
			else {
				if(detect==1) {
					stop=i;
					while(data[stop]<=edge && stop<nminus1) stop++;
					if(stop>start) stop--; // bring stop back to last sample exceeding half-threshold
					length= (stop-start);
					if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) ) {
						tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
						if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
						if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstart[etot]= start;
						tempstop[etot]= stop;
						etot++;
					}
					i=stop;
				}
				detect=0;
	}}}

	if(sign==0) {
		for(i=0;i<n1;i++) {
			if(fabsf(data[i])>=thresh) {
				if(detect==0) {start=i; while(fabs(data[start])>=edge && start>0) start--;if(start<i) start++;}
				detect=1;
			}
			else {
				if(detect==1) {
					stop=i;
					/* find half-threshold end-time - sign of detection depends on whether the current event is signed negative or positive */
					if(data[i]>thresh) while(data[stop]<=edge && stop<nminus1) stop++;
					else if(data[i]<thresh) while(data[stop]>=edge && stop<nminus1) stop++;
					if(stop>start) stop--; // bring stop back to last sample exceeding half-threshold
					length= (stop-start);
					if( (length>=nmin || nmin==0)  && (length<=nmax || nmax==0) ) {
						tempstart= (long *)realloc(tempstart,((etot+1)*sizeoflong));
						if(tempstart==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstop= (long *)realloc(tempstop,((etot+1)*sizeoflong));
						if(tempstop==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
						tempstart[etot]= start;
						tempstop[etot]= i;
						etot++;
					}
					i=stop;
				}
				detect=0;
	}}}

	(*estart)=tempstart;
	(*estop)=tempstop;

	return(etot);
}
