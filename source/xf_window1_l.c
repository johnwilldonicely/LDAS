/*
<TAGS>signal_processing time</TAGS>

DESCRIPTION:
	Fill an array with start-indices for a series of equally-spaced or equal-sized windows
	Assumes data is zero-offset (i.e. the first window will always start at index zero)
	[JRH, 18 March 2013]

USES:
	defining windows for stepwise analysis of data
	eg. defining windows for short-time FFT

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	long n: the total number of elements in the data to be windowed
	long winsize: the desired size of the windows (number of elements)
	int equalsize: 	a flag specifying whether all windows should be exactly the same size
			set to 0 to preserve equal-spacing
			set to 1 to ensure all windows are the same size
				- i.e. if the last window has < winsize elements, the start-index is adjusted backwards
				- adjustment only happens if n is not an exact multiple of nwin
	*nwin: the total number of windows detected (size of the window-index array which is returned)
		this value will be adjusted by the function
			- if nwin is -1 upon return, this indicates a memory allocation error
			- if nwin is 0 upon return, this indicates insufficient data for even a single window

RETURN VALUE:
	A pointer to an array of type long integer which indicates the start-index of each window

SAMPLE PROGRAM: define windows for a dataset of 10 elements, where each window should be 3 elemnts long:

	#include <stdio.h>
	#include <stdlib.h>

	long i,nwin,n=10,winsize=3,*windex=NULL;
	windex=xf_window1_l(n,winsize,&nwin,1);
	for(i=0;i<nwin;i++) printf("%ld\n",windex[i]);
	free(windex);

	//output looks like this... note last window starts at 7, preserving nwin size of 3 for the last window
	//	0
	//	3
	//	6
	//	7


*/

#include <stdlib.h>
long *xf_window1_l(long nn, long winsize, int equalsize, long *nwin) {

	long ii,jj,kk;
	long *windex=NULL;
	size_t size1;

	size1= sizeof(*windex);
	kk=0;
	for(ii=0;ii<nn;ii+=winsize) {
		if((windex= realloc(windex,(kk+1)*size1))!=NULL) { windex[kk++]= ii; }
		else { *nwin= -1 ; return(NULL); }
	}

	if(ii>nn && equalsize==1) windex[(kk-1)]= nn-winsize;

	/* update the number of windows */
	*nwin= kk;
	return(windex);
}
