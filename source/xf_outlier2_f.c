/*
<TAGS> dt.block stats </TAGS>

DESCRIPTION:
	- remove outliers from a continuous data-series contains multiple, equal-length blocks of data
	- removal is in the form of invalidating the blocks in which the data-series has an unusual shape

	1. checks the similarity between the shape in each block against one of two references:
		a. the median value at each point in the block
		b. the mean value at each point in the block
	2. the similarity score in each block is calculated using either:
		a. Pearson's correlation
		b. mean rectified difference at each data-point
	3. the mean and std.dev of the score is calculated
	4. the score for each block is then comared against thsi mean to see if it is under the rejection threshold

USES:

DEPENDENCIES:
	int xf_compare1_d(const void *a, const void *b)
	int xf_percentile1_f(float *data, long nn, double *result)
	float xf_stats1_f(double *dat1, long nn, int digits)

ARGUMENTS:
	float *dat1      : data-array containing multiple blocks
	long ndat1       : total number of data-points
	long *start      : array marking the start of each block (curve)
	long nblocks     : total number of blocks
	long mm          : length of each block
	long zero        : sample-in-block corresponding to time "zero" (for normalization)
	char *ref        : the reference used - "mean" or "median" (see above)
	char *score      : type of comparison - "corr" or "diff"  (see above)
	float thresh     : theshold (std.deviations) for rejecting a block
	double *result_d : array holding extra results (must be able to hold 12 values)
	char *message  : character array holding error message, if any, for return value of -1 (see below)

RETURN VALUE:
	- number of blocks after removal of outliers
		- *start array will be updated so only times of "good" blocks remain
		- *result_d will be updated
			result_d[0]: the actual value of the threshold
			result_d[1]: the mean score
			result_d[2]: the std.deviation of the score

	- error: -1
		- the message[] array will contain the description)

SAMPLE CALL:
	- for a recording of 12 EEG trials. remove trials where the shape of the EEG is unusual
	- sample rate is 1KHz, and each trial lasts 3 seconds (3000 samples), with time-zero actually at the 1-seond mark
	- in this example we specify use of the median trace

	mm= xf_outlier2_f(dat1,24000500,start,12,3000,500,"median","diff",2.5,result,message);
	if(mm<0) { fprintf(stderr,"\b\n\t%s/%s: %s\n",thisprog,errmessage,message; exit(1); }

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_compare1_d(const void *a, const void *b);
int xf_percentile1_f(float *data, long nn, double *result);
float xf_stats1_f(double *dat1, long nn, int digits);


long xf_outlier2_f(float *dat1, long ndat1, long *start, long nblocks, long blocklen, long zero, char *ref, char *score, float thresh, double *result_d, char *message) {

	char *thisfunc="xf_outlier2_f\0";
	long ii,jj;
	double aa=0.0,bb,cc;

	long index1,index2,block;
	float *dat2=NULL,*dat3=NULL;
	double *sumdat2=NULL,sum,sumofsq,mean=0.0,var,sd=0.0;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(ndat1<=0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,ndat1); nblocks=-1; goto END;}
	/* CHECK INTEGRITY OF DATA */
	for(ii=0;ii<ndat1;ii++) if(isfinite(dat1[ii])) break;
	if(ii>=ndat1) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); nblocks=-1; goto END;}

	/******************************************************************************
	ALLOCATE MEMORY FOR TEMP DATA STORAGE
	******************************************************************************/
	/* allocate memory for aligned data */
	dat2= realloc(dat2,(blocklen*sizeof(*dat2))); /* actual block length is 1 larger than stop-start */
	/* allocate memory for median-data calculations */
	dat3= realloc(dat3,(nblocks*sizeof(*dat3)));
	/* allocate memory for mean arrays - allow for binning to reduce required memory size */
	sumdat2=calloc(blocklen,sizeof(double));

	if(dat2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}
	if(dat3==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}
	if(sumdat2==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);nblocks=-1; goto END;}


	/********************************************************************************
	BUILD THE REFERENCE DATASET (DAT2)
	********************************************************************************/
	/* 1. build the median-normalized-curve (dat2) for all blocks - normalized to "zero" */
	/*	- median at each sample, across blocks, calculated using dat3 */
	for(ii=0;ii<blocklen;ii++) {
		for(block=0;block<nblocks;block++) {
			index1= start[block];
			index2= start[block]+zero;
			// set pointer to current block, offset by the current sample, normalized to "zero"
			dat3[block]= dat1[index1+ii] - dat1[index2];
		}
		/* save the median for this sample (dat3), across blocks */
		if(xf_percentile1_f(dat3,nblocks,result_d)==0) dat2[ii]= result_d[5];
		else {sprintf(message,"%s [ERROR]: insufficient memory in sub-function xf_percentile1_f",thisfunc);nblocks=-1; goto END;}
	}
	//TEST: for(ii=0;ii<blocklen;ii++) printf("median[%ld]: %g\n",ii,dat2[ii]); exit(0);

	/********************************************************************************
	CALCULATE THE DIFFERENCE_SCORE
	********************************************************************************/
	/* 2. calculate the rectified mean-difference of each normalized-curve from the median-normalized-curve */
	sum= 0.0;
	sumofsq= 0.0;
	for(block=0;block<nblocks;block++) {
		bb= 0.0; // summed rectified difference for this block
		index1= start[block];
		index2= start[block]+zero;
		for(ii=0;ii<blocklen;ii++) {
			// difference of normalized data-point from pre-normalized median curve
			aa= dat2[ii] - (dat1[index1+ii] - dat1[index2]) ;
			if(aa<0) aa= 0.0-aa; // rectify so all deviations are positive
			bb+= aa;
		}
		cc= bb/blocklen; // mean rectified difference from median curve, for this block
		sum+= cc;
		sumofsq+= cc*cc;
		dat3[block]= cc; // reporpose dat3 to store the mean rectified difference for each block
	}
	/* calculate the mean "mean-rectified-difference", and corresponding standard deviation */
	mean= sum/(double)nblocks;
	var= ( sumofsq - ( (sum*sum)/(double)nblocks )) / ((double)nblocks-1.0) ;
	if(blocklen>1) sd= sqrt(var);
	else var= sd= 0.0;

	/* 3. determine which blocks stay in based on mean deviation from median curve */
	aa= sd*thresh;
	for(block=jj=0;block<nblocks;block++) {
		if(dat3[block]< aa) start[jj++]= start[block];
	}
	nblocks= jj;

END:
	result_d[0]= aa; // actual threshold
	result_d[1]= mean; // mean score
	result_d[2]= sd; // standard deviation of score
	if(dat2!=NULL) free(dat2);
	if(dat3!=NULL) free(dat3);
	if(sumdat2!=NULL) free(sumdat2);
	return(nblocks);
}
