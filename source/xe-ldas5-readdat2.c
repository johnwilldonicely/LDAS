#define thisprog "xe-ldas5-readdat2"
#define TITLE_STRING thisprog" v 0: 31.January.2019 [JRH]"
#define MAXLINELEN 1000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h> /* to get maximum possible short value */
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS> file signal_processing filter </TAGS> */

/************************************************************************
v 0: 31.January.2019 [JRH]
	- add option to de-mean data before anti-aliasing & downsampling

v 0: 29.November.2018 [JRH]
	- add ability to use zero or -1 as the invalid value
v 0: 10.May.2016 [JRH]
	- add filtering for miniumum number of good points

v 0: 24.April.2016 [JRH]
	- derived from readdat1 v 7: 28.March.2016 [JRH]
	- new version of multi-channel dat file read to extract and process a single-channel
		- single-channel read completely into memory
		- added option for internal float conversion
			- supporting functions for each data type
		- added linear interpolation
		- added proper fractional decimation
		- added anti-aliasing filter (101-tap FIR) for decimation
		- allow saving as binary short or float
	- removed options for signed versus unsigned input
	- removed corrections for original bit-depth (correction when flipping data
	- removed arvbitrary value to ad to data
	- removed manual determination of blocksize
	- removed header definition (there is never a header in a .dat file)

*************************************************************************/

/* external functions start */
int xf_readbin1_v(FILE *fpin, void *buffer1, off_t *params, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_filter_mingood2_s(short *data0, size_t nn, size_t nchan, size_t mingood, short setbad, char *message);
int xf_filter_mingood2_f(float *data0, size_t nn, size_t nchan, size_t mingood, float setbad, char *message);
int xf_interp4_s(short *data, long ndata, short invalid, int setfill, long *result);
int xf_interp4_f(float *data, long ndata, float invalid, int setfill, long *result);
int xf_demean1_s(short *input, long nn, long nwin1, char *message);
int xf_demean1_f(float *input, long nn, long nwin1, char *message);
double *xf_filter_FIRcoef1(int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta, char *message);
int xf_filter_FIRapply1_s(short *data, long nn, double *coefs, int ncoefs, int shift, char *message);
int xf_filter_FIRapply1_f(float *data, long nn, double *coefs, int ncoefs, int shift, char *message);
long xf_decimate_s(short *data, long ndata, double winsize,  char *message);
long xf_decimate_f(float *data, long ndata, double winsize,  char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINELEN],message[256],*pline,*pcol;
	long int ii,jj,kk,nn,mm;
	int v,w,x,y,z,col,colmatch,sizeofshort=sizeof(short),sizeoffloat=sizeof(float);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL;
	short *buffpoint=NULL,*data1=NULL,bval,bval2,flipval;
	short realmin=-2047, realmax=2047; // 2^11, the real min/max value for the system
	int datasize,itemsread,itemstoread,blocksread;
	long result_l[8];
	long maxread,blocksize;
	off_t params[4]={0,0,0,0},block,nread;
	float *data2=NULL; // if data is to be converted to float, store it here

	/* variables for filtering, if needed */
	char fwin[]="kaiser";
	int ftaps,fpass,setshift;
	double *coefs=NULL,fomega,fbandwidth,fbeta=3,nyquist;

	/* arguments */
	char buffpointtype[256],*infile=NULL;
	int setout=0,setverb=0,setflip=0,setbad=1,setinterp=0,setfloat=0;
	double setdecimate=0,setsfreq=-1;
	off_t setstart=0,setntoread=0;
	long setmingood=0,setmean=0,setch=0,setchtot=1;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract & process 1 channel from an interlaced binary dat file\n");
		fprintf(stderr,"	- data is read into memory for processing\n");
		fprintf(stderr,"	- allows flipping, interpolation, conversion, and downsampling\n");
		fprintf(stderr,"	- use this program with caution for very large files\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		- samples are 16-bit short integers\n");
		fprintf(stderr,"		- a sample refers to a multi-channel set of buffpoint\n");
		fprintf(stderr,"		- channel and sample indices are zero-offset\n");
		fprintf(stderr,"		- eg. samp2/ch5 of a 16ch input is indexed 2*16+5\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-nch: total number of channels [%ld]\n",setchtot);
		fprintf(stderr,"	-ch: channel to extract: 0-(nch-1) [%ld]\n",setch);
		fprintf(stderr,"	-s: start reading at this sample (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of samples to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"		NOTE: if unset, all channels are output\n");
		fprintf(stderr,"	-bad: invalid value (0,-1, or 1=SHRT_MAX) [%d]\n",setbad);
		fprintf(stderr,"	-mg: minimum good values in a row to keep data [%ld]\n",setmingood);
		fprintf(stderr,"		- sequences of less than this will be set to invalid\n");
		fprintf(stderr,"	-f: flip good data values (0=NO 1=YES) [%d]\n",setflip);
		fprintf(stderr,"	-int: interpolate across invalid values (0=NO 1=YES) [%d]\n",setinterp);
		fprintf(stderr,"	-mean: size of sliding-window used to demean input (0=SKIP) [%ld]\n",setmean);
		fprintf(stderr,"	-dec: decimate to every nth sample (0=NO)[%g]\n",setdecimate);
		fprintf(stderr,"		NOTES: non-integers are allowed,, for precise downsampling\n");
		fprintf(stderr,"		     : an FIR anti-aliasing filter will be applied\n");
		fprintf(stderr,"		     	- 101 taps, Kaiser window, beta=3\n");
		fprintf(stderr,"		     	- will require setting -sf (below)\n");
		fprintf(stderr,"		     	- will reduce amplitude for high decimation\n");
		fprintf(stderr,"	-sf: sampling frequency (Hz), required for decimation [%g]\n",setsfreq);
		fprintf(stderr,"	-con: convert data to float for processing (0=NO 1=YES) [%d]\n",setfloat);
		fprintf(stderr,"	-out: output format (0=ASCII, 1=binary) [%d]:\n",setout);
		fprintf(stderr,"		NOTE: binary out is short (-con 0) or float (-con 1)\n");
		fprintf(stderr,"	-verb: verbose output (0=low,1=high,2=highest) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s buffpoint.txt -nch 64 -ch 0,1,2\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -nch 64 -ch 62,63\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/************************************************************/
	infile= argv[1];
	if(strcmp(infile,"stdin")!=0 && stat(infile,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);
		exit(1);
	}
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-nch")==0)   setchtot=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-ch")==0)    setch=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bad")==0)   setbad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-mg")==0)    setmingood=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-con")==0)   setfloat=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0)     setflip=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-int")==0)   setinterp=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-mean")==0)  setmean=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-dec")==0)   setdecimate=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)    setsfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)     setstart=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)     setntoread=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setchtot<1) {fprintf(stderr,"\n--- Error[%s]: invalid -nch (%ld) : must be >0\n\n",thisprog,setchtot);exit(1);}
	if(setch<0||setch>=setchtot) {fprintf(stderr,"\n--- Error[%s]: invalid -ch (%ld) : must be >0 and <nch (%ld)\n\n",thisprog,setch,setchtot);exit(1);}
	if(setbad<-1||setbad>1) {fprintf(stderr,"\n--- Error[%s]: invalid -bad (%d) : must be -1, 0, or 1 \n\n",thisprog,setbad);exit(1);}
	if(setmingood<0) {fprintf(stderr,"\n--- Error[%s]: invalid -mg (%ld) : must be >= 0\n\n",thisprog,setmingood);exit(1);}
	if(setinterp!=0 && setinterp!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -rep (%d) : must be 0-1\n\n",thisprog,setinterp);exit(1);}
	if(setout!=0 && setout !=1) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be 0 or 1\n\n",thisprog,setout);exit(1);}
	if(setflip<0 || setflip>1) {fprintf(stderr,"\n--- Error[%s]: invalid -f (%d) : must be 0-1\n\n",thisprog,setflip);exit(1);}
	if(setfloat<0 || setfloat>1) {fprintf(stderr,"\n--- Error[%s]: invalid -con (%d) : must be 0-1\n\n",thisprog,setfloat);exit(1);}
	if(setverb<0 || setverb>2) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-2\n\n",thisprog,setverb);exit(1);}
	if(setdecimate<0.0) {fprintf(stderr,"\n--- Error[%s]: invalid -dec (%g) : must be >= 0.00\n\n",thisprog,setdecimate);exit(1);}
	if(setdecimate>0 && setsfreq<=0.0) {fprintf(stderr,"\n--- Error[%s]: decimation requires setting sample-frequency (-sf %g)\n\n",thisprog,setsfreq);exit(1);}

	/************************************************************/
	/* SET DATASIZE AS THE BYTES READ FOR EACH MULTI-CHANNEL SAMPLE */
	/************************************************************/
	datasize=setchtot*sizeof(short);

	/************************************************************/
	/* INTERPRET SETBAD - IF "1", SET BAD VALUE TO SHORT_MAX  */
	/************************************************************/
 	if(setbad==0)  bval=0;
 	if(setbad==-1) bval=-1;
	if(setbad==1)  setbad=SHRT_MAX;

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE (MULTI_CHANNEL SAMPLES) */
	/* overridden by -b or -dec */
	/************************************************************/
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two

	/************************************************************/
	/* ALLOCATE BUFFER MEMORY */
	/************************************************************/
	maxread= blocksize*setchtot*datasize;
	if((buffer1=(void *)realloc(buffer1,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/************************************************************/
	/* OPEN INPUT AND SKIP THE REQUIRED NUMBER OF BYTES */
	/************************************************************/
	if(setverb==1) {
		fprintf(stderr,"\treading channel %ld from %s\n",setch,infile);
		if(setfloat==1) fprintf(stderr,"\tconverting to float\n");
	}
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file: %s\n\n",thisprog,infile);exit(1);}
	ii= setstart*datasize;
	if(fseeko(fpin,ii,SEEK_CUR)!=0) {
		fprintf(stderr,"\n--- Error[%s]:  problem reading binary input (errno=%d)\n\n",thisprog,ferror(fpin));
		exit(1);
	}
	/************************************************************/
	/* READ THE INPUT */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tblocksize:  %ld samples x %ld channels\n",blocksize,setchtot);
	if(setverb==2) fprintf(stderr,"\treading block: ");
	params[0]=datasize; /* ensures an error if bytes read do not match channel count */
	params[1]=blocksize;
	nn=block=z=0;
	while(!feof(fpin)) {
		if(setverb==2) fprintf(stderr,"%9ld\b\b\b\b\b\b\b\b\b",block);
		/* read a block of buffpoint */
		x= xf_readbin1_v(fpin,buffer1,params,message);
		/* check for error (fail to read, bad number of bytes read) */
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		/* set a short pointer to the buffer1, because that's what the input data is */
		buffpoint=(short *)buffer1;
		/* get the number of multi-channel buffpoint read */
		nread=params[2];
		/* if no buffpoint was read this time, that's the end of the file! */
		if(nread==0) break;
		/* if too much was read, adjust the nread variable to reduce the amount of output and set the break flag */
		if( setntoread>0 && (nn+nread)>=setntoread ) { nread=setntoread-nn; z=1; }
		/* this is the total number of values read */
		jj=nread*setchtot;
		/* store the data: two options, 3 (short) or 8 (convert to float) */
		if(setfloat==0) {
			/* allocate additional (nread) memory */
			data1=(short *)realloc(data1,(nn+nread)*sizeofshort);
			if(data1==NULL) { fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog); exit(1); }
			/* store the specified channel - channel number is the initial offset for the loop */
			for(ii=setch;ii<jj;ii+=setchtot) data1[nn++]=buffpoint[ii];
		}
		else {  // as above but convert to float
			data2=(float *)realloc(data2,(nn+nread)*sizeoffloat);
			if(data2==NULL) { fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog); exit(1); }
			for(ii=setch;ii<jj;ii+=setchtot) data2[nn++]=(float)buffpoint[ii];
		}
		/* once the last bit is read, z should have been set to 1 : if so, break */
		if(z!=0) break;
		block++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==1) fprintf(stderr,"\tread %ld samples\n",nn);

	// if(setfloat==0) for(ii=0;ii<nn;ii++) printf("%d\n",data1[ii]);
	// else            for(ii=0;ii<nn;ii++) printf("%g\n",data2[ii]);
	// exit(0);


	/************************************************************/
	/* KEEP ONLY SEQUENCES OF DATA AT LEAST <SETMINGOOD> LONG  */
	/************************************************************/
	if(setmingood>0) {
		if(setfloat==0) {
			x= xf_filter_mingood2_s(data1,nn,1,setmingood,setbad,message);
			if(x!=0) fprintf(stderr,"--- Warning: no valid data points found\n");
		}
		else {
			x= xf_filter_mingood2_f(data2,nn,1,setmingood,(float)setbad,message);
			if(x!=0) fprintf(stderr,"--- Warning: no valid data points found\n");
		}
	}

	/************************************************************/
	/* FLIP - IGNORING VALUES WHICH ARE DEFINED AS "BAD" */
	/************************************************************/
	if(setflip==1) {
		if(setverb==1) fprintf(stderr,"\tflipping data");
		if(setfloat==0) {
			if(setbad==0) {	for(ii=0;ii<nn;ii++) data1[ii]=-data1[ii]; }
			else { for(ii=0;ii<nn;ii++) { if(data1[ii]!=setbad) data1[ii]=-data1[ii]; } }
		}
		else {
			if(setbad==0) {	for(ii=0;ii<nn;ii++) data2[ii]=-data2[ii]; }
			else { for(ii=0;ii<nn;ii++) { if(data2[ii]!=setbad) data2[ii]=-data2[ii]; } }
		}
	}


	/************************************************************/
	/* INTERPOLATE (SETBAD MUST ALSO NOT BE 0) */
	/************************************************************/
	if(setinterp==1 && setbad!=0) {
		if(setverb==1) fprintf(stderr,"\tinterpolating if data=%d\n",setbad);
		if(setfloat==0) x= xf_interp4_s(data1,nn,setbad,3,result_l);
		else            x= xf_interp4_f(data2,nn,(float)setbad,3,result_l);
		if(x!=0 && setverb==1) fprintf(stderr,"--- Warning: no valid data points found\n");
	}

	/************************************************************/
	/* DE-MEAN THE DATA */
	/************************************************************/
	if(setmean>0) {
		if(setmean>nn) {
			setmean=nn;
			if(setverb==1) fprintf(stderr,"\t--- Warning: reducing demean window (-mean %ld) to fit short data length (%ld)\n",setmean,nn);
		}

		if(setverb==1) fprintf(stderr,"\tde-meaning with a %ld-sample window\n",setmean);
		if(setfloat==0) {
			z= xf_demean1_s(data1,nn,setmean,message);
			if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		}
		else{
			z= xf_demean1_f(data2,nn,setmean,message);
			if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		}
	}


	/************************************************************/
	/* DECIMATE (ANTI-ALIASING FILTER APPLIED FIRST) */
	/************************************************************/
	if(setdecimate>0.0) {
		/* first apply anti-aliasing filter */
		fpass=1; // low-pass filter
		ftaps=101; // 41-201 recommended, 41 = similar to Butterworth
		fbandwidth= 2.0 * (1.0 / setsfreq) ; // unused by LP filter, arbitrarily set for 1Hz
		nyquist= 0.5 * (setsfreq / setdecimate); // corner-frequency = 1/2 the sample-frequency, after decimation
		fomega=  2.0 * (nyquist/setsfreq) ;
		/* ... calculate filter coefficients */
		coefs= xf_filter_FIRcoef1(ftaps,fpass,fomega,fbandwidth,fwin,fbeta,message);
		if(coefs==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		/* ... apply the filters using shift=2 to adjust for phase-shift and pad */
		if(setverb==1) fprintf(stderr,"\tapplying anti-aliasing filter (Nyquist=%g Hz)\n",nyquist);
		if(setfloat==0) x= xf_filter_FIRapply1_s(data1,nn,coefs,ftaps,2,message);
		else            x= xf_filter_FIRapply1_f(data2,nn,coefs,ftaps,2,message);
		if(x!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

		/* now decimate */
		if(setverb==1) fprintf(stderr,"\tdecimating by %g (downsampling from %g to %g Hz)\n",setdecimate,setsfreq,(setsfreq/setdecimate));
		if(setfloat==0) nn= xf_decimate_s(data1,nn,setdecimate,message);
		else            nn= xf_decimate_f(data2,nn,setdecimate,message);
		if(nn<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	/************************************************************/
	/* OUTPUT THE DATA  */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\toutput data\n");
	if(setout==0){
		if(setfloat==0) for(ii=0;ii<nn;ii++) printf("%d\n",data1[ii]);
		if(setfloat==1) for(ii=0;ii<nn;ii++) printf("%g\n",data2[ii]);
	}
	if(setout==1) {
		if(setfloat==0) fwrite(data1,sizeof(short),nn,stdout);
		if(setfloat==1) fwrite(data2,sizeof(float),nn,stdout);
	}

	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(buffer1!=NULL) free(buffer1);
	exit(0);

}
