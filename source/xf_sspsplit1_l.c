/*
<TAGS>signal_processing time</TAGS>

DESCRIPTION:
	- split start-stop pairs into smaller sub-windows of fixed size

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *start   : pointer to original start array
	long *stop    : pointer to original stop array
	long nssp     : number of elements in start[] & stop[]
	long winsize  : size of sub-windows (samples)
	long **start  : unallocated pointer for results start array passed by calling function as &start2
	long **stop   : unallocated pointer for results stop array passed by calling function as &start2
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	success:
		- new size of start[] and stop[] arrays
		- start[] and stop[] arrays will be modified
	error:
		-1
		message array will hold error message

SAMPLE CALL:
	nn = xf_sspplit1_l(&start,&stop,nssp,25,&start2,&stop2,message);
	if(nn<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
*/

#include <stdio.h>
#include <stdlib.h>

long xf_sspsplit1_l(long *start1, long *stop1, long nssp, long winsize, long **start2, long **stop2, char *message) {

	char *thisfunc="xf_sspplit1_l\0";
	int sizeoflong=sizeof(long);
	long ii,jj,kk,nn;
	long *tempstart=NULL,*tempstop=NULL;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(start1==NULL) { sprintf(message,"%s [ERROR]: input start-array is NULL",thisfunc); return(-1); }
	if(stop1==NULL) { sprintf(message,"%s [ERROR]: input stop-array is NULL",thisfunc); return(-1); }
	if(nssp==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,nssp); return(-1); }
	if(winsize<1) { sprintf(message,"%s [ERROR]: invalid size of windows (%ld)",thisfunc,winsize); return(-1); }
	// TEST: for(ii=0;ii<10;ii++) printf("func: window %ld/%ld= %ld-%ld\n",ii,nssp,start1[ii],stop1[ii]);

	/* BUILD NEW START/STOP ARRAY */
	/* each window must be completely contained within existing start-stop pairs */
	for(ii=nn=0;ii<nssp;ii++) {
		kk= stop1[ii]-winsize;
		for(jj=start1[ii];jj<kk;jj+=winsize) {
			tempstart= realloc(tempstart,((nn+1)*sizeoflong));
			tempstop= realloc(tempstop,((nn+1)*sizeoflong));
			if(tempstart==NULL||tempstop==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
			tempstart[nn]= jj;
			tempstop[nn]= jj+winsize;
			nn++;
	}}

	/* SET POINTERS TO THE MEMORY ALLOCATED BY THIS FUNCTION - TEMPSTART & TEMPSTOP */
	(*start2)= tempstart;
	(*stop2) = tempstop;

	/* RETURN THE NEW NUMBER OF START-STOP PAIRS */
	return (nn);
}
