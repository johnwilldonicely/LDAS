/*
<TAGS>stats</TAGS>

DESCRIPTION:
	Normalize an array of double-precision floating-point numbers
	Data can be normalized in two ways:
		0. convert to values ranging from 0-1
		1. convert to z-scores
USES:
	Can make it easier to compare skewed datasets
	Good for analyzing trends in data with very different baselines

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data : the original data - will be transformed!
	long nn : number of elements in the array - if N<2 the data will not be altered.
	int normtype : the type of normalization
		0 for forcing data to a 0-1 range
		1 for producing z-scores

RETURN VALUE:
	None. This function modifies the data input. It should never fail provided
	N acurately reflects the memory pre-allocated for the contents of the data array

SAMPLE CALL
	xf_norm1_d(data,N,0)

*/

#include <math.h>

void xf_norm1_d(double *data,long nn,int normtype) {

	long ii;
	double min,max,sum,sumofsq,norm1,norm2;

	if(normtype==0) {
		// define norm1 (the minimum)
		min= max= data[0];
		for(ii=0;ii<nn;ii++) {if(data[ii]<min) min=data[ii];if(data[ii]>max) max=data[ii];}
		norm1= min;
		// define norm2 (the range)
		if(max>min) norm2= max-min;
		else norm2= 1.0;
	}
	else if(normtype==1) {
		sum= sumofsq= 0.0;
		// define norm1 (the mean)
		for(ii=0;ii<nn;ii++) {sum+=data[ii]; sumofsq+=data[ii]*data[ii];}
		norm1= sum/nn;
		// define norm2 (the standard deviation)
		if(nn>1) norm2= sqrt((double)((nn*sumofsq-(sum*sum))/(nn*(nn-1))));
		else norm2= 1.1;
	}
	else return;

	// transform the array
	for(ii=0;ii<nn;ii++) data[ii]= (data[ii]-norm1)/norm2;

	return;
}
