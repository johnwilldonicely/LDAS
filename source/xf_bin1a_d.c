/* LICENCE INFORMATION:
Copyright (c) 2005, John Huxter, j.r.huxter@gmail.com.
Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
THE SOFTWARE IS PROVIDED "AS IS" AND THE COPYRIGHT OWNERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE COPYRIGHT OWNERS BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN COsetnECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
<TAGS>signal_processing transform</TAGS>

DESCRIPTION:
	- reduce data to a fixed number of averaged bins
	- non-finite values (INF, NAN) do not contribute to the averages
	- allows for different numbers of elements to go into different bins to evenly spread the results (fractional bin-widths)
	- allows definition of a "zero" sample which is guaranteed to be the first sample in the bin corresponding with the new "zero"
	- guarantees no bins will be under-sampled
		- once the bin-size is determined, edge bins will contain between 1x and just-under-2x bindwidth samples

USES:
	Downsampling data

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data   : pointer to input array, which will be overwritten
	size_t *setn   : number of elements in data (overwritten - pass as address)
	size_t *setz   : element to treat as "zero" (overwritten - pass as address)
 	size_t setbins : desired length of data after binning (number of bins used for averaging)
	char *message  : array to hold error message

RETURN VALUE:
	- size of bins used for averaging or 0 on fail
		- setz and setn are also updated
		- can be used to reconstruct timestamps


SAMPLE CALL:
	char message[256];
	double data[19];
	double aa,bb,binsize,sampinterval=1;
	size_t ii,setn=19, setz=6;
	for(ii=0;ii<setn;ii++) { data[ii]=(double)ii; printf("%g\n",data[ii]); }
	printf("zero=item number %ld\n",setz);
	printf("\n");

	binsize= xf_bin1a_f(data,&setn,&setz,3,message);
	if(binsize<0) {fprintf(stderr,"*** %s\n",message); exit(1);}

	aa=(double)(setz)*(-1)*binsize*sampinterval;
	bb=binsize*sampinterval;
	for(ii=0;ii<setn;ii++)  { printf("%g\t%f\n",aa,data[ii]);	aa+=bb; }
	printf("new zero=item number %ld\n",setz);

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message) {

	char *thisfunc="xf_bin1a_d\0";
	size_t ii,jj,mm,n1,n2,zero,nsums,prevnsums,start;
	double f1,f2,f4,f0; // variables to indicate optimal binsizes
	double aa,bb,cc,dd,n1d,binsize,prebins,postbins,limit;
	double sum,prevsum;

	n1=*setn;
	zero=*setz;

	/* CHECK PARAMETERS */
	if(setbins<1||n1<1) {
		sprintf(message,"%s [ERROR]: number of samples (n1 %ld) and number bins (setbins %ld) must be >0",thisfunc,n1,setbins);
		return(0.0);
	}
	if(setbins==n1) {
		return(1.0);
	}
	if(setbins>n1) {
		sprintf(message,"%s [ERROR]: number of bins (%ld) must be no more than the data length (%ld)",thisfunc,setbins,n1);
		return(0.0);
	}
	if(zero>=n1) {
		sprintf(message,"%s [ERROR]: specified zero-sample (%ld) must be less than data length (%ld)",thisfunc,zero,n1);
		return(0.0);
	}

	/********************************************************************************
	DETERMINE BIN SIZE
	- function makes an iterative attempt at finding the best binsize
	- must produce the right number of bins
	- whole half or quarter-integer preferred
	********************************************************************************/
	//TEST: fprintf(stderr,"n1=%ld	zero=%ld\n",n1,zero);
	n1d=(double)n1;
	aa=(double)(n1+1);
	bb=(double)(setbins);
	cc=(double)(zero);
	f1=f2=f4=-1.0;
	while(aa-->bb) {
		binsize=aa/bb;
		mm= (size_t)(cc/binsize) + (size_t)((n1d-cc)/binsize); // sum of whole-bins before and after zero
		if(mm==setbins) {
			dd=binsize; if(dd==(size_t)dd) f1=binsize;     // a whole-integer binsize
			dd=2.0*binsize; if(dd==(size_t)dd) f2=binsize; // a half-integer binsize
			dd=4.0*binsize; if(dd==(size_t)dd) f4=binsize; // a quarter-integer binsize
			f0=binsize;                                    // at least we got the right number of bins!
		}
		else if(mm>setbins) break;
	}
	/* chose the best binsize based on whether a whole-half or quarter-integer size meeting setbin requirements was found */
	if(f1>0) binsize=f1;
	else if(f2>0) binsize=f2;
	else if(f4>0) binsize=f4;
	else if(f0>0) binsize=f0;
	else binsize= n1d/bb; // if all else fails, this will be close



	/* IF "ZERO" IS SET, CALCULATE THE NUMBER OF BINS BEFORE "ZERO" (PREBINS) */
	/* note that if prebins is not an integer, a portion will be combined with another bin */
	if(zero==0) prebins=0.0;
	else prebins=(double)(zero)/binsize;


	/* CALCULATE THE LIMIT FOR THE FIRST BIN - MAY BE FRACTIONAL */
	/* if there is at least one full bin before "zero", the first limit comes before "zero" as well */
	/* otherwise accumulate data up to "binsize" samples, excluding the fractional (or non-existent) pre-zero bin */
	if(prebins>=1.0) {
		limit= (double)(zero) - ( (size_t)(prebins-1.0) * binsize );
		start=0;
	}
	else {
		start=zero;
		limit=zero+binsize;
	}
	//TEST: fprintf(stderr,"start: %ld	zero: %ld	binsize:%g	limit:%g\n",start,zero,binsize,limit);

	/* START BINNING: LEFTOVER DATA AT THE END IS ADDED TO THE PRECEDING BIN */
	sum=prevsum=0.0;
	n2=nsums=prevnsums=0;
	for(ii=start;ii<n1;ii++) {
		if(ii>=limit) {
			//TEST:	printf("\tii=%ld	bin=%ld nsums=%ld	limits: %f to %f: next=%f\n",ii,n2,nsums,(limit-binsize),(limit),(limit+binsize));
			if(nsums>0) { data[n2]= sum/nsums;}
			else { data[n2]=NAN; }
			prevsum=sum; 			// keep record of current sum in case we need to add data at the end
			prevnsums=nsums; 		// keep record of current nsums in case we need to add data at the end
			sum=0.0; 				// reset the run1ing sum
			nsums=0;  				// reset the count within the window
			limit+= binsize; 		// readjust limit
			n2++;
		}
		/* build run1ing sum and total data-points */
		if(isfinite(data[ii])) { sum+=data[ii];	nsums++; }
	}

	/* if the last data-point doesn't tip over the bin-limit, data is added to the previous bin */
	if(ii<limit) { n2--; sum+=prevsum; nsums+=prevnsums;}

	/* AVERAGE THE REMAINING DATA FROM THE LAST BIN */
	if(nsums>=0) data[n2++]= sum/nsums;
	else data[n2++]=NAN;

	/* REASSIGN SETZ AND N */
	(*setz)= prebins;
	(*setn)= n2;

	return(binsize);
}
