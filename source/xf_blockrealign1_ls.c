/*
<TAGS>signal_processing time</TAGS>

DESCRIPTION:
	Keep a subset of timestamps based on block-definitions, collapsing across the removed blocks
	Block-definitions in this case are in the form of start-stop pairs

USES:
	Adjust data to remove noisy sections
	Example:
		- an original time series of 0-10 in which a spike occurs at sample 8
		- say we want ti remove samples 2-6 due to noise
		- this is equivalent of keeping 2 blocks, defined using start-stop pairs 0,2 and 7,11
		- in the adjusted data, sample-8 is now sample-3

		original                  compressed
		    _________   *               *
		0 1 2 3 4 5 6 7 8 9 10    0 1 2 3 4 5


DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long *samplenum : input holding timestamps (sample-numbers) needing realignment - assumes they are in ascending order
	short *class : input holding clssifier for each timestamp
	long nn : the size of the samplenum array
	long *bstart : array holding the start-samples (included) for each kept block of the original data
	long *bstop : array holding the stop-samples (not included) for each kept block
	long nblocks : the number of blocks
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	new sample-number array-length on success, -1 on error
	samplenum array will be adjusted
	char array will hold message (if any)

SAMPLE CALL:


*/

#include <stdio.h>
#include <stdlib.h>

long xf_blockrealign1_ls(long *samplenum, short *class, long nn, long *bstart, long *bstop, long nblocks, char *message) {

	char *thisfunc="xf_blockrealign1_ls\0";
	long ii,jj,kk,mm,adj,block;

	/* INTEGRITY CHECKS */
	if(nn<1) { sprintf(message,"%s [ERROR]: no timestamps",thisfunc); return(-1); }
	if(nblocks<1) { sprintf(message,"%s [ERROR]: no blocks",thisfunc); return(-1); }

	/* GET THE CUMULATIVE NUMBER OF SAMPLES AND CHECK BLOCKS ARE NON-OVERLAPPING AND RUN IN ASCENDING ORDER */
	kk=bstop[0]-bstart[0];
	for(ii=1;ii<nblocks;ii++) {
		if(bstart[ii]<bstart[ii-1]) { sprintf(message,"%s [ERROR]: block-starts are not in ascending order",thisfunc); return(-1); }
		if(bstart[ii]<bstop[ii-1])  { sprintf(message,"%s [ERROR]: block %ld overlaps with block %ld",thisfunc,ii,(ii-1)); return(-1); }
	}

	/* INITIALIZE BLOCK-COUNTER */
	block=0;

	/* SET INITIAL ADJUSTMENT TO THE START-SAMPLE OF THE FIRST BLOCK (WHICH MIGHT BE ZERO) */
	adj= bstart[block];

	/* FOR EVERY TIMESTAMP, KEEP AND ADJUST IF IT FALLS WITHIN A BLOCK */
	for(ii=mm=0;ii<nn;ii++) {
		kk= samplenum[ii];
		/* if we havent even reached the first block yet, continue */
		if(kk<bstart[block]) continue;
		/* if we're beyond the current block, increment blocks until bstop>kk, and update adj */
		while(kk>=bstop[block]) {
			block++;
			if(block<nblocks) adj+= bstart[block]-bstop[block-1];
			else break;
		}
		/* if we've got to the last block without finding a samlple-number < bstop, then break */
		if(block>=nblocks) break;
		/* otherwise kk>bstop, so reassign the samplenum applying the adjustment, and increment the mm counter */
		if(kk>=bstart[block]) {
			samplenum[mm]= kk-adj;
			class[mm]= class[ii];
			mm++;
		}
	}

	/* return the revised length of the array */
	return (mm);
}
