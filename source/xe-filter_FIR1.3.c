#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define thisprog "xe-filter_FIR1"
#define TITLE_STRING thisprog" v 3: 30.April.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing filter</TAGS>
v 3: 30.April.2016 [JRH]
	- add ability to define the invalid data value for interpolation

v 3: 11.May.2015 [JRH]
	- add ability to correct for data shift = (NumTaps-1)/2
	- remove limit to number of taps

v 2: 07.May.2015 [JRH]
	- add ability to use function to apply coefficients
	- added interpolation for invalid points
*/

/* external functions start */
double *xf_filter_FIRcoef1(int NumTaps, int PassType, double OmegaC, double BW, char *WindowType, double WinBeta, char *message);
int xf_filter_FIRapply1_f(float *data, long nn, double *coefs, int ncoefs, int shift, char *message);
long xf_interp3_f(float *data, long ndata);
int xf_interp4_f(float *data, long ndata, float invalid, int setfill, long *result);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[MAXLINELEN],*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long ii,jj,kk,nn,nbad;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd,ee;
	FILE *fpin,*fpout;
	size_t sizeoffloat=sizeof(float);

	/* program-specific variables */
	long result[8];
	float *data1=NULL;
	double *coefs=NULL,OmegaC,BW;
	double setbad=NAN;

	/* arguments */
	char WindowType[16];
	int NumTaps=41,PassType=1,setshift=2,setout=1;
	double setwidth=3.0,WinBeta=0.0,setsampfreq=1.0,setfreq=0.1;
	snprintf(WindowType,16,"kaiser");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"FIR filter program source-code\n");
		fprintf(stderr,"Assumes one valid numeric value per input line\n");
		fprintf(stderr,"Non-numeric values will be interpolated across\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%f]\n",setsampfreq);
		fprintf(stderr,"	-freq: corner freq. (LP,HP) or central freq. (BP, notch) [%g]\n",setfreq);
		fprintf(stderr,"	-pt: pass type 1=LP, 2=HP, 3=BP, 4=notch [%d]\n",PassType);
		fprintf(stderr,"	-wt: window type (none,kaiser,sync) [%s]\n",WindowType);
		fprintf(stderr,"		: windowing helps reduce ripple & other artefacts\n");
		fprintf(stderr,"	-bw: bandwidth, for BP and notch only [%g]\n",setwidth);
		fprintf(stderr,"	-ntaps: number of taps (51-255 recommended) [%d]\n",NumTaps);
		fprintf(stderr,"		: 51 gives results similar to Butterworth\n");
		fprintf(stderr,"	-beta: transition bandwidth (0-10) for Kaiser or Sinc windows [%g]\n",WinBeta);
		fprintf(stderr,"		: low values vive sharper cutoffs\n");
		fprintf(stderr,"	-out: output 0=coefficients, 1=filtered data [%d]\n",setout);
		fprintf(stderr,"	-shift: correct for shift (0-2) [%d]\n",setshift);
		fprintf(stderr,"		0: no correction, data will be shifted forward\n");
		fprintf(stderr,"		1: back-shift data to correct, last-sample-padding added\n");
		fprintf(stderr,"		2: also avoid starting-artefact (add equal front-padding)\n");
		fprintf(stderr,"	-bad: alternative value to interpolate across [%g]\n",setbad);
		fprintf(stderr,"		NOTE: if data also contains NAN,INF or non-numeric\n");
		fprintf(stderr,"		values, then these may get used for interpolation\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-bad")==0)   setbad=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)    setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-freq")==0)  setfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pt")==0)    PassType=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-wt")==0)    snprintf(WindowType,16,"%s",argv[++ii]);
			else if(strcmp(argv[ii],"-bw")==0)    setwidth=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-beta")==0)  WinBeta=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ntaps")==0) NumTaps=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-shift")==0) setshift=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	/* check validity of arguments */
	if(WinBeta<0||WinBeta>10) {fprintf(stderr,"\n--- Error[%s]: -beta (%g) must be 0-10\n\n",thisprog,WinBeta);exit(1);}
	if(PassType<1||PassType>4) {fprintf(stderr,"\n--- Error[%s]: -pt (%d) must be 1-4\n\n",thisprog,PassType);exit(1);}
	if(setshift<0||setshift>2) {fprintf(stderr,"\n--- Error[%s]: -shift (%d) must be 0-2\n\n",thisprog,setshift);exit(1);}
	if(setout<0||setout>1) {fprintf(stderr,"\n--- Error[%s]: -out (%d) must be 0-1\n\n",thisprog,setout);exit(1);}

	if(setfreq>(setsampfreq/2.0)) {fprintf(stderr,"\n--- Error[%s]: -freq (%g) must be less than 1/2 sample frequency (%g)\n\n",thisprog,setfreq,setsampfreq);exit(1);}

	OmegaC= 2.0 * (setfreq/setsampfreq) ;
	BW= 2.0 * (setwidth/setsampfreq) ;

	/********************************************************************************
	STORE DATA METHOD 1a - newline-delimited single float
	-  in this example, interpolate bad points to preserve line-count
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=nbad=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) { a=NAN; nbad++;}
		if((data1=(float *)realloc(data1,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data1[nn]=a;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: input \"%s\" is empty\n\n",thisprog,infile);exit(1);};
	if(nn<NumTaps) {fprintf(stderr,"\n--- Error[%s]: more coefficients (NumTaps=%d) than data (n=%ld) in input \"%s\"\n\n",thisprog,NumTaps,nn,infile);exit(1);};

	/********************************************************************************
	INTERPOLATE IF SOME DATA IS BAD
	********************************************************************************/
	if(nbad>0) {
		if(!isfinite(setbad)) {
			ii= xf_interp3_f(data1,nn);
			if(ii<0) {fprintf(stderr,"\n--- Error[%s]: input \"%s\" contains no finite numbers\n\n",thisprog,infile);exit(1);};
		}
		else {
			x= xf_interp4_f(data1,nn,setbad,3,result);
			if(x==1) {fprintf(stderr,"\n--- Error[%s]: input \"%s\" contains no valid numbers (invalid=%g)\n\n",thisprog,infile,setbad);exit(1);};
		}
	}


	/********************************************************************************
	CALCULATE THE FILTER COEFFICIENTS
	********************************************************************************/
	/*
	fprintf(stderr,"NumTaps:%d\n",NumTaps);
	fprintf(stderr,"PassType:%d\n",PassType);
	fprintf(stderr,"OmegaC:%g\n",OmegaC);
	fprintf(stderr,"BW:%g\n",BW);
	fprintf(stderr,"WindowType:%d\n",WindowType);
	fprintf(stderr,"WinBeta:%g\n",WinBeta);
	*/

	coefs= xf_filter_FIRcoef1(NumTaps,PassType,OmegaC,BW,WindowType,WinBeta,message);
	if(coefs==NULL) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

	/********************************************************************************
	OUTPUT THE COEFFICIENTS ...
	********************************************************************************/
	if(setout==0) {
		for(ii=0;ii<NumTaps;ii++) printf("%ld	%g\n",ii,coefs[ii]);
	}
	/********************************************************************************
	... OR APPLY THE COEFFICIENTS TO THE DATA AND OUTPUT
	********************************************************************************/
	else if(setout==1) {
		x= xf_filter_FIRapply1_f(data1,nn,coefs,NumTaps,setshift,message);
		if(x!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

		for(ii=0; ii<nn; ii++) printf("%g\n",data1[ii]);
	}

	free(data1);
	free(coefs);

	exit(0);
}
