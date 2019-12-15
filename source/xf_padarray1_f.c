/*
<TAGS>signal_processing</TAGS>
DESCRIPTION:
	Pad an array of floating-point numbers with extra values at the beginning and/or end
	Padding is designed to gradually reduce the rate of change at either end of the array to zero
	Total change in padded region is scaled to a fraction of the total data range
	The original array will be modified
		- the modified array size will be n+npad (if type=1 or type=2) or n+(npad*2) (if type=3)
		- this function allocates additional memory for the padded array

USES:
	To prevent edge-effects when filtering the array
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
	success: pointer to padded array
	failure: NULL

SAMPLE CALL:

	# pad an array of 1000 elements with 50 values at either end
	data= xf_padarray1_f(data,1000,50,3,message);
	if(data==NULL) {
		fprintf(stderr,"Error: %s\n",message);
		exit(1);
	}

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float *xf_padarray1_f(float *data, long nn, long npad, int type, char *message) {

	char *thisfunc="xf_padarray1_f\0";
	float a,b,min,max,newdat1=0,newdat2=0,step1=0,step2=0,inc1=0,inc2=0;
	long ii,jj,kk,n2,halfnpad;
	float aa;

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

	/* determine half-size of padding */
	halfnpad=(long)((double)(npad/10.0));

	/* allocate extra memory */
	data= (float *)realloc(data,n2*sizeof(float));
	if(data==NULL) {
		sprintf(message,"%s (memory allocation error)",thisfunc);
		return(NULL);
	}

	if(type==1||type==3) {
		/* calculate the default size of the first step in padding, based on the local data range */
		min=max=data[0];for(ii=1;ii<npad;ii++) {a=data[ii]; if(a<min)min=a; if(a>max) max=a;}
		b=(max-min)/npad;
		/* initialize first step, first value, and first incriment for padding at beginning of array */
		newdat1=data[0];
		step1=data[0]-data[1];
		if(step1>0.0) step1=b;
		else step1=-1.0*b;
		inc1=step1/(float)(npad-1);
		newdat1+=step1;
		step1-=inc1;
	}
	if(type==2||type==3) {
		/* calculate the default size of the first step in padding, based on the local data range */
		min=max=data[(nn-1)];for(ii=(nn-npad);ii<nn;ii++) {a=data[ii]; if(a<min)min=a; if(a>max) max=a;}
		b=(max-min)/npad;
		/* initialize first step, first value, and first incriment for padding at end of array */
		newdat2=data[(nn-1)];
		step2=data[(nn-1)]-data[(nn-2)];
		if(step2>0.0) step2=b;
		else step2=-1.0*b;
		inc2=step2/(float)(npad-1);
		newdat2+=step2;
		step2-=inc2;
	}

	/********************************************************************************
	If padding the beginning of data, copy the data forward, last element first
	********************************************************************************/
	if(type==1||type==3) { ii=nn+npad; jj=nn; while(ii-->npad) data[ii]=data[--jj];	}

	/********************************************************************************
	Pad beginning only
	********************************************************************************/
	if(type==1) {
		/* trend the beginning to a zero rate-of-change */
		ii=npad-1; jj=npad+1; while(ii>=0) { data[ii--]=newdat1; newdat1+=step1; step1-=inc1; }
	}
	/********************************************************************************
	Pad end only
	********************************************************************************/
	if(type==2) {
		/* trend the end to a zero rate-of-change */
		ii=nn; jj=ii-1; while(ii<n2) { data[ii++]=newdat2; newdat2+=step2; step2-=inc2; }
	}

	/********************************************************************************
	Pad beginning and end
	********************************************************************************/
	if(type==3) {
		/* trend the beginning to a zero rate-of-change */
		ii=npad-1; jj=npad+1; while(ii>=0) { data[ii--]=newdat1; newdat1+=step1; step1-=inc1; }
		/* trend the end to a zero rate-of-change */
		ii=nn+npad; jj=ii-1; while(ii<n2) { data[ii++]=newdat2; newdat2+=step2; step2-=inc2; }
	}

	sprintf(message,"%s (success)",thisfunc);
	return(data);
}
