/* LICENCE INFORMATION:
Copyright (c) 2005, John Huxter, j.r.huxter@gmail.com.

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE COPYRIGHT OWNERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE COPYRIGHT OWNERS BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
<TAGS>matrix</TAGS>

DESCRIPTION:
	- define a peak-zone of contiguous values exceeding a threshold in a 2D matrix
	- peak propogates out in an expanding radius from the highest un-masked element exceeding the threshold
	- propogation diagonally is prevented
	- simplified version of the old hux_findspot function

USES:
	- define a hippocampal place field
	- find a bright spot in a video frame
	- define a peak in a phase-amplitude coupling plot

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data1  : input array holding a matrix of values in which peak zone is to be found
	int *mask      : input/output array initialized by calling function and used to report peak detection
	long width     : width of the matrix
	long height    : height of the matrix
	float thresh   : minimum threshold to be exceeded for inclusion in the peak zone
	double *result : array initialized by calling function to hold results

	Regarding the mask array:
		- presumed to be zero at start
		- set to 1 on addition to the peak zone
		- if initially less than zero, that element in the data is ignored
			- allows successive calls to the funtion
			- set previously detected mask elements to -1 to allow detection of other peaks

	Regarding the results array:
		result[0]= number of elements in peak
		result[1]= x-position (zero-offset element) of max value
		result[2]= y-position (zero-offset element) of max value
		result[3]= x-position of centre of mass (left=0)
		result[4]= x-position of centre of mass (top=0)
		result[5]= mean value of elements inside peak zone
		result[6]= max value (value in the pixel which is the field kernal)

RETURN VALUE:
	1 on success, 0 on error

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_matrixpeak1_d(double *data1, int *mask, long width, long height, double thresh, double *result) {

	long ii,jj,kk,nn,npeak,range,radius,peakx,peaky;
	long x1,x2,y1,y2,p1,p2,pixelsadded;
	double max,centroid_x,centroid_y,peaksum,peakmean;

	/********************************************************************************/
	/* FIND LARGEST VALUE EXCEEDING THE THRESHOLD */
	/********************************************************************************/
	nn= width*height;
	max= thresh;
	npeak=0;
	for(ii=0;ii<nn;ii++) {
		if(isfinite(data1[ii])) {
			if(mask[ii]>=0) {
				if(data1[ii]>max) {
					p1= ii;
					max= data1[ii];
					npeak= 1;
			}}
		}
		else mask[ii]=-1;
	}

	/* make sure at least 1 pixel exceeding threshold and not previously masked was detected */
	if(npeak==1) {
		peaky= p1/width;
		peakx= p1-(peaky*width);
		mask[p1]=1; // so this pixel is skipped in subsequent searches
	}
	else {
		result[0]=0.0;
		for(ii=1;ii<7;ii++) result[ii]=NAN;
		return(0);
	}


	/********************************************************************************/
	/********************************************************************************
	DEFINE THE PEAK-AREA
	********************************************************************************/
	/********************************************************************************/
	/* set limits for radius of pixel search - this will be much larger than a realistic spot */
	if(width>height) range = width;
	else range = height;

	/* now work out in a widening radius */
	for(radius=1;radius<range;radius++)	{

		pixelsadded=0;

		/********************************************************************************/
		/* DO THE SIDES ************************************************************/
		/********************************************************************************/

		/* set theoretical ranges for scans of top & bottom row */
		x1=peakx-radius;
		x2=peakx+radius;
		y1=peaky-radius;
		y2=peaky+radius;
		/* make sure x stays within range - note that for horizontal scan x2 is a proxy for width */
		if(x1<0) x1=0;
		if(x2>width) x2=width;

		/* do top row (minus corners) */
		if(y1>=0) {
			p1= y1*width + x1 + 1;
			p2= y1*width + x2;
			for(ii=p1;ii<p2;ii++) { /* note that for horizontal scan, p2 must not be reached */
				if(data1[ii]>=thresh && mask[ii]>=0) {
					// look left and down for previous non-diagonal pixels that were added
					if(mask[ii-1]==1 || mask[ii+width]==1) {
						mask[ii]=1;
						pixelsadded++;
		}}}}

		/* do bottom row (minus corners) - note that for horizontal scan, p2 represents maximum which must not be exceeded */
		if(y2<height) {
			p1= y2*width + x1 + 1;
			p2= y2*width + x2;
			for(ii=p1;ii<p2;ii++) { /* note that for horizontal scan, p2 must not be reached */
				if(data1[ii]>=thresh && mask[ii]>=0) {
					// look left and up for previous non-diagonal pixels that were added
					if(mask[ii-1]==1 || mask[ii-width]==1) {
						mask[ii]=1;
						pixelsadded++;
		}}}}

		/* reset x1 and x2 to hypothetical values */
		x1=peakx-radius;
		x2=peakx+radius;

		/* make sure y stays within range - note that here y2 represents the last row which may be used */
		if(y1<0) y1=0;
		if(y2>height) y2=height;

		/* do left side (minus corners) */
		if(x1>=0) {
			p1= (y1+1)*width + x1;
			p2= (y2*width) + x1;
			for(ii=p1;ii<p2;ii+=width) { /* note that for vertical scan, p2 is a valid position which can be reached */
				if(data1[ii]>=thresh && mask[ii]>=0) {
					// look up and right for previous non-diagonal pixels that were added
					if(mask[ii-height]==1 || mask[ii+1]==1) {
						mask[ii]=1;
						pixelsadded++;
		}}}}
		/* do right side (minus corners) */
		if(x2<width) {
			p1= (y1+1)*width + x2;
			p2= (y2*width) + x2;
			for(ii=p1;ii<p2;ii+=width) {
				if(data1[ii]>=thresh && mask[ii]>=0) {
					// look up and left for previous non-diagonal pixels that were added
					if(mask[ii-height]==1 || mask[ii-1]==1) {
						mask[ii]=1;
						pixelsadded++;
		}}}}

		/********************************************************************************/
		/* DO THE CORNERS ***************************************************************/
		/********************************************************************************/

		/* reset the theoretical ranges */
		x1=peakx-radius;
		x2=peakx+radius;
		y1=peaky-radius;
		y2=peaky+radius;

		/* make sure x and y stay within range */
		if(x1<0) x1=0;
		if(x2>=width) x2=width-1;
		if(y1<0) y1=0;
		if(y2>=height) y2=height-1;

		if(y1>=0 && x1>=0) {
			/* do top left corner */
			ii= y1*width+x1;
 			if(data1[ii]>=thresh && mask[ii]>=0) {
				// look down and right for previous non-diagonal pixels that were added
				if(mask[ii+height]==1 || mask[ii+1]==1) {
					mask[ii]=1;
					pixelsadded++;
		}}}
		if(y1>=0 && x2<width) {
			/* do top right corner */
			ii= y1*width+x2;
 			if(data1[ii]>=thresh && mask[ii]>=0) {
				// look down and left for previous non-diagonal pixels that were added
				if(mask[ii+height]==1 || mask[ii-1]==1) { //??? change right term to +1
					mask[ii]=1;
					pixelsadded++;
		}}}
		if(y2<height && x1>=0) {
			/* do bottom left corner */
			ii= y2*width+x1;
 			if(data1[ii]>=thresh && mask[ii]>=0) {
				// look up and right for previous non-diagonal pixels that were added
				if(mask[ii-height]==1 || mask[ii+1]==1) {
					mask[ii]=1;
					pixelsadded++;
		}}}
		if(y2<height && x2<width) {
			/* do bottom right corner */
			ii= y2*width+x2;
 			if(data1[ii]>=thresh && mask[ii]>=0) {
				// look up and left for previous non-diagonal pixels that were added
				if(mask[ii-height]==1 || mask[ii-1]==1) {
					mask[ii]=1;
					pixelsadded++;
			}}
		}

		/* if pixels were added in this radius, add to pixels-in-peak, otherwise, end detection */
		if(pixelsadded>0) npeak+=pixelsadded;
		else break;

	} /* END OF RADIUS LOOP */


	/********************************************************************************/
	/* GET PEAK STATISTICS - note no need to check npeak>0 - already did this before the radius loop */
	/********************************************************************************/
	centroid_x = centroid_y = peaksum = 0.0;
	for(y1=0;y1<height;y1++) {
		for(x1=0;x1<width;x1++)	{
			p1=y1*width+x1;
			if(mask[p1]==1) {
				centroid_x += x1*data1[p1];
				centroid_y += y1*data1[p1];
				peaksum += data1[p1];
	}}}
	centroid_x /= peaksum;
	centroid_y /= peaksum;
	peakmean = peaksum/(double)npeak;


	/********************************************************************************/
	/* COPY VALUES TO RESULTS AND RETURN FLAG*/
	/********************************************************************************/
	result[0]=(double)npeak;
	result[1]=(double)peakx;
	result[2]=(double)peaky;
	result[3]=centroid_x;
	result[4]=centroid_y;
	result[5]=peakmean;
	result[6]=max;
	return(1);
}
