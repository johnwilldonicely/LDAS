/*
<TAGS>signal_processing screen time</TAGS>
DESCRIPTION:
	- screen time-series data using start-stop pairs (keep data falling between the pairs)
	- NOTE: timestamps are not adjusted

USES:
	- keeping only chunks of data falling within certain time-windows

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *start : input array of start-times used for screening
	long *stop  : input array of stop-times used for screening
	long nssp   : number of SSPs in total
	long *time1  : input array of data timestamps
	short *data : input array of data
	long ndata  : total number of items in the time[] and data[] arrays
	char *message : pre-allocated array to hold error message


RETURN VALUE:
	success: - the number of data[] samples passing the screening
		 - the original data[] array will typically be reduced
	error: -1

SAMPLE CALL:


*/

# include <string.h>
# include <stdio.h>

long xf_screen_ls(long *start, long *stop, long nssp, long *time1, short *data, long ndata, char *message) {

	char *thisfunc="xf_screen_ls\0";
	long ii,jj,kk=0,tempt;

	for(ii=0;ii<ndata;ii++) {
		tempt= time1[ii];
		/* test all the SSPs */
		for(jj=0;jj<nssp;jj++) {
			/* if current data falls inside current SSP... */
			if(tempt>=start[jj] && tempt<stop[jj]) {
				/* copy data back to kk counter */
				data[kk]= data[ii];
				/* increment the new data counter */
				kk++;
				/* do not check in any other SSPs */
				break;
			}
		}
	}

	return(kk);
}
