/*
<TAGS>signal_processing transform</TAGS>
DESCRIPTION:
	12.March.2021 [JRH]: based on xf_bin1b_d
	- fixed-bin-size averaging which doesn't destroy the position of the data in the original array
	- a parallel flag array indicates which elements represent the new data
	- INF & NAN do not contribute to averages
	- fractional bin-sizes allowed (true bin-widths vary to evenly span data)
	- definition of "zero" ensures no binning across zero itself
	- partial-bins at the beginning of the data (except just before zero) are absorbed into adjacent full-bins
	- partial bins at the end of the data form a new bin, but use some data from the preceeding full-bin as well

USES:
	- Downsampling data
	- Allows re-combination of binned data with columns of data corresponding with the original data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data      : pointer to input array, which will be overwritten
	short *flag       : pointer to flag (0-1) array - will identify elements which have been binned
	long setn         : number of elements in data
	long setz         : element to treat as "zero"
 	double setbinsize : desired bin-width (samples - can be a fraction)
	char *message     : array to hold error message

RETURN VALUE:
	- Failure: -1
	- Success: number of bins produced
		- data will be modified so some elements hold averages
		- flag will be modified to "1" for elements which hold the averages, and "0" for unchanged values

SAMPLE CALL:
	z1= 5; // define the "zero" sample
	binsize= 2.0; // define size of bins (samples)

	jj= xf_bin3_d(data1,flag1,n1,z1,2.0,message);
	if(jj<0) {fprintf(stderr,"*** %s\n",message); exit(1);}

	for(ii=0;ii<n1;ii++) printf("data1[%ld]=%g ... %d\n",ii,data2[ii],flag1[ii]);
	for(ii=0;ii<n1;ii++) if(flag1[ii]==1) printf("%ld\t%g\n",(ii-z1),data2[ii]);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long xf_bin3_d(double *data1, short *flag1, long n1, long zero, double setbinsize, char *message) {

	char *thisfunc="xf_bin3_d\0";
	long ii,jj,n2,nbins,nsums,start,binbegin=0;
	double aa,bb,cc,prebins,limit,sum;

	/* CHECK PARAMETERS */
	if(n1<1) { sprintf(message,"%s [ERROR]: number of samples (%ld) must be >0",thisfunc,n1); return(-1); }
	if(setbinsize==1.0) {return(n1);}
	if(setbinsize<=0) {sprintf(message,"%s [ERROR]: bin size (%g) must be >0",thisfunc,setbinsize);	return(-1);}
	if(zero>=n1) {sprintf(message,"%s [ERROR]: specified zero-sample (%ld) must be less than data array length (%ld)",thisfunc,zero,n1); return(-1);}
	//TEST: for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]); exit(1);

	/* IF "ZERO" IS SET, CALCULATE THE NUMBER OF BINS BEFORE "ZERO" (PREBINS) */
	/* note that if prebins is not an integer, a portion will be combined with another bin */
	if(zero>0) prebins=(double)(zero)/setbinsize;
	else prebins=0.0;
	/* INITIALISE VARIABLES HERE */
	sum= 0.0;
	n2=nsums= 0;
	for(ii=0;ii<n1;ii++) flag1[ii]= 0;
	/* CALCULATE THE LIMIT FOR THE FIRST BIN - MAY BE FRACTIONAL */
	/* if there is at least one full bin before "zero", the first limit comes before "zero" as well */
	if(prebins>=1.0) {
		limit= ((double)(zero)-1.0) - ((long)(prebins-1.0)*setbinsize);
		start= 0;
		binbegin= (long)((limit-setbinsize) +1.0); // this avoids defining a sample from a partial-bin as the official start of the bin
		if(binbegin<0) binbegin= 0;
	}
	/* otherwise accumulate data up to zero - this is the only instance where a bin can include less than the normal amount of data  */
	else {
		limit= (double)zero + setbinsize - 1.0;
		start= zero;
		for(ii=0;ii<zero;ii++) if(isfinite(data1[ii])) { sum+= data1[ii]; nsums++;}
		if(nsums>0) data1[0]= (sum/(double)nsums);
		else data1[0]=NAN;
		flag1[0]= 1; // flag the first sample before zero as the beginning of this bin
		sum= 0.0;
		nsums= 0;
		prebins=1.0; // indicates a part-bin was created - this ensures new zero is element "1"
		n2++; // increment the number of bins
		binbegin= start;
	}
	//TEST: fprintf(stderr,"start: %ld	zero: %ld	setbinsize:%.4f	prebins=%g	limit:%.16f\n",start,zero,setbinsize,prebins,limit);

	/* START BINNING: LEFTOVER DATA AT THE END IS ADDED TO THE PRECEDING BIN */
	for(ii=start;ii<n1;ii++) {
		/* build runing sum and total data-points - good data only */
		if(isfinite(data1[ii])) {
			sum+= data1[ii];
			nsums++;
		}
		// if the current sample-number is >= the limit defining the right edge of the curent window...
		if(ii>=limit) {
			//TEST: printf("ii=%ld data=%g nsums=%ld sum=%g n2=%ld   binbegin=%ld\n",ii,data1[ii],nsums,sum,n2,binbegin);
			if(nsums>0) data1[binbegin]= (sum/(double)nsums);
			else data1[binbegin]=NAN;
			flag1[binbegin]=1; // flag the sample for the beginning of this bin
			binbegin= ii+1; // set the next sample as the beginning for the next bin
			n2++;
			sum= 0.0; // reset the run1ing sum
			nsums= 0; // reset the count within the window
			limit+= setbinsize; // readjust limit
		}
	}
	//TEST: fprintf(stderr,"ii: %ld limit:%g	nsums:%ld sum:%g	\n",ii,limit,nsums,sum);

	/* MAKE ONE MORE BIN IF THERE IS LEFTOVER DATA (IE. IF LAST SAMPLE DIDN'T TIP THE LIMIT)  */
	/* add sufficient data from preceeding bin to make a full complement */
	if( ((ii-1)+setbinsize) != limit ) {
		jj= n1-(long)setbinsize; if(jj<0) jj=0;
		sum=0.0; nsums=0;
		for(ii=jj;ii<n1;ii++) { if(isfinite(data1[ii])) { sum+= data1[ii]; nsums++; }}
		if(nsums>=0) data1[binbegin]= sum/(double)nsums;
		else data1[binbegin]=NAN;
		flag1[binbegin]=1; // flag the sample for the beginning of this bin
		n2++;
	}

	/* RETURN THE NUMBER OF BINS GENERATED */
	return(n2);
}
