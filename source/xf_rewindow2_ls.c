/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Make new event-time and event-class arrays based on events inside windows centred on reference-class events
	Builds new time and class arrays using only data from the reference class and events in a window around them
	This function ensures no duplication of events, and the sequence of events is preserved
	Method:
	- identifies reference events
	- builds new array from events in a window around the reference event
	- cuts the window short if another reference-class event is encountered in the window
	- skips to next event falling outside the window and repeat
	- does not include events from previous windows when looking back

	12.November.2016 [JRH]: based on xf_rewindow1, 20 November 2011

USES:
	Speed up calculation of cross-correlations cells-pairs

ARGUMENTS:
	long  *t1    : input, array of event timestamps
	short *c1    : input, array of the class-ids for each event
	long   n1    : input, number of elements in t1 and c1
	short  id    : input, the class-id of events on which to centre the time windows
	long winsize : input, the size (samples) of the time-windows
	long  *t2    : output, preallocated array to store event times
	short *c2    : output, preallocated array to store event class-ids

RETURN VALUE:
	n2, the number of elements in the new time and class-id arrays

NOTES:
	t1 and c1 should be filled before calling this function
	t2 and c2 must have adequate memory allocated before calling this function.
	It should be possible for t2 and c2 to hold up the same number of elements as the input arrays

*/

long xf_rewindow2_ls(long *t1, short *c1, long n1, short id, long winsize, long *t2, short *c2) {

	long ii,jj,kk,n2;
	long halfwin,startindex=-1,endindex=-1,endprev,timeA,timeB;

	n2= 0;       // size of the new array
	endprev= -1; // index to the last event included from a previous window - should not be included again
	halfwin= winsize/2;

	/* INITIALIZE THE RANGE OF DATA INDICES TO SCAN - SAVES HAVING TO TEST FOR START OR END OF RECORD WITHIN THE LOOP */
	timeA= t1[0]+halfwin;
	timeB= t1[(n1-1)]-halfwin;
	for(ii=0;ii<n1&&t1[ii]<timeA;ii++) startindex=ii;
	for(ii=(n1-1);ii>=0&&t1[ii]>timeB;ii--) endindex=ii;

	/* SCAN THE DATA LOOKING FOR REFERENCE-EVENTS (ID) AND COLLECT EVENTS AROUND THEM INTO t2 & c2 */
	for(ii=startindex;ii<endindex;ii++) {

		if(c1[ii]!=id) continue;  // do not scan around non-reference events

		/* set the timestamp boundaries for this window */
		timeA= t1[ii]-halfwin;
		timeB= t1[ii]+halfwin;

		/* scan backwards to find index from which to begin (kk) - this may be the reference index (ii) */
		for(jj=kk=ii;jj>endprev;jj--) { if(t1[jj]>timeA) kk=jj;	else break; }

		/* the default value for "previously detected event" is set to kk - which if nothing else will be the reference index ii */
		endprev= kk;

		/* now go forward building new arrays from events from here to the end of the time-window */
		for(jj=kk;t1[jj]<=timeB;jj++) {
			if(c1[jj]==id&&jj>ii) break; // include current, but not future reference events in the same window
			t2[n2]=t1[jj];  // add new time
			c2[n2]=c1[jj];  // add new class id
			n2++;           // increment size of new array
			endprev=jj;     // identify the last sample that was included - not less than ii
		}

		/* advance ii to last event in the window (incrimented at end of loop), to avoid repeat-counting */
		ii= endprev;
	}
	return(n2);
}
