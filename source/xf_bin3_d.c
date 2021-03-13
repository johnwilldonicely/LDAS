/*
<TAGS>signal_processing transform</TAGS>
DESCRIPTION:
	12.March.2021 [JRH]: based on xf_bin1b_d
	- fixed-bin-size averaging which doesn't destroy the position of the data in the original array
	- a parallel flag array indicates which elements represent the new data
	- INF & NAN do not contribute to averages
	- fractional bin-sizes allowed (true bin-widths vary to evenly span data)
	- definition of "zero" ensures division of data either side of "zero"

	- partial-bins at the beginning:
	 	- will be incorporated into subsequent full-bin
		- will not use data from "zero" or after
	- partial bins at the end:
	 	- form a new bin using some data from the preceeding bin
		- will not use data from before "zero"

USES:
	- Downsampling data
	- Allows re-combination of binned data with columns of data corresponding with the original data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data      : pointer to input array, which will be overwritten
	short *flag       : pointer to flag (0-1) array - will identify elements which have been binned
	long n1           : number of elements in data array
	long setz         : zero-offset element-number representing "zero"- setz itself will always be the beginning of a bin
 	double binsize : desired bin-width (samples - can be a fraction)
	char *message     : array to hold error message

RETURN VALUE:
	- Failure: -1
	- Success: number of bins produced
		- data will be modified so some elements hold averages
		- flag will be modified to "1" for elements which hold the averages, and "0" for unchanged values

SAMPLE CALL:
	# data1 = (double) data array of size n1
	char message[256]
	short *flag1= calloc(n1,sizeof(*flag1));
	long ii,jj,zero= 5;
	double binsize=2.0;

	jj= xf_bin3_d(data1,flag1,n1,zero,binsize,message);
	if(jj<0) {fprintf(stderr,"*** %s\n",message); exit(1);}

	for(ii=0;ii<n1;ii++) printf("data1[%ld]=%g ... %d\n",ii,data2[ii],flag1[ii]);
	for(ii=0;ii<n1;ii++) if(flag1[ii]==1) printf("%ld\t%g\n",(ii-z1),data2[ii]);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long xf_bin3_d(double *data1, short *flag1, long n1, long setz, double binsize, char *message) {

	char *thisfunc="xf_bin3_d\0";
	long ii,jj,nbins=0,nsums=0,start,binbegin=0;
	double aa,bb,cc,prebins=-1.0,limit=-1.0,sum=0.0;

	/* CHECK PARAMETERS */
	if(n1<1) { sprintf(message,"%s [ERROR]: number of samples (%ld) must be >0",thisfunc,n1); return(-1); }
	if(binsize==1.0) {
		for(ii=0;ii<n1;ii++) flag1[ii]= 1;
		return(n1);
	}
	if(binsize<=0) {sprintf(message,"%s [ERROR]: bin size (%g) must be >0",thisfunc,binsize);	return(-1);}
	if(setz>=n1) {sprintf(message,"%s [ERROR]: specified zero-sample (%ld) must be less than data array length (%ld)",thisfunc,setz,n1); return(-1);}
	//TEST: for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]); exit(1);

	/* INITIALISE FLAGS */
	for(ii=0;ii<n1;ii++) flag1[ii]= 0;

	/* IF "ZERO" IS SET, CALCULATE THE NUMBER OF BINS BEFORE "ZERO" (PREBINS) */
	/* note that if prebins is not an integer, a portion will be combined with another bin */
	if(setz>0) prebins=(double)(setz)/binsize;
	else prebins=0.0;


	/**********************************************************************/
	/**********************************************************************/
	/* PRE-BIN AND SET START FOR MAIN BINNING SECTION */
	/**********************************************************************/
	/**********************************************************************/
	/* if prebins is zero or an integer, we can start from sample-zero with no special measures */
	if(fmod(prebins,1)==0.0) {
		start= 0;
		binbegin= start;
		limit= binsize - 1.0;
	}
	/* otherwise, build a fractional bin and proceed from the first full-bin */
	else {
		// define limits for first bin which will include the partial bin + 1 full bin
		limit= ((double)(setz)-1.0) - ((long)(prebins-1.0)*binsize);
		if(limit>=setz) limit= setz-1;
		// set the theoretical beginning of this first bin - excluding the partial bin
		binbegin= (long)(limit-binsize);
		if(binbegin<0) binbegin=0;
		//TEST:fprintf(stderr,"\nzero: %ld\nbinsize:%.4f\nprebins=%g\nstart: %ld\nlimit:%.4f\n",setz,binsize,prebins,start,limit);
		// build the bin
		for(ii=0;ii<=limit;ii++) if(isfinite(data1[ii])) { sum+= data1[ii]; nsums++;}
		if(nsums>0) data1[binbegin]= (sum/(double)nsums);
		else data1[binbegin]=NAN;
		flag1[binbegin]= 1;
		nbins++;
		// set parameters for main loop
		start= (long)limit+1;
		binbegin= start;
		limit+= binsize;
	}
	//TEST:	fprintf(stderr,"\nzero: %ld\nbinsize:%.4f\nprebins=%g\nstart: %ld\nlimit:%.4f\n",setz,binsize,prebins,start,limit);


	/**********************************************************************/
	/**********************************************************************/
	/* START BINNING: LEFTOVER DATA AT THE END IS ADDED TO THE PRECEDING BIN */
	/***********************************************************************/
	/**********************************************************************/
	sum= 0.0;
	nsums= 0;
	for(ii=start;ii<n1;ii++) {
		/* build runing sum and total data-points - good data only */
		if(isfinite(data1[ii])) {
			sum+= data1[ii];
			nsums++;
		}
		// if the current sample-number is >= the limit defining the right edge of the curent window...
		if(ii>=limit) {
			//TEST: printf("ii=%ld data=%g nsums=%ld sum=%g nbins=%ld   binbegin=%ld\n",ii,data1[ii],nsums,sum,nbins,binbegin);
			if(nsums>0) data1[binbegin]= (sum/(double)nsums);
			else data1[binbegin]=NAN;
			flag1[binbegin]=1; // flag the sample for the beginning of this bin
			binbegin= ii+1; // set the next sample as the beginning for the next bin
			nbins++;
			sum= 0.0; // reset the run1ing sum
			nsums= 0; // reset the count within the window
			limit+= binsize; // readjust limit
		}
	}
	//TEST: fprintf(stderr,"ii: %ld limit:%g	nsums:%ld sum:%g	\n",ii,limit,nsums,sum);

	/* MAKE ONE MORE BIN IF THERE IS LEFTOVER DATA (IE. IF LAST SAMPLE DIDN'T TIP THE LIMIT)  */
	/* add sufficient data from preceeding bin to make a full complement */
	if( ((ii-1)+binsize) != limit ) {
		jj= n1-(long)binsize;
		if(jj<setz) jj=setz; // cannot integrate data from before zero!
		sum=0.0; nsums=0;
		for(ii=jj;ii<n1;ii++) { if(isfinite(data1[ii])) { sum+= data1[ii]; nsums++; }}
		if(nsums>=0) data1[binbegin]= sum/(double)nsums;
		else data1[binbegin]=NAN;
		flag1[binbegin]=1; // flag the sample for the beginning of this bin
		nbins++;
	}

	/* RETURN THE NUMBER OF BINS GENERATED */
	return(nbins);
}
