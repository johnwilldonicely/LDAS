/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Pad an array of floating-point numbers with extra values at the beginning and/or end
	Padding is with zeros

	The original array will be modified
	NOTE: original array MUST be defined as float * in calling function
	NOTE: calling function MUST reallocate additional memory for original array before calling this function

USES:
	To create an aray of a specified size
	Note: the array should subsequently be trimmed to remove the extra values

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data   : array already holding data to be padded
	long nn       : number of elements in the array
	long npad     : size of the padding to be added
	int type      : type of  padding: 1=start, 2=end, 3=both
	char* message : array to hold reults message

RETURN VALUE:
		positive integer: number of elements in the padded array
		-1: error: invalid type
		-2: error: invalid amount of padding

SAMPLE CALL:

	# pad an array of 1000 elements with 50 values at either end
	data= xf_padarray0_f(data,1000,50,3,message);
	if(data==NULL) {
		fprintf(stderr,"Error: %s\n",message);
		exit(1);
	}

*/

#include <stdio.h>
#include <stdlib.h>
float *xf_padarray0_f(float *data, long nn, long npad, int type, char *message) {

	char *thisfunc="xf_padarray0_f\0";
	long ii,jj,kk,n2;
	float a,b,min,max,newdat1,newdat2,step1,step2,inc1,inc2;


	/* check for invalid pad type or invalid amount of padding */
	if(type<1||type>3) {
		sprintf(message,"%s [ERROR]: invalid padding type %d",thisfunc,type);
		return(NULL);
	}
	if(npad>nn) {
		sprintf(message,"%s [ERROR]: padding (%ld) must be <= data-length (%ld)",thisfunc,npad,nn);
		return(NULL);
	}
	if(npad<1) {
		sprintf(message,"%s : no padding requested",thisfunc);
		return(data);
	}

	/* determine n-elements in padded array and allocate memory */
	n2=nn+npad; if(type==3) n2+=npad;

	/* allocate extra memory */
	data= (float *)realloc(data,n2*sizeof(float));
	if(data==NULL) {
		sprintf(message,"%s [ERROR]: memory allocation error",thisfunc);
		return(NULL);
	}

	/********************************************************************************
	If padding the beginning of data, copy the data forward, last element first
	********************************************************************************/
	if(type==1||type==3) { ii=nn+npad; jj=nn; while(ii-->npad) data[ii]=data[--jj];	}

	/********************************************************************************
	Pad beginning only
	********************************************************************************/
	if(type==1) { for(ii=0;ii<npad;ii++) data[ii]=0;}

	/********************************************************************************
	Pad end only
	********************************************************************************/
	if(type==2) { for(ii=nn;ii<n2;ii++) data[ii]=0; }

	/********************************************************************************
	Pad beginning and end
	********************************************************************************/
	if(type==3) {
		for(ii=0;ii<npad;ii++) data[ii]=0;
		for(ii=nn+npad;ii<n2;ii++)  data[ii]=0;
	}

	sprintf(message,"%s : success",thisfunc);
	return (data);
}
