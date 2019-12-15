/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	Align events to the inflection nearest the midpoint
	Calling function specifies whether inflection should be positive or negative
	Calling function specifies the maximum allowable shift

USES:
	Prevent possible cancellation of peaks and troughs when oscillatory events are averaged

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data   : the raw data containing events- should be filtered (smooth) in the event frequency range
	long ndata    : the number of samples in data
	long *etime   : sample-numbers in data representing event-times - typically the middle of the event
	long nevents  : the number of events in etime
	int sign      : detect nearest positive (1) or negative (-1) inflection
	long shiftmax : largest allowable shift (samples)
	char *message : arrray to hold message in the event of an error - calling function must allocate memory

RETURN VALUE:
	0 on success, -1 on error

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_eventadjust1_f(float *data, long ndata, long *etime, long nevents, int sign, long shiftmax, char *message) {

	char *thisfunc="xf_eventadjust1_f\0";
	long ii,jj,kk,nm1,event,shiftpos,shiftneg;

	if(sign!=-1 && sign!=0 && sign!=1) { sprintf(message,"%s: invalid sign (%d) - must be -1, 0 or 1",thisfunc,sign); return(-1); }
	nm1=ndata-1;

	/* ALIGN TO NEAREST POSITIVE INFLECTION */
	if(sign==1) {
	for(event=0;event<nevents;event++) {
		/* initialize shift values */
		shiftpos=shiftneg=0;
		/* search forward */
		for(ii=0;ii<shiftmax;ii++) {   // restrict search to valid shift values
			jj=etime[event]+ii;  // index to original data
			if(jj>=nm1) break; // out of bounds check
			if(data[jj]>data[jj-1] && data[jj]>data[jj+1]) {shiftpos=ii;break;}
		}
		/* now search backwards */
		for(ii=1;ii<shiftmax;ii++) {   // restrict search to valid shift values
			jj=etime[event]-ii;  // index to original data
			if(jj<=0) break;      // out of bounds check
			if(data[jj]>data[jj-1] && data[jj]>data[jj+1]) {shiftneg=ii;break;}
		}
		/* apply the shift corresponding to the nearest inflection */
		if(shiftpos<= shiftneg) etime[event]+= shiftpos;
		if(shiftneg<  shiftpos) etime[event]-= shiftneg;
	}}

	/* ALTERNATIVELY, ALIGN TO NEAREST NEGATIVE INFLECTION */
	if(sign==-1) {
	for(event=0;event<nevents;event++) {
		/* initialize shift values */
		shiftpos=shiftneg=0;
		/* search forward */
		for(ii=0;ii<shiftmax;ii++) {   // restrict search to valid shift values
			jj=etime[event]+ii;  // index to original data
			if(jj>=nm1) break; // out of bounds check
			if(data[jj]<data[jj-1] && data[jj]<data[jj+1]) {shiftpos=ii;break;}
		}
		/* now search backwards */
		for(ii=1;ii<shiftmax;ii++) {   // restrict search to valid shift values
			jj=etime[event]-ii;  // index to original data
			if(jj<0) break;      // out of bounds check
			if(data[jj]<data[jj-1] && data[jj]<data[jj+1]) {shiftneg=ii;break;}
		}
		/* apply the shift corresponding to the nearest inflection */
		if(shiftpos<= shiftneg) etime[event]+= shiftpos;
		if(shiftneg<  shiftpos) etime[event]-= shiftneg;
	}}

	return(0);
}
