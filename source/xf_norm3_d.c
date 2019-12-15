/*
<TAGS>stats</TAGS>
DESCRIPTION:
	Normalize an array of double-precision floating-point numbers
	Data can be normalized as follows:
		NORMALIZE TYPE-0: CONVERT TO A RANGE FROM 0 TO 1
		NORMALIZE TYPE-1: CONVERT TO Z-SCORES (use start-stop)
		NORMALIZE TYPE-2: difference from first valid sample (beginning at start)
		NORMALIZE TYPE-3: difference from mean (use start-stop)
		NORMALIZE TYPE-4: ratio of mean (use start-stop)

	- to use like previous versions of xf_norm, jj= xf_norm3_d(data,n,type,0,-1,message)

USES:
	Can make it easier to compare skewed datasets
	Good for analyzing trends in data with very different baselines

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data  : the original data - will be transformed!
	long ndata    : number of elements in the array
	int normtype  : the type of normalization (see above)
	long start    : first sample (0 to [ndata-1]) to use for normalization (-1= first valid sample)
	long stop     : last sample+1 (1 to ndata) to use for normalization (-1= last valid sample)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	on success: number of valid numerical data points in data
		- the input data will be modified
	on error:
		-1 - there is no valid data, or reference-zone is invalid
			- all input becomes NAN
			- message array will hold WARNING text
		-2 if invalid parameters were passed
			- input is unaltered
			- message array will hold ERROR text

SAMPLE CALL
	type=3; start=-1; stop=-1;
	ii= xf_norm3_d(data,ndata,type,start,stop,message);
	if(ii==-2) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	if(ii==-1) {
		if(setverb>0) fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
		for(jj=0;jj<ndata;jj++) datval[jj]=NAN;
	}

*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message) {

	char *thisfunc="xf_norm3_d\0";
	int flag= 0; /* flag for return-behaviour: 0=good, 1=set-to-NAN */
	long int ii,jj,mm,firstgood=-1,lastgood=-1;
	double *tempdata=NULL;
	double aa,min,max,sum=0.0,sumofsq=0.0,norm1=0.0,norm2=1.0;

	/* DETERMINE FIRST AND LAST VALID SAMPLE IN THE DATA */
	for(ii=0;ii<ndata;ii++) { if(isfinite(data[ii])) { if(firstgood==-1) firstgood= ii; lastgood= ii; }}
	if(firstgood==-1) { sprintf(message,"%s [WARNING]: no valid data",thisfunc); return(-1); }
	if(start==-1) start= firstgood;
	if(stop==-1) stop= lastgood+1;

	/* CHECK VALIDITY OF ARGUMENTS AND FINAL VAUES FOR START & STOP */
	if(normtype<0||normtype>4) { sprintf(message,"%s [ERROR]: invalid norm-type (%d) must be 0-4",thisfunc,normtype); return(-2); }
	if(ndata==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,ndata); return(-2); }
	if(start<0) { sprintf(message,"%s [ERROR]: invalid start-sample (%ld)",thisfunc,start); return(-2); }
	if(start>=stop) { sprintf(message,"%s [ERROR]: start-sample (%ld) must be < stop-sample (%ld)",thisfunc,start,stop); return(-2); }
	if(start>=ndata) { sprintf(message,"%s [ERROR]: start-sample (%ld) must be < ndata (%ld)",thisfunc,start,ndata); return(-2); }
	if(stop>ndata) { sprintf(message,"%s [ERROR]: stop-sample (%ld) must be <= ndata (%ld)",thisfunc,stop,ndata); return(-2); }

	//TEST:	fprintf(stderr,"data[0]=%g\n",data[0]);

	/* NORMALIZE TYPE-0: CONVERT TO A RANGE FROM 0 TO 1 */
	if(normtype==0) {
		min= max= data[firstgood];
		/* define norm1 (the minimum) */
		for(ii=0;ii<ndata;ii++) { aa=data[ii]; if(isfinite(aa)) {if(aa<min)min=aa;if(aa>max)max=aa;} }
		norm1= min;
		/* define norm2 (the range) */
		if(max>min) norm2= max-min;
		else norm2=1.0;
	}
	/* NORMALIZE TYPE-1: CONVERT TO Z-SCORES - use start & stop */
	else if(normtype==1) {
		/* define norm1 (the mean) */
		mm=0;
		for(ii=start;ii<stop;ii++) { aa=data[ii]; if(isfinite(aa)) {sum+=aa; sumofsq+=aa*aa; mm++;} }
		if(mm==0) { sprintf(message,"%s [WARNING]: no valid data in reference-zone (samples %ld-%ld)",thisfunc,start,stop); flag=1; }
		else {
			norm1= sum/(double)mm;
			/* define norm2 (the standard deviation) */
			if(mm>1) norm2= sqrt((double)((mm*sumofsq-(sum*sum))/(mm*(mm-1))));
			else norm2= 1.1;
		}
	}
	/* NORMALIZE TYPE-2: difference from first valid sample, beginning at "start" */
	else if(normtype==2) {
		for(ii=start;ii<ndata;ii++) if(isfinite(data[ii])) { break; }
		if(!isfinite(data[start])) { sprintf(message,"%s [WARNING]: data at start-sample %ld is invalid",thisfunc,start); flag=1; }
		else { norm1= data[start]; norm2= 1;}
	}
	/* NORMALIZE TYPE-3: difference from mean (start-stop) */
	else if(normtype==3) {
		sum= 0.0; mm= 0;
		for(ii=start;ii<stop;ii++) if(isfinite(data[ii])) { sum+= data[ii]; mm++; }
		if(mm==0) { sprintf(message,"%s [WARNING]: no valid data in reference-zone (samples %ld-%ld)",thisfunc,start,stop); flag=1; }
		else { norm1= sum/(double)mm; norm2= 1; }
	}
	/* NORMALIZE TYPE-4: ratio of mean (start-stop) */
	else if(normtype==4) {
		sum= 0.0; mm= 0;
		for(ii=start;ii<stop;ii++) if(isfinite(data[ii])) { sum+= data[ii]; mm++; }
		if(mm==0) { sprintf(message,"%s [WARNING]: no valid data in reference-zone (samples %ld-%ld)",thisfunc,start,stop); flag=1; }
		else { norm1= 0.0; norm2= sum/(double)mm; }
	}


	/* TRANSFORM THE ARRAY - ONLY THE VALID DATA POINTS */
	if(flag==0) {
		for(ii=mm=0;ii<ndata;ii++) { if(isfinite(data[ii])) { mm++; data[ii]= (data[ii]-norm1)/norm2; } }
		/* RETURN THE NUMBER OF VALID DATA POINTS */
		return(mm);
	}
	/* otherwise, if there was no good reference data for normalization... */
	else { for(ii=mm=0;ii<ndata;ii++) data[ii]=NAN ; return(-1); }
}
