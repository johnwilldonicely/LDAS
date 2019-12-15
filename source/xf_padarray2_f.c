/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Pad an array of floating-point numbers with extra values at the beginning and/or end
	Padding is a reverse-copy of the data at either end, trended to zero using a cosine taper
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
	data= xf_padarray2_f(data,1000,50,3,message);
	if(data==NULL) {
		fprintf(stderr,"Error: %s\n",message);
		exit(1);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float *xf_padarray2_f(float *data, off_t nn, off_t npad, int type, char *message) {

	char *thisfunc="xf_padarray2_f\0";
	off_t ii,jj,kk,index,n2;
	float aa;

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

	/* determine n-elements in padded array and allocate memory */
	n2=nn+npad; if(type==3) n2+=npad;

	/* allocate extra memory */
	data= (float *)realloc(data,n2*sizeof(float));
	if(data==NULL) {
		sprintf(message,"%s (memory allocation error)",thisfunc);
		return(NULL);
	}

	/* pre-calculate the correction-factor to sample-number for cosine-conversion of the padded data */
	aa=M_PI/(float)npad;



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
		/* pad beginning with reversed & tapered copy of the old beginning of the array */
		ii=npad; jj=npad; while(ii-->0) data[ii]= data[jj++] * 0.5*(1.0-cosf((float)(ii)*aa));
	}
	/********************************************************************************
	Pad end only
	********************************************************************************/
	if(type==2) {
		/* pad end with reversed & tapered copy of the old end of the array */
		ii=nn; jj=ii-1; while(ii<n2) { data[ii] = data[jj--] *  0.5*(1.0-cosf((float)(n2-ii)*aa)); ii++; }
	}
	/********************************************************************************
	Pad beginning and end
	********************************************************************************/
	if(type==3) {
		/* pad beginning with reversed & tapered copy of the old beginning of the array */
		ii=npad; jj=npad; while(ii-->0) data[ii]= data[jj++] * 0.5*(1.0-cosf((float)(ii)*aa));
		/* pad end with reversed & tapered copy of the old end of the array */
		ii=nn+npad; jj=ii-1; while(ii<n2) { data[ii] = data[jj--] *  0.5*(1.0-cosf((float)(n2-ii)*aa)); ii++; }

	}

	sprintf(message,"%s (success)",thisfunc);
	return(data);
}
