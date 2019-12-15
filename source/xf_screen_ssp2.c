/*
<TAGS>signal_processing screen time</TAGS>
DESCRIPTION:
	- screen ldas5 SSP (start1-stop1 pair) data using a second set of start1-stop1 pairs (screen)
	- keep only pairs overlapping (mode 1) or not-overlapping (mode2) the screening pairs
	- unlike xf_screen_ssp1, this function also adjusts a third timestamp (for example a midpoint or peak timestamp)

USES:
	- selecting mutually inclusive sets of SSPs

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *start1 : input array of start1-times used for screening
	long *stop1  : input array of stop1-times used for screening
	long nssp1   : number of screening SSPs in total
	long *start2 : input array of start-times to be screened
	long *stop2  : input array of stop-times to be screened
	long *extra2 : additional input array to be included for screenening
	long nssp2   : number of SSPs to be screened
	int mode     : keep (1) or reject (2) start2/stop2 pairs falling within start1/stop1 bounds
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	the number of start2/stop2 pairs passing the screening (new size of start2/stop2 arrays)
	-1 on error

	NOTE: modifies the input start2/stop2 arrays

SAMPLE CALL: keep only start2/stop2 pairs which fall within the boundaries defined by start1/stop1

	mm= xf_screen_ssp1(start1,stop1,nssp1,start2,stop2,nssp2,1,message);
	if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	nssp2=mm;

*/

# include <string.h>
# include <stdio.h>

long xf_screen_ssp2(long *start1, long *stop1, long nssp1, long *start2, long *stop2, long *extra2, long nssp2, int mode, char *message) {

	char *thisfunc="xf_screen_ssp2\0";
	long ii,jj,kk=0,tempstart,tempstop,reject;

	if(mode!=1&&mode!=2) { sprintf(message,"%s [ERROR]: invalid mode (%d) - must be 0 or 1",thisfunc,mode); return(-1); }

	/* keep overlapping SSPs */
	if(mode==1){
		for(ii=0;ii<nssp2;ii++) {
			tempstart=start2[ii];
			tempstop=stop2[ii];
			for(jj=0;jj<nssp1;jj++) {
				/* if current data falls inside current screening-SSP, copy the data */
				if(tempstart>=start1[jj] && tempstop<=stop1[jj]) {
					start2[kk]=start2[ii];
					stop2[kk]=stop2[ii];
					extra2[kk]=extra2[ii];
					/* increment the new data counter */
					kk++;
					/* do not check in any other SSP's */
					break;
		}}}
	}
	/* keep non-overlapping SSPs */
	if(mode==2){
		for(ii=0;ii<nssp2;ii++) {
			tempstart=start2[ii];
			tempstop=stop2[ii];
			reject=0;
			for(jj=0;jj<nssp1;jj++) {
				/* if either the start or stop falls inside current screening-SSP, reject it */
				if( (tempstart>=start1[jj]&&tempstart<=stop1[jj]) || (tempstop>=start1[jj]&&tempstop<=stop1[jj]) ) {
					reject=1;
					break;
			}}
			if(reject==0) {
				start2[kk]=start2[ii];
				stop2[kk]=stop2[ii];
				extra2[kk]=extra2[ii];
				kk++;
	}}}

	/* return new size of start2/stop2 arraps - this should be set to nssp2 by calling function */
	return(kk);
}
