/*
<TAGS>signal_processing stats time</TAGS>

DESCRIPTION:

	Calculate interval between two classes of events in a list of predefined time windows
	Time windows must be non-overlapping
	Returns pointer to array holding a list of intervals
	NOTE!! Both events and windows must be time-ordered

ARGUMENTS (9 TOTAL):
	time: array holding the time of the events
	group: array holding the class of the events
	n: the total number of events
	g1: the class of event providing the reference times
	g2: the class of event whose times, relative to g1, are used to build the interval list
	wint1, wint2: arrays holding matching start & end times for each window
	nw: the number of windows
	result_l: result array to hold the nintsal number of elements in *intervals

SAMPLE CALL:
	int g1=1, g2=2, nevents=1000,nwin=64, result_i[32];
	double time[1001],group[1001],winstarts[65],winends[65],*intervals=NULL;

	intervals=hux_wint(time,group,nevents,g1,g2,wstarts,winends,nwin,result_l)

EXAMPLE USE:
	to obtain the values for a spike cross-correlation histogram, for a pair of cells,
	looking only in discrete time windows corresponding to theta cycles, sleep periods, running, etc
*/

#include <stdio.h>
#include <stdlib.h>
double *xf_wint2_di(double *time, int *group, long n, int g1, int g2, double *wt1, double *wt2, long nw, long *result_l) {

	int sizeofdouble=sizeof(double);
	long i,j,k,nints=0,win;
	double aa,bb,start,end,*intervals=NULL;

	/* OUTPUT INTERVALS IN A SLIDING WINDOW */
	i=1; // set event index to second event
	for(win=0;win<nw;win++) {
		start=wt1[win]; end=wt2[win]; //define the start and end times of the current window

		for(i=i;i<n;i++) {	// for all events, picking up from the last break in the loop
			if(time[i]<start) continue;	// loop continues here if events haven't reached the current window yet
			if(time[i]>=end) break;		// loop breaks here if event happened after the window - go to next window
			if(group[i]!=g1) continue;
			/* look back from starting point */
			for(j=i-1;j>=0;j--) {
				if(time[j]<start) break; // obviously the first event inside the current window will fail immediately
				if(group[j]!=g2) continue;
				aa=time[j]-time[i];
				intervals=(double *)realloc(intervals,(nints+1)*sizeofdouble);
				intervals[nints++]=aa;
			}
			/* look forward from starting point */
			for(j=i+1;j<n;j++) {
				if(time[j]>end) break;
				if(group[j]!=g2) continue;
				aa=time[j]-time[i];
				intervals=(double *)realloc(intervals,(nints+1)*sizeofdouble);
				intervals[nints++]=aa;
			}
		} // end of event loop
	} // end of window loop

	result_l[0]=nints;	// holds total number of intervals
	return (intervals);	// return the array holding n=nints intervals
}
