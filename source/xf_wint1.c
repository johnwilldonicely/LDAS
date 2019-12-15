/*
<TAGS>signal_processing stats time</TAGS>

DESCRIPTION:
	Calculate interval between two classes of events in a sliding event-centred window

USES:
	Typically used as the first step to generate auto- and cross-corellograms for
	neuronal spike-trains. Typically used in conjuction with function xf_hist1

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *tsec : array already holding timestamps (seconds) for each event
	int *group : array already holding event ids (eg. cell-numbers)
	long n : number of elements in the arrays
	int g1 : the id for the reference event
	int g2 : the id for the event the timing of which is to be compared with g1
	float winsize : the size of the window centred on each event, within which to find intervals
	long *result_l : results array - result_l[0] will holds the number of intervals found

RETURN VALUE:
	A pointer to an array of type double with result_i[0] elements
	This variable must be initialized by the calling function and assigned value NULL
	Te function will handle assigning sufficient memory

SAMPLE CALL:
	int g1=1, g2=2;
	long result_i[32];
	float winsize=0.01;
	double time[1001],group[1001],*intervals=NULL;
	// time and group data must be stored in memory first
	// now find the intervals between events of class g1 and g2
	intervals=xf_wint1(time,group,n,g1,g2,winsize,result_l);
*/

#include <stdio.h>
#include <stdlib.h>
double *xf_wint1(double *time, int *group, long n, int g1, int g2, float winsize, long *result_l) {

	int sizeofdouble=sizeof(double);
	long i,j,k,tot=0;
	double aa,bb,halftime;
	double *intervals=NULL;

	bb=(double)-1.0;

	/* CALCULATE INTERVALS IN A SLIDING WINDOW */
	halftime=(double)winsize/2.0;
	for(i=1;i<n;i++) {
		if(group[i]!=g1) continue;
		/* look back from starting point */
		for(j=i-1;j>=0;j--) {
			aa=time[j]-time[i];
			if((aa*bb) > halftime) break; // NOTE: bb=-1.0
			if(group[j]!=g2) continue;
			intervals=(double *)realloc(intervals,(tot+1)*sizeofdouble);
			intervals[tot++]=aa;
		}
		/* look forward from starting point */
		for(j=i+1;j<n;j++) {
			aa=time[j]-time[i];
			if(aa > halftime) break;
			if(group[j]!=g2) continue;
			intervals=(double *)realloc(intervals,(tot+1)*sizeofdouble);
			intervals[tot++]=aa;
	}}
	result_l[0]=tot;
	return (intervals);
}
