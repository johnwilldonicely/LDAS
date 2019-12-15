/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Pad an array of floating-point numbers with extra values at the beginning and/or end
	Padding is "sample-and-hold" : the first and/or last values in the input array
	Benefit: cannot introduce any sharp transitions or oscillations into the data
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
	long* nn      : number of elements in the array, passed as address, will be updated
	long npad     : size of the padding to be added (must be <=n)
	int type      : type of  padding: 1=start, 2=end, 3=both
	char* message : array to hold reults message

RETURN VALUE:
	success: pointer to padded array, nn variable will be updated
	failure: NULL

SAMPLE CALL:

	# pad an array of 1000 elements with 50 values at either end
	nn= 1000;
	data= xf_padarray4_f(data,&nn,50,3,message);
	if(data==NULL) { fprintf(stderr,"Error: %s\n",message);	exit(1);}

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float *xf_padarray4_f(float *data, long *nn, long npad, int type, char *message) {

	char *thisfunc="xf_padarray4_f\0";
	long ii,jj,kk,index,n1,n2;
	float aa,bb;
	double sum;

	/* make temporary holder for original array size */
	n1= *nn;

	/* check for invalid pad type or invalid amount of padding */
	if(type<1||type>3) {
		sprintf(message,"%s (invalid padding type %d)",thisfunc,type);
		return(NULL);
	}
	if(npad>n1) {
		sprintf(message,"%s (padding (%ld) must be <= data-length (%ld))",thisfunc,npad,n1);
		return(NULL);
	}
	if(npad<1) {
		sprintf(message,"%s : no padding requested",thisfunc);
		return(data);
	}

	/* determine values to be used as padding */
	aa= data[0];
	bb= data[n1-1];

	/* determine n-elements in padded array and allocate memory */
	n2= n1+npad;
	if(type==3) n2+= npad;

	/* allocate extra memory */
	data= (float *)realloc(data,n2*sizeof(float));
	if(data==NULL) {
		sprintf(message,"%s (memory allocation error)",thisfunc);
		return(NULL);
	}


//TEST: fprintf(stderr,"n1=%ld\n",n1);
//TEST: fprintf(stderr,"type=%ld\n",type);
//TEST: fprintf(stderr,"npad=%ld\n",npad);
//TEST: fprintf(stderr,"n2=%ld\n",n2);

	/********************************************************************************
	If padding the beginning of data, copy the data forward, last element first
	********************************************************************************/
	if(type==1||type==3) {
		ii= n1+npad; jj=n1; while(ii-->npad) data[ii]=data[--jj];
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
		for(ii=n1;ii<n2;ii++) data[ii]= bb;
	}
	/********************************************************************************
	Pad beginning and end
	********************************************************************************/
	if(type==3) {
		for(ii=0;ii<npad;ii++) data[ii]= aa;
		for(ii=(n1+npad);ii<n2;ii++) data[ii]= bb;

	}

	sprintf(message,"%s (success)",thisfunc);
	(*nn)= n2;
	return(data);
}
