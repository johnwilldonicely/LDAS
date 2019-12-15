/*
<TAGS>signal_processing transform</TAGS>

DESCRIPTION:
	Bin an array of ydata, and output the max deviations in each bin (window)
		- NAN values will tend not to affect min or max
		- INF values will obviously affect max!
	Overwrites the original xdata and ydata arrays, beginning at element zero

	Changes this version (xf_binpeak4)
		- outputs values at the start & midpoint of each window
		- no setmid option, since peak-detection must output 2 points for every window anyway
		- use simple min/max algorithm instead of calculating deviation from mean

USES:
	Downsample a time-series with peak-detection to avoid loosing significant features

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *xdata	: array holding xdata values
	double *ydata	: array holding ydata values
	long n			: number of elements in the xdata and ydata arrays
	double winwidth : size of the binning window in units of [xdata]

RETURN VALUE:
	Number of elements in the revised xdata & ydata arrays
	-1 if not enough memory to hold the temporary array
	-2 if the window size is too large ( total xdata range < 2*winwidth )

SAMPLE CALL:
*/

#include <stdio.h>
#include <stdlib.h>

long int xf_binpeak4(double *xdata, double *ydata , long n, double winwidth) {

	int leftover_data=0,sizeofdouble=sizeof(double);
	long i,j,k,l,m,n2=0,tot=0;
	double aa,bb,cc,dd,range,xdata_start,xdata_elapsed,sum,mean,min,max;
	double *ydata2=NULL;

	/* allocate memory for a window guaranteed to be large enough to hold the ydata */
	if((ydata2=(double *)realloc(ydata2,(n+1)*sizeofdouble))==NULL) return(-1);

	/* make sure ydata array is long enough to accomodate windowing */
	/* if window must be at least twice as long to */
	range=xdata[(n-1)]-xdata[0]; if(range<=(2*winwidth)) return -2;

	winwidth-=0.00000001; // reduce size of window slightly - ensures detection of intervals >= winwidth
	aa= xdata_start= -1.0;

	/* BIN ydata */
	for(i=0;i<n;i++) {
		aa=xdata[i];
		bb=ydata[i];
		if(i==0) xdata_start=aa;
		xdata_elapsed= aa-xdata_start;
		leftover_data=0;	// by default there is no leftover data at the end of the loop

		if(xdata_elapsed>=winwidth) {
			leftover_data=1;		// flag potential presence of leftover data

			/* calculate min & max values in this window */
			l=m=0;min=max=0.0;
			for(j=0;j<tot;j++) {
				dd=ydata2[j];
				if(dd<min){min=dd;l=j;}
				if(dd>max){max=dd;m=j;}
			}
			ydata[n2]=ydata2[l]; 	// overwrite previous ydata with min-value
			xdata[n2]=xdata_start;	// overwrite previous xdata with value at start of this window
			n2++;
			ydata[n2]=ydata2[m]; 	// overwrite previous ydata with max-value
			xdata[n2]=xdata_start; 	// overwrite previous xdata with value corresponding to start of this window
			n2++;

			xdata_start=aa;			// reset start-xdata
			tot=0;
		}

		ydata2[tot]=bb;  // build temporary array of ydata in this window
		tot++;			// number of data points in the window
	}

	/* IF THERE WAS LEFTOVER ydata, CALCULATE ONE LAST REVISED xdata & ydata PAIR */
	xdata_elapsed=aa-xdata_start;
	if(xdata_elapsed>0.0 || leftover_data==1) {

		l=m=0;min=max=0.0;
		for(j=0;j<tot;j++) {
			dd=ydata2[j];
			if(dd<min){min=dd;l=j;} if(dd>max){max=dd;m=j;}
		}

		ydata[n2]=ydata2[l];
		xdata[n2]=xdata_start;
		n2++;

		ydata[n2]=ydata2[m];
		xdata[n2]=xdata_start;
		n2++;
	}

	free(ydata2);
	return (n2);
}
