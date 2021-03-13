/*
<TAGS>signal_processing transform</TAGS>
DESCRIPTION:
	- reduce data to averaged bins of a fixed size
	- non-finite values are impossible for short-integer input
	- allows for different numbers of elements to go into different bins to evenly spread the results (fractional bin-widths)
	- allows definition of a "zero" sample which is guaranteed to be the first sample in the bin corresponding with the new "zero"
	- guarantees no bins will be under-sampled - edge bins will contain between 1x and just-under-2x bindwidth samples
		- exception - a single partial-bin just before zero can be included, as data around zero is usually too important to exclude

USES:
	Downsampling data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	short *data       : pointer to input array, which will be overwritten
	long *setn        : number of elements in data (overwritten - pass as address)
	long *setz        : element to treat as "zero" (overwritten - pass as address)
 	double setbinsize : desired bin-width (samples - can be a fraction)
	char *message     : array to hold error message

RETURN VALUE:
	- status (0=success, -1=fail)
	- setz and setn are also updated
		- can be used to reconstruct timestamps
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int xf_bin1b_s(short *data, long *setn, long *setz, double setbinsize, char *message) {

	char *thisfunc="xf_bin1b_s\0";
	long ii,jj,n1,n2=0,zero,sum=0,nsums=0,start;
	double aa,bb,cc,prebins,limit;

	n1=*setn;
	zero=*setz;
	//TEST: fprintf(stderr,"n1=%ld\tzero=%ld\tsetbinsize=%.9f\n",n1,zero,setbinsize);


	/* CHECK PARAMETERS */
	if(n1<1) {
		sprintf(message,"%s [ERROR]: number of samples (%ld) must be >0",thisfunc,n1);
		return(-1);
	}
	if(setbinsize==1.0) {
		return(n1);
	}
	if(setbinsize<=0) {
		sprintf(message,"%s [ERROR]: bin size (%g) must be >0",thisfunc,setbinsize);
		return(-1);
	}
	if(zero>=n1) {
		sprintf(message,"%s [ERROR]: specified zero-sample (%ld) must be less than data array length (%ld)",thisfunc,zero,n1);
		return(-1);
	}
	//TEST: for(ii=0;ii<n1;ii++) printf("%g\n",data[ii]); exit(1);

	/* IF "ZERO" IS SET, CALCULATE THE NUMBER OF BINS BEFORE "ZERO" (PREBINS) */
	/* note that if prebins is not an integer, a portion will be combined with another bin */
	if(zero>0) prebins=(double)(zero)/setbinsize;
	else prebins=0.0;

	/* PRE-BIN AND SET START FOR MAIN BINNING SECTION */
	/* if prebins is zero or an integer, we can start from sample-zero with no special measures */
	if(fmod(prebins,1)==0.0) {
		start= 0;
		limit= setbinsize - 1.0;
	}
	/* otherwise, build a fractional bin and proceed from the first full-bin */
	else {
		// define limits for first bin which will include the partial bin + 1 full bin
		limit= (zero-1) - (long)((prebins-1.0)*setbinsize);
		if(limit>=zero) limit= zero-1;
		// build the bin
		for(ii=0;ii<=limit;ii++) { sum+= data[ii]; nsums++;}
		if(nsums>0) data[n2]= (short)(sum/nsums);
		else data[n2]=0;
		n2++;
		// set parameters for main loop
		start= (long)limit+1;
		limit+= setbinsize;
	}
	//TEST: fprintf(stderr,"start: %ld	zero: %ld	setbinsize:%.4f	prebins=%g	limit:%.16f\n",start,zero,setbinsize,prebins,limit);

	/* START BINNING: LEFTOVER DATA AT THE END IS ADDED TO THE PRECEDING BIN */
	sum= 0;
	nsums= 0;
	for(ii=start;ii<n1;ii++) {
		/* build runing sum and total data-points */
		sum+= data[ii]; nsums++;
		// if the current sample-number is >= the limit defining the right edge of the curent window...
		if(ii>=limit) {
			//TEST:	printf("\tii=%ld	bin=%ld nsums=%ld	limits: %f to %f: next=%f\n",ii,n2,nsums,(limit-setbinsize),(limit),(limit+setbinsize));
			data[n2]= (short)(sum/nsums);
			n2++;
			sum=0;   // reset the running sum
			nsums=0; // reset the count within the window
			limit+= setbinsize; // readjust limit
		}
	}
	//TEST: fprintf(stderr,"ii: %ld limit:%g	nsums:%ld sum:%g	\n",ii,limit,nsums,sum);

	/* MAKE ONE MORE BIN IF THERE IS LEFTOVER DATA (IE. IF LAST SAMPLE DIDN'T TIP THE LIMIT)  */
	if( ((ii-1)+setbinsize) != limit ) {
		jj= n1-(long)setbinsize;
		if(jj<zero) jj=zero; // cannot integrate data from before zero!
		sum=0; nsums=0;
		for(ii=jj;ii<n1;ii++) {
			sum+= data[ii];
			nsums++;
		}
		if(nsums>=0) data[n2]= (short)(sum/nsums);
		else data[n2]=0;
		n2++;
	}

	/* REASSIGN SETZ AND N */
	(*setz)= prebins;
	(*setn)= n2;
	return(0);
}
