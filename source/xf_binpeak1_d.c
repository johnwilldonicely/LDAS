/*
<TAGS>signal_processing transform</TAGS>

DESCRIPTION:
	Bin an array of data using non-overlapping windows, using peak-detection
	Overwrites the original data
	Ignores NAN or INF values

USES:
	Downsample a time-series with peak-detection to avoid losing significant features

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data1  : array holding data values
	long n         : number of elements in the array
	double binsize : size (number of elements) in the binning window - note that this can be fractional

RETURN VALUE:
	new size of binned data array, or -1 on error

SAMPLE CALL:
	n= xf_binpeak1_d(data,n,100,message);
	if(n>=0) for(i=0;i<n;i++) printf("%g\n",data[i]);
	else fprintf(stderr,"%s\n",message);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long int xf_binpeak1_d(double *data1,long n, double binsize, char *message) {

	char *thisfunc="xf_binpeak1_d\0";
	int leftover_data,sizeofdouble=sizeof(double);
	long i,j,k,l,m,n1,n2,n3,n_final,imax;
	double aa,bb,limit,sum,mean,max;
	double *buffer=NULL;

	/* check arguments */
	if(binsize<1) {
		sprintf(message,"%s [ERROR]: binsize (%g) must be 1 or greater",thisfunc,binsize);
		return(-1);
	}
	if(binsize>=n) {
		sprintf(message,"%s [ERROR]: binsize (%g) must be < n (%ld)",thisfunc,binsize,n);
		return(-1);
	}


	/* INITIALIZE VARIABLES */
	n2=0; // total valid values in a given bin
	n3=0; // total "leftover" values which have not been binned at the end of reading the input
	n_final=0;
	sum=0.0; // the sum for any given bin
	limit=binsize-1.0; // the initial threshold that the sample-number must exceed to trigger output

	i= (1+(long)binsize)*sizeof(double);
	if((buffer=(double *)realloc(buffer,i))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		return(-1);
	}

	for(i=0;i<n;i++) {
		// assign datum to aa
		aa=data1[i];
		// if the value is a finite number, it contributes to the buffer contents (i.e. n2)
		if(isfinite(aa)) { sum+=aa; buffer[n2++]=aa; }
		// if the current sample is >= the limit defining the right edge of the curent window, output the mean and reset
		if((double)i>=limit) {
			if(n2>0) {
				max=0.00;
				imax=0;
				mean=sum/(double)n2;
				for(j=0;j<n2;j++) {
					bb=fabs((buffer[j]-mean));
					if(bb>max) { imax=j; max=bb; }
				}
				data1[n_final++]= buffer[imax];
			}
			else {
				data1[n_final++]= NAN;
			}
			limit+=binsize; // increase the sample-number threshold
			sum=0.0;
			n2=n3=0;
		}
		// increment leftover counter regardless of whether input is a valid number
		else n3++;
	}


	/* IF THERE WAS LEFTOVER DATA, DEAL WITH IT */
	if(n3>0) {
		if(n2>0) {
			max=0.00;
			imax=0;
			mean=sum/(double)n2;
			for(j=0;j<n2;j++) {
				bb=fabs((buffer[j]-mean));
				if(bb>max) { imax=j; max=bb; }
			}
			data1[n_final++]= buffer[imax];
		}
		else {
			data1[n_final++]= NAN;
		}
	}

	if(buffer!=NULL) free(buffer);
	return (n_final);
}
