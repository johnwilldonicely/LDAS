/*
<TAGS>signal_processing filter</TAGS>
DESCRIPTION:
	Apply a boxcar-averaging smoother to a data series (acts as a low-pass filter)
	Uses a single-sample-step sliding window
	Data at the beginning and end of the data is the average of a fraction of a window looking forward/backward, respectively
	This version uses an internal buffer so that the data array can be overwriten
		- hence, no memory required for an output array

	NOTE: if data contains invalid values, NAN or INF, output will be invalid
		- in such cases it is recommended to interpolate the data before passing to this function
USES:
	Signal analysis

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *data    : pointer to array holding time series to be smoothed
	size_t nn      : length of the data & output arrays
	size_t halfwin : half-window size of the smoothing window - final window will be (halfwin*2)+1
	char *message  : feedback returned to the calling function, which should allocate memory for this array (256 characters)

RETURN VALUE:
	 0 on success
	-1 on error

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int xf_smoothbox2_f(float *data, size_t nn, size_t halfwin, char *message) {

	char *thisfunc="xf_smoothbox2_d\0";
	size_t ii,jj,kk,mm,start,stop,mid,nwin,buffindex;
	float *buffer=NULL;
	double sum,scale,mean;

	/* CALCULATE TOTAL WINDOW SIZE */
	nwin= halfwin+halfwin+1;

	/* CHECK FOR INVALID ARGUMENTS */
	if(nn<3) { sprintf(message,"%s (data length %ld must not be less than 3)",thisfunc,nn); return(-1); }
	if(nwin>nn) { sprintf(message,"%s (window size %ld exceeds data length %ld)",thisfunc,nwin,nn); return(-1); }

	/* ALLOCATE THE BUFFER */
	buffer= (float *) malloc((nwin)*sizeof(float));
	if(buffer==NULL) { sprintf(message,"%s [ERROR]: memory allocation error",thisfunc); return(-1); }

	/* CALCULATE THE SCALING FACTOR USED FOR CALCULATING THE MEAN */
	scale=1.0/(double)nwin;

	/* PRE-FILL THE FIRST HALF OF THE BUFFER AND GET THE SUM : MM IS THE BUFFER-COUNT */
	mm=0;
	sum=0.00;
	for(ii=0;ii<halfwin;ii++) {
		sum+=data[ii];
		buffer[mm]=data[ii];
		mm++;
	}

	/* SMOOTH THE FIRST HALF-WINDOW USING THE INCREASING BUFFER-COUNT (MM) */
	mid=0;
	for(ii=halfwin;ii<nwin;ii++) {
		sum+= data[ii];
		buffer[mm]=data[ii];
		mm++;
		data[mid]=(float)(sum/(double)mm);
		mid++;
	}

	/* SMOOTH THE MAIN PORTION OF THE DATA, INDEXED BY "MID" */
	buffindex=0;
	for(ii=nwin;ii<nn;ii++) {
		sum-= buffer[buffindex];
		sum+= data[ii];
		buffer[buffindex]=data[ii];
		data[mid]= (float)(sum*scale);
		mid++;
		buffindex++;
		if(buffindex>=nwin) buffindex=0;
	}

	/* SMOOTH THE END OF THE DATA USING A DECLINING BUFFER-COUNT (MM) */
	mm=nwin;
	for(ii=mid;ii<nn;ii++) {
		sum-= buffer[buffindex];
		mm--;
		data[ii]= (float)(sum/(double)mm);
		buffindex++;
		if(buffindex>=nwin) buffindex=0;
	}


	/* WRAP-UP */
	sprintf(message,"%s:  calculated boxcar average for %ld data points, window size %ld)",thisfunc,nn,nwin);
	free(buffer);
	return(0);
}
