#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-detectcycles2"
#define TITLE_STRING thisprog" v 7: 7.April.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>

v 7: 7.April.2015 [JRH]
	- use new version of xf_bin1a_f - guarantees no under-sampling in edge bins

v 6: 6.May.2014 [JRH]
	- add normalized amplitude output for each cycle

v 5: 7.April.2014 [JRH]
	- bugfix: reading sample-frequency argument was using old "i" variable instead of "ii"

v 4: 10.February.2014 [JRH]
	- bugfix: allow setting of sample-frequency - previously was pre-set to 1000Hz
v 3: 26.January.2014 [JRH]
	- use new xf_bin1a_f function for outputting "average cycle"

v 2: 24.January.2014 [JRH]
	- add calculation of cycle midpoint (peak)

v 1: 19.January.2014 [JRH]
	- use simplified binary file read function xf_readbin2_f ,which handles file-opening and conversion to float
*/

/* external functions start */
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
long xf_interp3_f(float *data, long ndata);
float *xf_padarray2_f(float *data, size_t nn, size_t npad, int type, char *message);
int xf_filter_bworth1_f(float *data, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_detectcycles2_f(float *data, size_t ndat, size_t cmin, size_t cmax, size_t **cstart, size_t **cpeak, size_t **cstop, size_t *ctot, char *message);
double xf_bin1a_f(float *data, size_t *setn, size_t *setz, size_t setnbins, char *message);
int xf_stats2_f(float *data, long n, int varcalc, float *result_f);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN];
	long i,j,k;
	size_t ii,jj,kk,nn,mm;
	int v,w,x,y,z,col,colmatch,status=0;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	size_t *cstart=NULL,*cpeak=NULL,*cstop=NULL,n2,*nsum;
	size_t cmin,cmax,tempmax,cycletot=0,npad=0;
	float *data1=NULL,*camp=NULL,*tempdata;
	off_t datasize,startbyte,bytestoread;
	double *sum;
	/* arguments */
	int setdatatype=3,setmethod=2,setout=1,setverb=0,setnorm=-1;
	float setfmin=4,setfmax=12,setsampfreq=1000.0,setresonance=1.0,setinvalid=NAN;
	off_t setheaderbytes=0,setstart=0,setntoread=0,parameters[5];

	/* set default exit status to 1 - unless program naturally completes all operations, this means exiting with an error */
	status=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Detect cycles in a given frequency range in a BINARY file\n");
		fprintf(stderr,"Non-numeric values will be interpolated\n");
		fprintf(stderr,"Includes padding and filtering function\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name:  Format= 2 columns: <time> <data>\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data [%d]\n",setdatatype);
		fprintf(stderr,"		0-9= uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-h: size of header (bytes) excluded from output [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-s: start reading at this element (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of elements to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"		NOTE: -s and -n will be internally converted to bytes\n");
		fprintf(stderr,"	-low: minimum frequency to detect (-1 = no lower bound) [%g]\n",setfmin);
		fprintf(stderr,"	-high: maximum frequency to detect (-1 = no upper bound) [%g]\n",setfmax);
		fprintf(stderr,"	-res: filter resonance (0.1 to sqrt(2)=1.4142) [%g]\n",setresonance);
		fprintf(stderr,"		NOTE: low values can produce ringing in the output\n");
		fprintf(stderr,"		NOTE: high values can dampen the signal\n");
		fprintf(stderr,"	-o: output [%d]: \n",setout);
		fprintf(stderr,"		1=cycle start peak stop and amplitude\n");
		fprintf(stderr,"			NOTE: amplitude= peak - ((start+stop)/2)\n");
		fprintf(stderr,"		2=filtered data\n");
		fprintf(stderr,"		3=average waveform of cycles, normalized to the width of the shortest cycle\n");
		fprintf(stderr,"	-z: amplitude normalization (output style 1 only) [%d]: \n",setnorm);
		fprintf(stderr,"		-1= none\n");
		fprintf(stderr,"		 0= proportion of peak\n");
		fprintf(stderr,"		 1= standard score, z= (value-mean)/stddev\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	See \"-o\" option, above\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */

	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0) setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0) setheaderbytes=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0) setstart=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0) setntoread=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0) setfmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) setfmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-res")==0) setresonance=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0) setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-z")==0) setnorm=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be 0-9 \n\n",thisprog,setdatatype); exit(1);}

	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: sample frequency (-sf %g) must be > 0\n\n",thisprog,setsampfreq);exit(1);};
	if(setout<1||setout>3) {fprintf(stderr,"\n--- Error[%s]: invalid output (-o %d), must be 1-3\n\n",thisprog,setout);exit(1);};
	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n--- Error[%s]: -verb (%d) must be 0 or 1\n\n",thisprog,setverb);exit(1);};
	if(setnorm<-1||setnorm>1) {fprintf(stderr,"\n--- Error[%s]: -zb (%d) must be -1, 0 or 1 \n\n",thisprog,setnorm);exit(1);};

	/************************************************************
	READ THE DATA
	************************************************************/
	parameters[0]=setdatatype;
	parameters[1]=setheaderbytes; // header-bytes
	parameters[2]=setstart; // numbers to skip
	parameters[3]=setntoread; // if set to zero, will read all available bytes after the header+(start*datasize)

	data1 = xf_readbin2_f(infile,parameters,message);

	nn=parameters[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
	if(data1==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);goto END1;}
	if(nn<10) {fprintf(stderr,"\n--- Error[%s]: insufficient data for cycle-detection (need at least 10 points)\n\n",thisprog);goto END2;}

	/************************************************************
	CONDITION AND FILTER THE DATA
	************************************************************/
	/* interpolate NANs */
	w= xf_interp3_f(data1,nn);
	if(w<0) {fprintf(stderr,"\n--- Error[%s]: no valid data in %s\n\n",thisprog,infile); goto END2;}

	/* pad the data */
	npad=(int)((float)nn/4.0);
	if(npad<1 || npad>nn) npad=nn;
	n2=nn+npad+npad;
	data1= xf_padarray2_f(data1,nn,npad,3,message);
	if(data1==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n",thisprog,message);goto END2;}

	/*apply the filter */
	w = xf_filter_bworth1_f(data1,n2,setsampfreq,setfmin,setfmax,setresonance,message);
	if(w<0) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n",thisprog,message);status=1; goto END2;}

	/************************************************************
	SHIFT DATA1 FORWARD TO SKIP PADDING
	- because padding increases size of array
	- must shift back before freeing the data
	************************************************************/
	data1+=npad;

	/************************************************************
	DETECT CYCLES: note that original nn is used, not n2
	************************************************************/
	/* determine hypothetical range of cycle-lenngths (used for exclusion criteria) */
	cmin=(size_t)(setsampfreq/setfmax);
	cmax=(size_t)(setsampfreq/setfmin);
	/* detect the cycles */
	x= xf_detectcycles2_f(data1,nn,cmin,cmax, &cstart,&cpeak,&cstop,&cycletot, message);
	if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); goto END3;}


	/************************************************************
	CALCULATE NORMALIZED AMPLITUDE
	************************************************************/
	if(setout==1) {

		if((camp=(float *)realloc(camp,(cycletot)*sizeof(float)))==NULL){fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=0;ii<cycletot;ii++) {
			camp[ii]= data1[cpeak[ii]] - ((data1[cstart[ii]] + data1[cstart[ii]]) / 2.0);
		}
		if(setnorm>=0) {
			xf_stats2_f(camp,(long)cycletot,1,result_f);
			if(setnorm==0) {
				b=result_f[5]; // max
				for(ii=0;ii<cycletot;ii++) camp[ii]=(camp[ii])/b;
			}
			if(setnorm==1) {
				a=result_f[0]; // mean
				b=result_f[2]; // standard deviation
				for(ii=0;ii<cycletot;ii++) camp[ii]=(camp[ii]-a)/b;
			}
		}
	}

	/************************************************************
	OUTPUT
	************************************************************/
	// cycle start-peak-stop triplets
	if(setout==1) {
		printf("start\tpeak\tend\tamp\n");
		for(ii=0;ii<cycletot;ii++) printf("%ld\t%ld\t%ld\t%.3f\n",cstart[ii],cpeak[ii],cstop[ii],camp[ii]);
	}

	// filtered raw data
	if(setout==2) {
		for(ii=0;ii<nn;ii++) printf("%f\n",data1[ii]);
	}

	// average cycle (normalized to length of shortest cycle)
	if(setout==3) {
		/* find maximum cycle length */
		cmin=cmax= cstop[0]-cstart[0];
		for(ii=0;ii<cycletot;ii++) {
			jj= cstop[ii]-cstart[ii];
			if(jj<cmin) cmin=jj;
			if(jj>cmax) cmax=jj;
		}
		/* increase value by one */
		cmin++;
		cmax++;

		if((sum=(double *)calloc(cmin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);goto END3;}
		if((nsum=(size_t *)calloc(cmin,sizeof(size_t)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog); goto END3;}
		if((tempdata=(float *)malloc(cmax*sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);goto END3;}
		/* tally up the sums */
		for(ii=0;ii<cycletot;ii++) {
			/* copy data for current cycle to temporary array */
			kk=0; for(jj=cstart[ii];jj<=cstop[ii];jj++) tempdata[kk++]=data1[jj];
			/* downsample to size of shortest cycle */
			mm=0;
			aa= xf_bin1a_f(tempdata,&kk,&mm,cmin,message);
			if(aa==0.0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
			/* calculate the sum */
			for(jj=0;jj<cmin;jj++) sum[jj]+=tempdata[jj];
		}

		/* print the average */
		for(ii=0;ii<cmin;ii++) printf("%ld\t%f\n",ii,(sum[ii]/cycletot));

		/* free memory - only here because if setout != 3 sum is not allocated any memory anyway */
		free(sum);
		free(nsum);
		free(tempdata);
	}

	/* if you get to this point we assume there were no errors */
	status=0;

END3:
	free(cstart);
	free(cpeak);
	free(cstop);
	free(camp);
	data1-=npad ;
END2:
	free(data1);
END1:
	exit(status);
}
