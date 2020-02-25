/*
<TAGS> dt.block stats </TAGS>

DESCRIPTION:
	- remove outliers from a continuous data-series
	- assumes the series contains multiple, equal-length blocks of data
	- blocks may represent individal trials, detected events, etc.
	- removal= invalidating blocks in which the data has an unusual shape

	1. calculate median shape for all blocks of data
	2. calculate similarity score for each block - mean rectified difference
	3. calculate the median and median-absolute-deviation for the scores
	4. test each score to see if it is under the rejection threshold

	- NOTE: input (dat1) should be interpolated to remove non-finite numbers

REFERENCES:
	Boris Iglewicz and David Hoaglin (1993), "Volume 16: How to Detect and Handle Outliers", The ASQC Basic References in Quality Control: Statistical Techniques, Edward F. Mykytka, Ph.D., Editor.
	- modified Z-scores > 3.5 should be labelled potential outliers.

DEPENDENCIES:
	int xf_compare1_d(const void *a, const void *b)
	int xf_percentile1_f(float *data, long nn, double *result)

ARGUMENTS:
	float *dat1      : data-array containing multiple blocks of trials, events, etc
	long ndat1       : total number of data-points
	long *start      : array marking the start of each block
	long nblocks     : total number of blocks
	long mm          : length of each block
	long zero        : sample-in-block corresponding to time "zero" (for normalization)
	float thresh     : theshold (std.deviations) for rejecting a block
	double *result_d : array holding extra results (must be able to hold 12 values)
	char *message  : character array holding error message, if any, for return value of -1 (see below)

RETURN VALUE:
	- number of blocks after removal of outliers
		- *start array will be updated so only times of "good" blocks remain
		- *result_d will be updated
			result_d[0]: the median score
			result_d[1]: the median-absolute-deviation for the scores (MAD)
			result_d[2]: the adjusted threshold (thresh*MAD)

	- error: -1
		- the message[] array will contain the description)

SAMPLE CALL:
	# for a recording of 12 EEG trials. remove trials where the shape of the EEG is unusual
	# sample rate is 1KHz, and each trial lasts 3 seconds (3000 samples), with time-zero actually at the 1-seond mark
	mm= xf_outlier2_f(dat1,24000500,start,12,3000,500,2.5,result,message);
	if(mm<0) { fprintf(stderr,"\b\n\t%s/%s: %s\n",thisprog,errmessage,message; exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int xf_compare1_d(const void *a, const void *b);
int xf_percentile1_d(double *data, long nn, double *result);

long xf_outlier2_f(float *dat1, long ndat1, long *start, long nblocks, long blocklen, long zero, float thresh, double *result_d, char *message) {

	char *thisfunc="xf_outlier2_f\0";
	long ii,jj,kk;
	double aa=0.0,bb;

	long index1,index2,block;
	float  *pdat1=NULL;
	double *dat2=NULL,*dat3=NULL,score;
	double sumx,sumy,sumxy,sumx2,sumy2,spxy,ssx,ssy,median=0.0,mad=0.0;

	/******************************************************************************
	CHECKS ON ARGUMENTS AND INPUTS
	******************************************************************************/
	/* check validity of arguments */
	if(ndat1<=0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,ndat1); nblocks=-1; goto END; }
	if(blocklen>=ndat1) { sprintf(message,"%s [ERROR]: block-length (%ld) must be less than length of input (%ld)",thisfunc,blocklen,ndat1); nblocks=-1; goto END; }
	if(zero>=blocklen) { sprintf(message,"%s [ERROR]: must set zero (%ld) to be less than block-length (%ld)",thisfunc,zero,blocklen); nblocks=-1; goto END; }

	/* check integrity of data */
	for(ii=0;ii<ndat1;ii++) if(isfinite(dat1[ii])) break;
	if(ii>=ndat1) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); nblocks=-1; goto END; }
	/* check validity of the blocks */
	for(ii=jj=kk=0;ii<nblocks;ii++) {
		index1=start[ii];
		index2=index1+blocklen;
		if(index1<0||index1>=ndat1) jj++; // original starts are out of range
		if(index2<0||index2>=ndat1) kk++; // size of block puts it out of range
	}
	if(jj>0) { sprintf(message,"%s [ERROR]: %ld invalid blocks (start <0 or >= input_length (%ld)",thisfunc,jj,ndat1); nblocks=-1; goto END; }
	if(kk>0) { sprintf(message,"%s [ERROR]: %ld invalid blocks - blocklength (%ld) causes them to fall beyond length of input  (%ld)",thisfunc,kk,blocklen,ndat1); nblocks=-1; goto END; }


	/******************************************************************************
	ALLOCATE MEMORY FOR TEMP DATA STORAGE
	******************************************************************************/
	/* allocate memory for reference curve */
	dat2= realloc(dat2,(blocklen*sizeof(*dat2))); /* actual block length is 1 larger than stop-start */
	if(dat2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}
	/* allocate memory for scores for each block  - also used for temp storage in median calculations */
	dat3= realloc(dat3,(nblocks*sizeof(*dat3)));
	if(dat3==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}


	/********************************************************************************
	BUILD THE REFERENCE CURVE (DAT2), NORMALIZED TO "ZERO"
	********************************************************************************/
	/* build the median-normalized-curve (dat2) for all blocks - normalized to "zero" */
	/*	- median at each sample, across blocks */
	/*	- use dat3 to temporarily store the n=nblocks samples required to calculate the median */
	for(ii=0;ii<blocklen;ii++) {
		for(block=0;block<nblocks;block++) {
			index1= start[block];
			index2= start[block]+zero;
			// temporarily store all the normalized samples for this timepoint, across blocks
			dat3[block]= dat1[index1+ii] - dat1[index2];
		}
		/* save the median for this sample (dat3), as dat2 */
		if(xf_percentile1_d(dat3,nblocks,result_d)==0) dat2[ii]= result_d[5];
		else {sprintf(message,"%s [ERROR]: insufficient memory in sub-function xf_percentile1_f",thisfunc);nblocks=-1; goto END;}
	}

	//TEST:	for(ii=0;ii<blocklen;ii++) printf("median[%ld]: %g\n",ii,dat2[ii]); //exit(0);

	/********************************************************************************
	CALCULATE THE DIFfERENCE SCORE - STORE IN DAT3
	- rectified mean-difference of each normalized-curve from the median-normalized-curve
	********************************************************************************/
	for(block=0;block<nblocks;block++) {
		bb= 0.0; // summed rectified difference for this block
		index1= start[block];
		index2= start[block]+zero;
		for(ii=0;ii<blocklen;ii++) {
			// difference of normalized data-point from pre-normalized reference data (dat2)
			aa= (dat1[index1+ii]-dat1[index2]) - dat2[ii] ;
			if(aa<0) aa= 0.0-aa; // rectify so all deviations are positive
			bb+= aa;
		}
		dat3[block]= bb/blocklen; // dat3 becomes the score for each block
	}

	/* ALTERNATIVE: calculate the correlation between each block and the median block */
	// /* NOTE this does not perform as well as the difference score - not sensitive to scale */
	// for(block=0;block<nblocks;block++) {
	// 	sumx=sumy=sumxy=sumx2=sumy2= 0.0;
	// 	pdat1= dat1+start[block];
	// 	for(ii=0;ii<blocklen;ii++) { aa= pdat1[ii]; bb= dat2[ii]; sumx+= aa; sumy+= bb; sumx2+= aa*aa; sumy2+= bb*bb; sumxy+= aa*bb; }
	// 	ssx= sumx2-((sumx*sumx)/blocklen);
	// 	ssy= sumy2-((sumy*sumy)/blocklen);
	// 	spxy= sumxy-((sumx*sumy)/blocklen);
	// 	if(ssx==0.0 || ssy==0.0) {sprintf(message,"%s [ERROR]: can't calculate correlation for single points, or vertical or horizontal line",thisfunc);nblocks=-1; goto END;}
	// 	else dat3[block]= spxy/(sqrt(ssx)*sqrt(ssy));
	// }

	/********************************************************************************
	CALCULATE THE MEDIAN SCORE, AND MEDIAN ABSOLUTE DEVIATION
	********************************************************************************/
	/* get the median of the scores (dat3) */
	if(xf_percentile1_d(dat3,nblocks,result_d)==0) median= result_d[5];
	else {sprintf(message,"%s [ERROR]: insufficient memory in sub-function xf_percentile1_f",thisfunc);nblocks=-1; goto END;}
	/* re-use dat2 - realloc if necessary */
	if(blocklen<nblocks) dat2= realloc(dat2,(nblocks*sizeof(*dat2)));
	if(dat2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}
	/* calculate the median absolute deviation (MAD) */
	for(block=0;block<nblocks;block++) { aa= dat3[block]-median; if(aa<0.0) aa=0.0-aa; dat2[block]= aa; }
	if(xf_percentile1_d(dat2,nblocks,result_d)==0) mad= result_d[5];
	else {sprintf(message,"%s [ERROR]: insufficient memory in sub-function xf_percentile1_f",thisfunc);nblocks=-1; goto END;}

	/********************************************************************************
	DETERMINE WHICH BLOCKS STAY IN
	********************************************************************************/
	aa= thresh*mad; // pre-calculate the modified difference-threshold
	for(block=jj=0;block<nblocks;block++) {
		if( (dat3[block]-median) < aa ) start[jj++]= start[block];
	}
	nblocks= jj;

END:
	result_d[0]= median; // the median score
	result_d[1]= mad; // median-absolute-deviation
	result_d[2]= thresh*mad; // adjusted threshold
	if(dat2!=NULL) free(dat2);
	if(dat3!=NULL) free(dat3);
	return(nblocks);
}
