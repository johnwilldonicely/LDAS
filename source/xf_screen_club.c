/*
<TAGS>signal_processing screen time spikes</TAGS>
DESCRIPTION:
	- screen time-series club using start-stop pairs (keep club falling between the pairs)
	- NOTE: timestamps (clubt) will be adjusted, because they are part of the club/t parallel-array structure

USES:
	- keeping only chunks of club falling within certain time-windows

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *start : input array of start-times used for screening
	long *stop  : input array of stop-times used for screening
	long nssp   : number of SSPs in total
	long *clubt : input array of club timestamps
	short *club : input array of club cluster-id's
	long nclub  : total number of items in the clubt[]] and club[] arrays
	char *message : pre-allocated array to hold error message


RETURN VALUE:
	the number of clubt[] and club[] samples passing the screening (-1 on error)
	the original clubt[] and club[] arrays will typically be reduced

SAMPLE CALL:


*/

# include <string.h>
# include <stdio.h>

long xf_screen_club(long *start, long *stop, long nssp, long *clubt, short *club, long nclub, char *message) {

	char *thisfunc="xf_screen_club\0";
	long ii,jj,kk=0,tempt;

	for(ii=0;ii<nclub;ii++) {
		tempt= clubt[ii];
		/* test all the SSPs */
		for(jj=0;jj<nssp;jj++) {
			/* if current club falls inside current SSP... */
			if(tempt>=start[jj] && tempt<stop[jj]) {
				/* copy club back to kk counter */
				clubt[kk]= clubt[ii];
				club[kk]= club[ii];
				/* increment the new club counter */
				kk++;
				/* do not check in any other SSP's */
				break;
			}
		}
	}

	return(kk);
}
