/*
<TAGS>stats</TAGS>

DESCRIPTION :
	Find the Spearman's rank correlation for pairs of data

USES:


DEPENDENCIES:
	xf_qsortindex1_f(xdat2,xrank,ndata);

ARGUMENTS:
	float *data1   : array of voltage values for multi-channel dataform #1
	float *data2   : array of voltage values for multi-channel dataform #2
	long ndata     : the number of elements in data1 and data2
	char *message  : a string array to hold a status message

RETURN VALUE:
	on success: Spearman's rho
	on failure: NAN
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void xf_qsortindex1_f(float *data, long *index,long nn);

double xf_spearmans1_f(float *xdat1, float *ydat1, long ndata, char *message) {

	char *thisfunc="xf_spearmans1_f\0";
	long *xrank=NULL,*yrank=NULL,*tempindex=NULL,ii,jj,kk,mm,nn;
	float *xdat2=NULL,*ydat2=NULL,*tempdata=NULL;
	double rankmean,aa,bb,SUMx,SUMy,SUMx2,SUMy2,SUMxy, SSx,SSy,SPxy,rho;

	/* ALLOCATE MEMORY */
	xdat2= malloc(ndata * sizeof *xdat2);
	ydat2= malloc(ndata * sizeof *ydat2);
	xrank= malloc(ndata * sizeof *xrank);
	yrank= malloc(ndata * sizeof *yrank);
	tempdata= malloc(ndata * sizeof *tempdata);
	tempindex= malloc(ndata * sizeof *tempindex);
	if(xdat2==NULL||ydat2==NULL||xrank==NULL||yrank==NULL||tempdata==NULL||tempindex==NULL) {
			sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
			return(-1);
	}

	/********************************************************************************/
	/* BUILD TEMPORARY ARRAYS FOR GOOD DATA AND RANKS */
	/********************************************************************************/
	for(ii=nn=0;ii<ndata;ii++) {
		if(isfinite(xdat1[ii]) && isfinite(ydat1[ii])) {
			xdat2[nn]= xdat1[ii];
			ydat2[nn]= ydat1[ii];
			nn++;
	}}
	/* If there are no good values, there can be no correlation */
	if(nn==0) return(NAN);
	/* mean rank is always going to be half the total good values and identical for both variables */
	rankmean= nn/2.0;

	/********************************************************************************/
	/* CALCULATE RANKS FOR THE X-DATA */
	/********************************************************************************/
	/* - make temp array and index 0-nn */
	for(ii=0;ii<nn;ii++) { tempdata[ii]= xdat2[ii]; tempindex[ii]= ii; }
	/* - sort the data+index - index preserves original data-position */
	xf_qsortindex1_f(tempdata,tempindex,nn);
	/* assign ranks to origial data at each saved-position (tempindex) according to the current sorted position (ii) */
	for(ii=0;ii<nn;ii++) xrank[tempindex[ii]]= ii;

	/* REPEAT FOR THE Y-DATA */
	for(ii=0;ii<nn;ii++) { tempdata[ii]= ydat2[ii]; tempindex[ii]= ii; }
	xf_qsortindex1_f(tempdata,tempindex,nn);
	for(ii=0;ii<nn;ii++) yrank[tempindex[ii]]= ii;

	// TEST	for(ii=0;ii<nn;ii++) fprintf(stderr,"%f	%ld	%f	%ld\n",xdat2[ii],xrank[ii],ydat2[ii],yrank[ii]);

	/********************************************************************************/
	/* CALCULATE SPEARMAN'S RHO (PEARSON'S R FOR THE PAIRED RANKS, ZEROED TO THE RANK-MEAN) */
	/********************************************************************************/
	SUMx=SUMy=SUMx2=SUMy2=SUMxy=0.00;
	for(ii=0;ii<nn;ii++) {
		aa= (double)xrank[ii]-rankmean;
		bb= (double)yrank[ii]-rankmean;
		SUMx+= aa;
		SUMy+= bb;
		SUMx2+= aa*aa;
		SUMy2+= bb*bb;
		SUMxy+= aa*bb;
	}
	SSx= SUMx2-((SUMx*SUMx)/nn);
	SSy= SUMy2-((SUMy*SUMy)/nn);
	SPxy= SUMxy-((SUMx*SUMy)/nn);
	if(SSy== 0.0) rho= 0.0; /* if data describes horizontal line... */
	else if(SSx== 0.0) rho= 0.0;  /* if data describes vertical line (or single point) ... */
	else rho = SPxy/(sqrt(SSx)*sqrt(SSy)); /* otherwise, if there is variability in both x and y... */


	/* SET MESSAGE, FrEE MEMORY, AND RETURN RHO */
	sprintf(message,"%s [OK]",thisfunc);
	free(xdat2);
	free(ydat2);
	free(xrank);
	free(yrank);
	free(tempdata);
	free(tempindex);

	return(rho);
}
