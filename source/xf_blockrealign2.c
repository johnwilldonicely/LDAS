/*
<TAGS>signal_processing time</TAGS>
DESCRIPTION:
	Restore original timestamps based on block-definitions previously used to keep only subsections of a dataset
	Block-definitions in this case are in the form of start-stop pairs

USES:
	Say an original data series was adjusted to remove noisy data
	If spike events are identified in the modified time-series, adjustment is required to put the
		spike times back in register with the original time series
	Example:
		- an original time series of 0-10 in which a spike occurs at sample 8
		- say samples (2-6) were removed due to noise
		- this is equivalent of keeping 2 blocks, defined using start-stop pairs 0,2 and 7,11
		- spike detection on the compressed data would show spike detection at sample #3
		- this function will correct for compression, restoring the original timestamps

		original                  compressed    corrected
		    _________   *               *             *
		0 1 2 3 4 5 6 7 8 9 10    0 1 2 3 4 5   0 1 7 8 9 10


DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *samplenum : input holding timestamps (sample-numbers) needing realignment - assumes they are in ascending order
	long nn : the size of the samplenum array
	long *bstart : array holding the start-samples (included) for each kept block of the original data
	long *bstop : array holding the stop-samples (not included) for each kept block
	long nblocks : the number of blocks
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	samplenum array will be adjusted
	char array will hold message (if any)

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>

int xf_blockrealign2(long *samplenum, long nn, long *bstart, long *bstop, long nblocks, char *message) {

	char *thisfunc="xf_blockrealign2\0";
	long ii,jj,kk,thresh,adj,block,sampmax;

	/* integrity checks */
	if(nn<1) { sprintf(message,"%s [ERROR]: no timestamps",thisfunc); return(-1); }
	if(nblocks<1) { sprintf(message,"%s [ERROR]: no blocks",thisfunc); return(-1); }
	/* get largest sample-number - assumes sample-numbers are in ascending order! */
	sampmax=samplenum[nn-1];
	/* get the cumulative number of samples and check blocks are non-overlapping and run in ascending order */
	kk=bstop[0]-bstart[0];
	for(ii=1;ii<nblocks;ii++) {
		if(bstart[ii]<bstart[ii-1]) { sprintf(message,"%s [ERROR]: block-starts are not in ascending order",thisfunc); return(-1); }
		if(bstart[ii]<bstop[ii-1])  { sprintf(message,"%s [ERROR]: block %ld overlaps with block %ld",thisfunc,ii,(ii-1)); return(-1); }
		kk+=bstop[ii]-bstart[ii];
	}
	if(kk<=sampmax) { sprintf(message,"%s [ERROR]: block-sum (%ld) <= max-samplenum (%ld): blocks couldn't have generated input",thisfunc,kk,sampmax); return(-1); }

	/* initialize block-counter */
	block=0;
	/* set initial adjustment to the start-sample of the first block (which might be zero) */
	adj=bstart[block];
	/* initial time-threshold for increasing the adjustment is the size of the first block */
	thresh= bstop[block] - bstart[block];

	/* for every timestamp, apply the adjustment */
	for(ii=0;ii<nn;ii++) {
		while(samplenum[ii]>=thresh) {
			/* use next block - break if that was the last one */
			if(++block >= nblocks) break;
			/* update the time-adjustment by the samples elapsed since the previous block */
			adj+= bstart[block]-bstop[block-1];
			/* update the sample-threshold by the size of the current block */
			thresh+= bstop[block] - bstart[block];
		}
		/* apply the current adjustment */
		samplenum[ii]+=adj;
	}
	return (0);
}
