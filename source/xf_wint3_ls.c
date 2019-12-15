/*
<TAGS>signal_processing stats time</TAGS>

DESCRIPTION:
	Calculate interval between either of two classes of events in a sliding event-centred window
	The function will handle assigning sufficient memory
	NOTE: function assumes time values are in ascending order

USES:
	Determine the effect on the autocorrelogram if two classes of events were combined

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *time : array already holding timestamps for each event
	long *group : array already holding event ids (eg. cell-numbers)
	long nn : number of elements in the arrays
	long g1 : the id for the reference event1
	long g2 : the id for the reference event2
	long winsize     : the size of the window centred on each event, within which to find intervals
	long *nintervals : results, number of intervals detected (pass to function as address)

RETURN VALUE:
	Success:
		A pointer to an array of interval values with nintervals elements
		Value for nintervals will be overwritten
		NOTE: pointer will be NULL if nintervals=0 (non-match of group to g1 and g2)
	Failure:
		NULL, with nintervals set to -1 (if memory allocation fails)

SAMPLE CALL:
	long time[1001], group[1001], g1=1, g2=2, winsize=150, nintervals=0, *intervals=NULL;;
	// time and group data must be stored in memory first
	intervals= xf_wint2_ls(time,group,n,g1,g2,winsize,&nintervals);
	if(nintervals==-1) { fprintf(stderr,"\n--- Error [%s/xf_wint1_l]: memory allocation error\n\n",thisprog);exit(1);}
	else if(nintervals==0) { fprintf(stderr,"--- Warning [%s]: no intervals for groups %d vs. %d\n",thisprog,g1,g2); }

*/

#include <stdio.h>
#include <stdlib.h>

long *xf_wint3_ls(long *time, short *group, long nn, short g1, short g2, long winsize, long *nintervals) {

	int sizeoflong=sizeof(long);
	long ii,jj,kk,halftimepos,halftimeneg,tempint,tot,*intervals=NULL;

	halftimepos= winsize/2;
	halftimeneg= -winsize/2;
	tot=0;

	/* CALCULATE INTERVALS IN A SLIDING WINDOW */
	for(ii=0;ii<nn;ii++) {
		if(group[ii]!=g1 && group[ii]!=g2) continue;
		/* look back from starting point */
		for(jj=ii-1;jj>=0;jj--) {
			if(group[jj]!=g1 && group[jj]!=g2 ) continue;
			tempint=time[jj]-time[ii]; // time[jj] will be the smaller number
			if(tempint < halftimeneg) break;
			intervals=(long *)realloc(intervals,(tot+1)*sizeoflong);
			if(intervals==NULL) { *nintervals=-1; return(NULL); }
			intervals[tot++]=tempint;
		}
		/* look forward from starting point */
		for(jj=ii+1;jj<nn;jj++) {
			if(group[jj]!=g1 && group[jj]!=g2 ) continue;
			tempint=time[jj]-time[ii]; // time[ii] will be the smaller number
			if(tempint > halftimepos) break;
			intervals=(long *)realloc(intervals,(tot+1)*sizeoflong);
			if(intervals==NULL) { *nintervals=-1; return(NULL); }
			intervals[tot++]=tempint;
		}
	}

	*nintervals=tot;
	return (intervals);
}
