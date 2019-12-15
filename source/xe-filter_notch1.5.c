#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-filter_notch1"
#define TITLE_STRING thisprog" v5: 18.September.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing filter</TAGS>

v5: 18.September.2015 [JRH]
	- update padding function used (hold mean of first and last good samples)

v4: 16.September.2015 [JRH]
	- bugfix in call to xf_writebin2_v: should pass data size, not type

v 3: 8.October.2014 [JRH]
	- BUGFIX to binary write
		- previously did not allow setting of datasize
		- now xf_writebin2 has been modified to acccept datatype as argument instead, so size is determined within the function
		- data type for output is fxed to float, for simplicity

v 2: 3.October.2014 [JRH]
	- add binary read/write

*/



/* external functions start */
float *xf_padarray3_f(float *data, long nn, long npad, int type, char *message);
long xf_interp3_f(float *data, long ndata);
int xf_filter_notch1_f(float *X, size_t nn, float sample_freq, float notch_freq, float notch_width, char *message);
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN];
	long ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	off_t startbyte=0,bytestoread,parameters[8];
	size_t n1,n2,start,stop,nbad,npad=0;
	float *data1=NULL;
	double sum,mean;
	/* arguments */
	off_t setheaderbytes=0;
	long int setpad=-1;
	int setdatatype=-1,setdatatypeout=-1,setoutpad=0, setint=1, setdemean=0, setverb=0;
	float setsampfreq=100.0; // sample frequency of input (samples/s)
	float notch_freq=0.0,notch_width=0;  // filter cutoffs
	float setresonance=1.0; // filter resonance setting ()

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Apply a bidirectional recursive notch filter to the input\n");
		fprintf(stderr,"	- filtering is bidirectional to avoid temporal shifting of data\n");
		fprintf(stderr,"	- non-numeric values, NAN or INF will invalidate the output\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" comprised of a single column\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data  [%d]\n",setdatatype);
		fprintf(stderr,"		-1= ASCII\n");
		fprintf(stderr,"		 0-9= binary uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"		NOTE: binary input will produce binary (float) output\n");
		fprintf(stderr,"	-h: for binary input, size of header (bytes) excluded from output [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-n: notch (centre of stop-band, Hz) [%g]\n",notch_freq);
		fprintf(stderr,"	-w: notch width (Hz) [%g]\n",notch_width);
		fprintf(stderr,"	-m: de-mean the data before filtering (0=NO, 1=YES) [%d]\n",setdemean);
		fprintf(stderr,"	-pad: apply cosine-tapered padding? (-1=AUTO,0=NO,>0=SAMPLES) [%ld]\n",setpad);
		fprintf(stderr,"		NOTE: if -low is set, default=  4x the low freq. wavelength\n");
		fprintf(stderr,"		NOTE: if -low is not set, default=  1/4 the input length\n");
		fprintf(stderr,"	-int: interpolate across invalid data (0=NO,exit instead, 1=YES [%d]\n",setint);
		fprintf(stderr,"	-op: output padding as well as original data (0=NO,1=YES) [%d]\n",setoutpad);
		fprintf(stderr,"	-v: set verbose output (0=NO,1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)   setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0) setheaderbytes=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)    notch_freq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)    notch_width=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-m")==0)    setdemean=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-res")==0) 	setresonance=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pad")==0) 	setpad=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-op")==0) 	setoutpad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-int")==0) 	setint=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)    setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: sample frequency (-sf %g) must be > 0\n\n",thisprog,setsampfreq);exit(1);};
	if(notch_freq<0) {fprintf(stderr,"\n--- Error[%s]: notch frequency (-n %g) must be >= 0 \n\n",thisprog,notch_freq);exit(1);};
	if(notch_width<0) {fprintf(stderr,"\n--- Error[%s]: notch width (-w %g) must be >= 0 \n\n",thisprog,notch_width);exit(1);};
	if(setpad<0&&setpad!=-1) {fprintf(stderr,"\n--- Error[%s]: padding (-pad %ld) must be -1, 0 or >0\n\n",thisprog,setpad);exit(1);};
	if(setoutpad!=0 && setoutpad!=1) {fprintf(stderr,"\n--- Error[%s]: padding-output (-op %d) must be 0 or 1 \n\n",thisprog,setoutpad);exit(1);};
	if(setint!=0 && setint!=1) {fprintf(stderr,"\n--- Error[%s]: interpolation (-int %d) must be 0 or 1 \n\n",thisprog,setint);exit(1);};
	if(setdemean!=0 && setdemean!=1) {fprintf(stderr,"\n--- Error[%s]: de-meaning (-m %d) must be 0 or 1 \n\n",thisprog,setdemean);exit(1);};
	if(setverb!=0 && setverb!=1) {fprintf(stderr,"\n--- Error[%s]: verbocity (-v %d) must be 0 or 1 \n\n",thisprog,setverb);exit(1);};
	if(setdatatype<-1||setdatatype>9) {fprintf(stderr,"\n--- Error[%s]: -dt (%d) must be -1 or 0-9\n\n",thisprog,setdatatype);exit(1);};


	/********************************************************************************/
	/* STORE RAW DATA - SINGLE COLUMN OR BINARY STREAM */
	/********************************************************************************/
	if(setdatatype==-1) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		n1= nbad= 0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%f",&a)!=1) a=NAN;
			if((data1=(float *)realloc(data1,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			if(!isfinite(a)) nbad=1;
			data1[n1++]=a;
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	else {
		parameters[0]= setdatatype;
		parameters[1]= setheaderbytes;
		parameters[2]= 0; /* bytes to skip */
		parameters[3]= 0; /* bytes to read (zero = read all) */
		nbad= 0;
		data1= xf_readbin2_f(infile,parameters,message);

		if(data1!=NULL) n1=parameters[3];
		else { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}
	//TEST:	for(ii=0;ii<1000;ii++) printf("%ld	%g\n",ii,data1[ii]);free(data1);exit(0);
	n2=n1;

	/********************************************************************************
	WARN, OUTPUT & EXIT IF THERE ARE TOO FEW DATA POINTS TO ACTUALLY FILTER
	********************************************************************************/
	if(n1==0){
		fprintf(stderr,"\n--- Error[%s]: input \"%s\" is empty\n\n",thisprog,infile);
		exit(1);
	}
	if(n1<4) {
		for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]);
		if(data1!=NULL) free(data1);
		fprintf(stderr,"--- Warning[%s]: input \"%s\" has less than 4 data points - no filtering performed\n",thisprog,infile);
		exit(0);
	}

	/********************************************************************************/
	/* INTERPOLATE IF THERE ARE INVALID DATAPOINTS */
	/********************************************************************************/
	if(nbad==1) {
		if(setint==1) {
			kk= xf_interp3_f(data1,(size_t)n1);
			if(kk<0) {
				fprintf(stderr,"\n\t --- Error [%s]: input \"%s\" contains no valid numbers\n\n",thisprog,infile);
				free(data1);
				exit(1);
			}
		}
		else {
				fprintf(stderr,"\n\t --- Error [%s]: input \"%s\" contains invalid numbers - use interpolation option -int\n\n",thisprog,infile);
				free(data1);
				exit(1);
		}
	}

	/********************************************************************************
	DE-MEAN THE DATA
	********************************************************************************/
	if(setdemean==1) {
		sum=0.0;
		for(ii=0;ii<n1;ii++) sum+=data1[ii];
		mean=sum/(double)n1;
		for(ii=0;ii<n1;ii++) data1[ii]-=mean;
	}

	/********************************************************************************
	PAD THE DATA
	********************************************************************************/
	/* automatic padding selection: 4/notch_freq or n1/4 */
	if(setpad<0) {
		if(notch_freq<=0.0) npad=(int)((float)n1/4.0);
		else npad=(int)((setsampfreq*4.0)/notch_freq);
		if(npad<1 || npad>n1) npad=n1;
		n2=n1+npad+npad;
	}
	/* user-specified padding */
	else if(setpad>0) {
		npad=setpad;
		n2=n1+npad+npad;
	}
	/* no padding */
	else {
		npad=0;
		n2=n1;
	}
 	/* apply the cosine-taper padding type 3 (beginning and end) */
 	if(npad>0) {
		data1 = xf_padarray3_f(data1,n1,npad,3,message);
		if(data1==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);free(data1);exit(1);}
	}


	/********************************************************************************
	APPLY THE FILTER
	********************************************************************************/
	ii= xf_filter_notch1_f(data1,n2,setsampfreq,notch_freq,notch_width,message);

	if(ii<0) {
		fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);
		free(data1);
		exit(1);
	}
	if(setverb==1) {
		fprintf(stderr,"\n");
		fprintf(stderr,"%s\n",message);
		fprintf(stderr,"infile=%s\n",infile);
		fprintf(stderr,"samples: %d\n",n1);
		fprintf(stderr,"padding: %d\n",npad);
		fprintf(stderr,"\n");
	}

	/********************************************************************************
	OUTPUT THE FILTERED DATA, OMITTING ANY PADDING
	********************************************************************************/
	if(setoutpad==0) { start=npad; stop=n1+npad; }
	if(setoutpad==1) { start=0; stop=n2; n1=n2;}

	if(setdatatype==-1) {
		for(ii=start;ii<stop;ii++) printf("%g\n",data1[ii]);
	}
	else {
		ii= xf_writebin2_v("stdout",(void *)(data1+start),n1,sizeof(float),message);
		if(ii!=0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}

	free(data1);
	exit(0);

exit(0);
}
