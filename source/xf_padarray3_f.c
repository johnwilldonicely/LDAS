/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Pad an array of floating-point numbers with extra values at the beginning and/or end
	Padding is composed of the mean for the first and/or last npad values in the input array
	The original array will be modified
		- the modified array size will be n+npad (if type=1 or type=2) or n+(npad*2) (if type=3)
		- this function allocates additional memory for the padded array

USES:
	To prevent edge-effects when filtering the array
	Note: the array should subsequently be trimmed to remove the extra values

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float* data   : array already holding data to be padded
	long nn       : number of elements in the array
	long npad     : size of the padding to be added (must be <=n)
	int type      : type of  padding: 1=start, 2=end, 3=both
	char* message : array to hold reults message

RETURN VALUE:
	success: pointer to padded array
	failure: NULL

SAMPLE CALL:

	# pad an array of 1000 elements with 50 values at either end
	data= xf_padarray3_f(data,1000,50,3,message);
	if(data==NULL) {
		fprintf(stderr,"Error: %s\n",message);
		exit(1);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float *xf_padarray3_f(float *data, long nn, long npad, int type, char *message) {

	char *thisfunc="xf_padarray3_f\0";
	long ii,jj,kk,index,n2;
	float aa,bb;
	double sum;

//TEST: fprintf(stderr,"nn=%d\n",nn);
//TEST: fprintf(stderr,"type=%d\n",type);
//TEST: fprintf(stderr,"npad=%d\n",npad);

	/* check for invalid pad type or invalid amount of padding */
	if(type<1||type>3) {
		sprintf(message,"%s (invalid padding type %d)",thisfunc,type);
		return(NULL);
	}
	if(npad>nn) {
		sprintf(message,"%s (padding (%ld) must be <= data-length (%ld))",thisfunc,npad,nn);
		return(NULL);
	}
	if(npad<1) {
		sprintf(message,"%s : no padding requested",thisfunc);
		return(data);
	}

	/* determine values to be used as padding */
	sum=0.0; for(ii=0;ii<npad;ii++) sum+=data[ii]; aa= sum/(double)npad;
	sum=0.0; for(ii=(nn-npad);ii<nn;ii++) sum+=data[ii]; bb= sum/(double)npad;

	/* determine n-elements in padded array and allocate memory */
	n2=nn+npad; if(type==3) n2+=npad;

	/* allocate extra memory */
	data= (float *)realloc(data,n2*sizeof(float));
	if(data==NULL) {
		sprintf(message,"%s (memory allocation error)",thisfunc);
		return(NULL);
	}


	/********************************************************************************
	If padding the beginning of data, copy the data forward, last element first
	********************************************************************************/
	if(type==1||type==3) {
		ii=nn+npad; jj=nn; while(ii-->npad) data[ii]=data[--jj];
	}


	/********************************************************************************
	Pad beginning only
	********************************************************************************/
	if(type==1) {
		for(ii=0;ii<npad;ii++) data[ii]= aa;
	}
	/********************************************************************************
	Pad end only
	********************************************************************************/
	if(type==2) {
		for(ii=nn;ii<n2;ii++) data[ii]= bb;
	}
	/********************************************************************************
	Pad beginning and end
	********************************************************************************/
	if(type==3) {
		for(ii=0;ii<npad;ii++) data[ii]= aa;
		for(ii=(nn+npad);ii<n2;ii++) data[ii]= bb;

	}

	sprintf(message,"%s (success)",thisfunc);
	return(data);
}
