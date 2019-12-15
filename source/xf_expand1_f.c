/*
<TAGS>signal_processing transform</TAGS>

[JRH] 23 November 2015

DESCRIPTION:
	Expand a series of numbers so there are a user-defined number of elements [setn]
	Data is duplicated as required

	Alters input array
	NAN values will be ignored, but INF will affect the results

USES:
	- modifying different series of data so they are all the same length
	- making an irregular matrix of data have rows all the same length

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data: input, array of numbers to be resampled
	long nn: input, number of elements in data
	long setn: input, the new number of elements requested for data
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	Pointer to modified data array - NULL if memory allocation error occurs

SAMPLE CALL:

*/

#include <stdio.h>
#include <stdlib.h>

float *xf_expand1_f(float *data , long nn, long setn, char *message) {

	char *thisfunc="xf_expand1_f\0";
	int leftover_data;
	long ii,jj,kk,n2,tot;
	double binsize;
	double sum;

	if(nn==setn) return(data);

	binsize= (double)nn/(double)setn;

	data= realloc(data,setn*sizeof(*data));

	if(data!=NULL) {
		for(ii=(setn-1);ii>=0;ii--) data[ii]= data[((int)((double)ii*binsize))];
	}
	else {
		sprintf(message,"%s [ERROR]: memory allocation error",thisfunc);
	}

	return (data);
}
