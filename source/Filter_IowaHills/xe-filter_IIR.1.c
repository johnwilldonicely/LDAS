#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define thisprog "xe-filter_IIR"
#define TITLE_STRING thisprog" v 1: 4.October.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*

v 1: 4.October.2015 [JRH]
	to compile: xs-progcompile xe-filter_IIR.1.c -c g++ -o ./ -O 0
	to test: clear ; ./xe-filter_IIR $in1 -p 10

??? TO DO: 
	- calcualtion of Omega & Bandwidth (0-1) 
		- from xf_filter_bworth5: omega = tan( M_PI * (double)low_freq/(double)sample_freq );
	- change passtype to a number 1-4 (low,high,band,notch)
	- functio(type,pass,sfreq,stopband,width,ripple,gamma)
	
	- test phase-shift, implement reverse-filter?

*/

/* external functions start */
double *xf_filter_IIRcoef1_d(char *setfilt, double *params, int *nsections, char *message);
int xf_filter_IIRapply1_d(double *Input, double *Output, long NumSigPts, double *coefs, int NumSections, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN];
	long int i,j,k,n,nbad,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64]; 
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,ll,nn,mm;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */ 
	double *coefs=NULL;
	int nsections;
	off_t datasize,startbyte,ntoread,nread,bytestoread,parameters[8];
	double *data1=NULL,*data2=NULL,params[16];

	/* arguments */
	char setfilt[32];	// butterworth,chebyshev,chebyshevinv,gauss,gaussadj,bessel,elliptic
	int setout=1;           // output coefficients (0) or filtered data (1) 
	int setpoles=2;         // 2-10, for 1-5th-order filters
	int settype=1;       // 1=lowpass,2=highpass,3=bandpass,4=notch
	double setfreq=0.25;    // stop-band (Hz) - must be < 0.5 * setsampfreq
	double setsampfreq=1.0; // sample-frequency (Hz)
	double setwidth=0.1;    // bandwidth (Hz) for bandwidth and notch filters
	double setrip=0.0;      // 0.0 - 1.0 dB for Chebyshev in increments of 0.10  0.0 - 0.2 for the Elliptics in increments 0f 0.02
	double setdb=0.0;       // 20 - 100 dB for the Inv Cheby  40 - 100 for the Elliptics
	double setgamma=0.0;    // -1.0 <= Gamma <= 1.0  Gamma controls the transition BW on the Adjustable Gauss

	snprintf(setfilt,32,"butterworth");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"IIR Filter program\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-filt: filter model to use [%s]\n",setfilt);
		fprintf(stderr,"		butterworth\n");
		fprintf(stderr,"		chebyshev\n");
		fprintf(stderr,"		chebyshevinv (inverse Chebyshev) \n");
		fprintf(stderr,"		gauss\n");
		fprintf(stderr,"		gaussadj (adjusted Gaussian)\n");
		fprintf(stderr,"		bessel\n");
		fprintf(stderr,"		elliptic\n");
		fprintf(stderr,"	-poles: poles, 2-10 (except Elliptics, which are 4-10) [%d]\n",setpoles);
		fprintf(stderr,"	-pt: pass type 1=LP, 2=HP, 3=BP, 4=notch [%d]\n",settype);
		fprintf(stderr,"	-freq: corner freq. (LP,HP) or central freq. (BP, notch) [%g]\n",setfreq);
		fprintf(stderr,"	-db: stop band dB [%g]\n",setdb);
		fprintf(stderr,"		- for chebyshevinv, 20 - 100 dB\n");
		fprintf(stderr,"		- for elliptic, 40-100\n");
		fprintf(stderr,"	-sf: sample frequency (sample/s) [%g]\n",setsampfreq);
		fprintf(stderr,"	-bw: bandwidth, for BP and notch only [%g]\n",setwidth);
		fprintf(stderr,"	-rip: ripple dB  [%g]\n",setrip);
		fprintf(stderr,"		- for Chebyshev, 0.0 - 1.0, in 0.1 increments\n");
		fprintf(stderr,"		- for Eliptical, 0.0 - 0.2, in 0.02 increments\n");
		fprintf(stderr,"	-gamma: controls gaussadj transition bandwidth (-1 to 1) [%g]\n",setgamma);
		fprintf(stderr,"	-out: output 0=coefficients, 1=filtered data [%d]\n",setout);
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


	/* READ THE INPUT NAME AND OPTIONAL ARGUMENTS  */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-filt")==0)   snprintf(setfilt,32,"%s",argv[++ii]);
			else if(strcmp(argv[ii],"-poles")==0)  setpoles=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-pt")==0)     settype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-freq")==0)   setfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-db")==0)     setdb=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)     setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-bw")==0)     setwidth=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-rip")==0)    setrip=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-gamma")==0)  setgamma=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setout<0||setout>1) {fprintf(stderr,"\n--- Error[%s]: -out (%d) must be 0-1\n\n",thisprog,setout);exit(1);}

	/* READ THE DATA */
	nn=0; 
 	if(strcmp(infile,"stdin")==0) fpin=stdin;
 	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) { 
		if(sscanf(line,"%lf",&aa)!=1) continue;
		if(isfinite(aa)) {
			if((data1=(double *)realloc(data1,(nn+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			data1[nn]=aa; 
			nn++;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
 	//for(ii=0;ii<nn;ii++) printf("%ld:	%g\n",ii,data1[ii]);
	

	/* ALLOCATE MEMORY FOR RESULTS ARRAY */
	if((data2=(double *)realloc(data2,nn*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* CALCULATE THE COEFFICIENTS */
	params[0]=(double)setpoles;
	params[1]=(double)settype;
	params[2]=setfreq;
	params[3]=setsampfreq;
	params[4]=setwidth;
	params[5]=setrip;
	params[6]=setdb;
	params[7]=setgamma;

	coefs= xf_filter_IIRcoef1_d(setfilt,params,&nsections,message);

	/* APPLY THE FILTER  */
	xf_filter_IIRapply1_d(data1,data2,nn,coefs,nsections,message);
	
	
	/* OUTPUT */ 
	if(setout==0) {
		printf("\n");
		for(ii=0;ii<nsections;ii++) {
 			printf("Section %d\n", ii);
 			printf("	a0= %9.9f  a1= %9.9f  a2= %9.9f\n", coefs[ii*6+0], coefs[ii*6+1], coefs[ii*6+2]);
 			printf("	b0= %9.9f  b1= %9.9f  b2= %9.9f\n\n",coefs[ii*6+3], coefs[ii*6+4], coefs[ii*6+5]);
 		}
	} 
	else if(setout==1) {
		for(ii=0;ii<nn;ii++) printf("%g\n",data2[ii]);	
	}

	
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(coefs!=NULL) free(coefs);
	exit(0);
}

