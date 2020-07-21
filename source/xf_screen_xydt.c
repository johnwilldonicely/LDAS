/*
<TAGS>signal_processing screen</TAGS>
DESCRIPTION:
	- screen ldas5 XYD(T) data using start-stop pairs (keep data falling between the pairs)
	- NOTE: timestamps (xydt) will be adjusted, because they are part of the xyd/t parallel-array structure

USES:
	- keeping only chunks of data falling within certain time-windows

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *start : input array of start-times used for screening
	long *stop  : input array of stop-times used for screening
	long nssp   : number of SSPs in total
	long *xydt  : input array of xyd timestamps
	float *xydx : input array of x-values
	float *xydy : input array of y-values
	float *xydd : input array of direction-values
	long ndata  : total number of items in the time[] and data[] arrays
	char *message : pre-allocated array to hold error message


RETURN VALUE:
	the number of xyd(t) samples passing the screening (-1 on error)
	the original arrays will typically be reduced

SAMPLE CALL:
	mm= xf_screen_xydt(start1,stop1,nlist,xydt,xydx,xydy,xydd,nn,message);
	if(mm==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }


*/

# include <string.h>
# include <stdio.h>

long xf_screen_xydt(long *start, long *stop, long nssp, long *xydt, float *xydx, float *xydy, float *xydd, long ndata, char *message) {

	char *thisfunc="xf_screen_xydt\0";
	long ii,jj,kk=0,tempt;

	for(ii=0;ii<ndata;ii++) {
		tempt=xydt[ii];
		/* test all the SSPs */
		for(jj=0;jj<nssp;jj++) {
			/* if current data falls inside current SSP... */
			if(tempt>=start[jj] && tempt<stop[jj]) {
				/* copy data back to kk counter */
				xydt[kk]= tempt;
				xydx[kk]= xydx[ii];
				xydy[kk]= xydy[ii];
				xydd[kk]= xydd[ii];
				/* increment the new data counter */
				kk++;
				/* do not check in any other SSPs */
				break;
			}
		}
	}

	return(kk);
}
