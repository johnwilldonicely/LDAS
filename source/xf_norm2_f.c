/*
<TAGS>stats</TAGS>
DESCRIPTION:
	Normalize an array of double-precision floating-point numbers
	Data can be normalized in two ways:
		0. convert to values ranging from 0-1
		1. convert to z-scores

	Similar to xf_norm1_d but NAN and INF are not included in calculations

USES:
	Can make it easier to compare skewed datasets
	Good for analyzing trends in data with very different baselines

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data : the original data - will be transformed!
	long ndata   : number of elements in the array - if N<2 the data will not be altered.
	int normtype : the type of normalization
		0 for forcing data to a 0-1 range
		1 for producing z-scores

RETURN VALUE:
	The number of valid numerical data points in data
	OR
		-1: memory allocation error
		-2: no finite numbers in data
		-3: invalid normalization type

	This function modifies the data input. It should never fail provided N
	accurately reflects the memory pre-allocated for the contents of the data array

SAMPLE CALL
	x= xf_norm1_f(data,ndata,0)
	if(x==-1) { fprintf(stderr,"\b\n\t--- Error [%s]: insufficient memory\n\n",thisprog); exit(1); }
	if(x==-2) { fprintf(stderr,"\b\n\t--- Error [%s]: no valid numbers\n\n",thisprog); exit(1); }
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int xf_norm2_f(float *data,long ndata,int normtype) {

	long int ii,jj,nn;
	double *tempdata=NULL;
	double aa,min,max,sum=0.0,sumofsq=0.0,norm1=0.0,norm2=1.0;

	/* BUILD TEMPORARY DATA ARRAY FROM ONLY THE VALID NUMERICAL DATA */
	if((tempdata=(double *)realloc(tempdata,ndata*sizeof(double)))==NULL) return(-1);
	for(ii=nn=0;ii<ndata;ii++) { if(isfinite(data[ii])) tempdata[nn++]= (double)data[ii]; }

	/* IF THERE IS NO VALID DATA, RETURN AN ERROR MESSAGE */
	if(nn<1) { free(tempdata); return(-2); }

	/* NORMALIZE TYPE-0: DATA CONVERTED TO A RANGE FROM 0 TO 1 */
	if(normtype==0) {
		// define norm1 (the minimum)
		min=max=tempdata[0];
		for(ii=0;ii<nn;ii++) {if(tempdata[ii]<min) min=tempdata[ii];if(tempdata[ii]>max) max=tempdata[ii];}
		norm1= min;
		// define norm2 (the range)
		if(max>min) norm2= max-min;
		else norm2=1.0;
	}
	/* NORMALIZE TYPE-1: tempdata CONVERTED TO Z-SCORES */
	else if(normtype==1) {
		// define norm1 (the mean)
		for(ii=0;ii<nn;ii++) {sum+=tempdata[ii]; sumofsq+= tempdata[ii]*tempdata[ii];}
		norm1= sum/(double)nn;
		// define norm2 (the standard deviation)
		if(nn>1) norm2= sqrt((double)((nn*sumofsq-(sum*sum))/(nn*(nn-1))));
		else norm2=1.1;
	}
	else return(-3);

	/* TRANSFORM THE ARRAY - ONLY THE VALID DATA POINTS */
	for(ii=0;ii<ndata;ii++) {
		if(isfinite(data[ii])) data[ii]= (data[ii]-(float)norm1)/(float)norm2;
	}

	free(tempdata);
	return(nn);
}
