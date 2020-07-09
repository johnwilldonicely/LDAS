#define thisprog "xe-detectevents1"
#define TITLE_STRING thisprog" v 4: 12.November.2018 [JRH]"
#define MAXLINELEN 1000
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
<TAGS>signal_processing</TAGS>

v 4: 12.November.2018 [JRH]
	- allow start & stop setting for normalizing to baseline

v 4: 4.November.2018 [JRH]
	- add pre-normalization filter options
	- add ability to use a baseline (samples 0-base1) for normalization

v 4: 6.June.2018 [JRH]
	- add peak value (pval) and event-normalized peak-value (npval) to output - saves much hassle for other programs!

v 4: 15.January.2016 [JRH]
	- bugfix: setupper is now a multiple of the threshold >1
	- for sign-independent detection (sign=0) threshold is converted to absolute value
	- added ability to enforce a detection refractory period between event peaks (applied after initial detection)
	- added ability to reserve -pre and -post samples after peak
		- ie. to avoid detection for the first -pre or last -post samples
		- prevents problems later if you look for chunks of data surrounding the peaks
	- add ability to convert signal to z-scores before detection with user-defined mean and stddev

v 4: 29.March.2015 [JRH]
	- do not skip non-finite numbers - now interpolates instead
	- some corrections to instructions

v 3: 12.March.2015 [JRH]
	- add upper limit (u) to exclude events which are too large
	- new detect function identifies peak-value in the event and outputs this instead of the mid-point

v 2: 17.February.2015 [JRH]
	- allow setting of edge detection criterion
	- slightly improved xf_detectevents function - only includes samples above edge threshold

v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
long xf_interp3_f(float *data, long ndata);
float *xf_padarray4_f(float *data, long *nn, long npad, int type, char *message);
int xf_filter_notch1_f(float *X, size_t nn, float sample_freq, float notch_freq, float notch_width, char *message);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_stats2_f(float *data1, long nn, int setlarge, float *result_f);
long xf_detectevents2_f(float *data,long n1,float thresh, float upper, float edge, int sign,long nmin,long nmax,long **estart, long **epeak, long **estop,char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long ii,jj,kk,nn,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[16];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	size_t sizeoffloat=sizeof(float);

	/* program-specific variables */
	char *words=NULL;
	long n1,n2,nbad,nevents,mid,nwin=50000,npad=0;
	float *data0=NULL,*datanorm=NULL,*pdata0;
	long *estart=NULL,*epeak=NULL,*estop=NULL;
	off_t parameters[8];
	/* arguments */
	int setsign=0,setdatatype=-1,setverb=0;
	float setsampfreq=-1.0, setthresh=3.0, setupper=10.0, setedge=0.5,setzscore1=0.0,setzscore2=1.0;
	long setbase1=-1,setbase2=-1,setmin=1,setmax=0,setref=0,setpre=0,setpost=0;
	float notch_freq=50.0,notch_width=0.0;  // Notch filter settings
	float setlow=0.0,sethigh=0,setresonance=1.4142;;  // Butterworth filter settings

	if((line=(char *)realloc(line,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Thresholded event-detection in a time series\n");
		fprintf(stderr,"- non-numeric and non-finite values will be interpolated\n");
		fprintf(stderr,"- optional notch and Butterworth filtering\n");
		fprintf(stderr,"- normalization to baseline or user-defined mean & std.dev\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" representing data series\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data [%d]\n",setdatatype);
		fprintf(stderr,"		-1 = ascii\n");
		fprintf(stderr,"		0-9= uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-verb sets verbosity (0=simple, 1=verbose) [%d]\n",setverb);
		fprintf(stderr," ...filter options...\n");
		fprintf(stderr,"	-sf sample-frequency (Hz) - must be >0 if filters are used [%g]\n",setsampfreq);
		fprintf(stderr,"	-notchw: notch filter width (Hz, 0=none) [%g]\n",notch_width);
		fprintf(stderr,"	-notchf: notch filter stop-band, Hz) [%g]\n",notch_freq);
		fprintf(stderr,"	-low: Butterworth filter low freq. limit (0=none) [%g]\n",setlow);
		fprintf(stderr,"	-high: Butterworth filter high freq. limit (0=none) [%g]\n",sethigh);
		fprintf(stderr," ...z-score normalization options...\n");
		fprintf(stderr,"	-base1: start-sample for baseline-derived z-score [%ld]\n",setbase1);
		fprintf(stderr,"	-base2: stop-sample  for baseline-derived z-score [%ld]\n",setbase2);
		fprintf(stderr,"		-1,-1= skip, 0,-1= use total dataset\n");
		fprintf(stderr,"	-z1: manually define subtractor (mean, 0=none) [%g]\n",setzscore1);
		fprintf(stderr,"	-z2: manually define divisor (stddev, 1=none) [%g]\n",setzscore2);
		fprintf(stderr," ...threshold options...\n");
		fprintf(stderr,"	-t: detection lower threshold [%g]\n",setthresh);
		fprintf(stderr,"		NOTE: can be positive or negative\n");
		fprintf(stderr,"	-e: multiple of -t (0-1) defining event edges) [%g]\n",setedge);
		fprintf(stderr,"	-u: multiple of -t (>1) defining rejection limit [%g]\n",setupper);
		fprintf(stderr,"		- event peak values must fall between (-t) and (-u)*(-t) \n");
		fprintf(stderr,"	-s: detection sign (-1=negative, +1=positive, 0=either) [%d]\n",setsign);
		fprintf(stderr,"		NOTE: for -ive, -t may also need to be -ive\n");
		fprintf(stderr,"	-min: minimum event length (0=none) [%ld]\n",setmin);
		fprintf(stderr,"	-max: maximum event length (0=none) [%ld]\n",setmax);
		fprintf(stderr,"	-ref: impose refractory period between peaks (0=none) [%ld]\n",setref);
		fprintf(stderr,"	-pre: reserved samples before each event peak [%ld]\n",setpre);
		fprintf(stderr,"	-post: reserved samples after each event [%ld]\n",setpost);
		fprintf(stderr,"		- pre and post determine the first and last input sample\n");
		fprintf(stderr,"		  which can be used for detection of valid events\n");
		fprintf(stderr,"		- eg. -pre 5 rejects events before input sample 5\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 3  -e 0.5 -s 1 -min 12000 -max 120000\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	event	start	peak	stop	pval	apval\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"	event: event-number (starts from 0)\n");
		fprintf(stderr,"	start: sample-number for leading edge of event\n");
		fprintf(stderr,"	peak:  sample-number for peak (minimum of maximum) of event\n");
		fprintf(stderr,"	stop:  sample-number for trailing edge of event\n");
		fprintf(stderr,"	pval:  data-value at peak (filtered, but pre-normalization)\n");
		fprintf(stderr,"	npval: normalized pval (see z-score options)\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)   setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)    setthresh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0)   setsampfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-notchf")==0)  notch_freq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-notchw")==0) notch_width=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)   setlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0)  sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-base1")==0) setbase1=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-base2")==0) setbase2=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-z1")==0)   setzscore1=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-z2")==0)   setzscore2=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-u")==0)    setupper=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-e")==0)    setedge=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)    setsign=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)  setmin=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)  setmax=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-ref")==0)  setref=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-pre")==0)  setpre=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-post")==0) setpost=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setbase1!=-1 && (setzscore1!=0.0 || setzscore2!=1.0)) { fprintf(stderr,"\n--- Error [%s]: cannot set both -base1 and -z1/-z2: use one normalization method or the other\n\n",thisprog);exit(1);}
	if(setbase2!=-1 && (setzscore1!=0.0 || setzscore2!=1.0)) { fprintf(stderr,"\n--- Error [%s]: cannot set both -base2 and -z1/-z2: use one normalization method or the other\n\n",thisprog);exit(1);}
	if(setbase1==-1 && setbase2!=-1) { fprintf(stderr,"\n--- Error [%s]: if -base1 is \"-1\", -base2 (%ld) must also be \"-1\"\n\n",thisprog,setbase2);exit(1);}
	if(setbase2!=-1 && setbase2<=setbase1) { fprintf(stderr,"\n--- Error [%s]: -base1 (%ld) must be < -base2 (%ld)\n\n",thisprog,setbase1,setbase2);exit(1);}


	/* STORE DATA METHOD 1a - newline-delimited single float */
	n1=nbad=0;
	if(setdatatype==-1) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) { a=NAN; nbad++;}
			if((data0=(float *)realloc(data0,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			data0[n1]=a;
			n1++;
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	else if(setdatatype>=0 && setdatatype <=9) {
		parameters[0]=setdatatype;
		parameters[1]=0; // header-bytes
		parameters[2]=0; // numbers to skip
		parameters[3]=0; // if set to zero, will read all available bytes after the header+(start*datasize)
		data0 = xf_readbin2_f(infile,parameters,message);// read into memory, converting to float
		n1=parameters[3]; // parameters[3] is reset to the number of elements (bytes/datasize) by xf_readbin2_f
		if(data0==NULL) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
		for(ii=0;ii<n1;ii++) if(!isfinite(data0[ii])) nbad++;
	}
	if(setverb==1) fprintf(stderr,"\tRead %ld data points (%ld needed interpolation)\n",n1,nbad);

	/* allocate memory for normalized copy of data */
	if((datanorm=realloc(datanorm,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/********************************************************************************/
	/* INTERPOLATE IF SOME DATA IS BAD */
	/********************************************************************************/
	if(nbad>0) xf_interp3_f(data0,n1);


	/********************************************************************************/
	/* APPLY FILTERS  */
	/********************************************************************************/
	npad= 0;
	if(notch_width>0.0 || setlow>0.0 || sethigh>0.0) {
		if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: sample frequency (%g) must be >0 for use of filters\n\n",thisprog,setsampfreq);exit(1);};

		/* de-mean the data */
		aa=0.0; for(ii=0;ii<n1;ii++) aa+= data0[ii]; aa/= (double)n1;
		for(ii=0;ii<n1;ii++) data0[ii]-= aa;
		/* apply padding (1 second sample-and-hold) */
		npad= (long)(setsampfreq*1.0);
		n2= n1;
		data0 = xf_padarray4_f(data0,&n2,npad,3,message);
		/* notch filter */
		if(notch_width>0.0) {
			if(setverb==1) fprintf(stderr,"\tapplying %gHz notch filter (width %gHz)\n",notch_freq,notch_width);
			ii= xf_filter_notch1_f(data0,n2,setsampfreq,notch_freq,notch_width,message);
			if(ii<0) { fprintf(stderr,"\n\t --- Error [%s]: %s\n\n",thisprog,message); exit(1); }
		}
		/* Butterworth filter*/
		if(setlow>0.0 || sethigh>0.0) {
			if(setverb==1) fprintf(stderr,"\tapplying Butterworth filter (%gHz - %gHz)\n",setlow,sethigh);
			z= xf_filter_bworth1_f(data0,n2,setsampfreq,setlow,sethigh,setresonance,message);
			if(z<0) { fprintf(stderr,"\n\t --- Error [%s]: %s\n\n",thisprog,message); exit(1); }
		}
		/* reapply the mean */
		for(ii=0;ii<n1;ii++) data0[ii]+= aa;
		/* adjust start position of now-padded data array */
		data0 +=npad;
	}

	/********************************************************************************
	MAKE A Z-SCORE VERSION OF THE DATA
	- z1 (mean) and z2 (std.dev) are 0 and 1 respectively by default
	- if they are manually set to anything else, base1 is not allowed to be set
	- but if base1 is set, calculate mean and std.dev from the baseline period
	********************************************************************************/
	if(setbase1!=-1) {
		if(setbase1>=n1) {fprintf(stderr,"\n--- Error[%s]: -base1 (%ld) exceeds number of data points (%ld)\n\n",thisprog,setbase1,n1);exit(1);};
		if(setbase2>n1)  {fprintf(stderr,"\n--- Error[%s]: -base2 (%ld) exceeds number of data points (%ld)\n\n",thisprog,setbase2,n1);exit(1);};
		if(setbase2==-1) setbase2= n1; // use rest of the data
		ii= setbase2-setbase1; // total samples to use fr baseline
		z= xf_stats2_f((data0+setbase1),ii,1,result_f);
		setzscore1= result_f[0]; // mean
		setzscore2= result_f[2]; // standard deviation
	}
	if(setverb==1) {
		fprintf(stderr,"\tnormalization mean: %g\n",setzscore1);
		fprintf(stderr,"\tnormalization std.dev: %g\n",setzscore2);
	}
	/* apply normalization */
	for(ii=0;ii<n1;ii++) datanorm[ii]= (data0[ii]-setzscore1)/setzscore2;


	/********************************************************************************/
	/* DETECT EVENTS */
	/********************************************************************************/
	nevents= xf_detectevents2_f(datanorm,n1,setthresh,setupper,setedge,setsign,setmin,setmax,&estart,&epeak,&estop,message);
	if(nevents<0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}


	/********************************************************************************/
	/* MAKE SURE EVENT PEAKS ARE A MINIMUM DISTANCE APART */
	/* i.e. after detection, reject subsequent detection within imposed refractory period */
	/********************************************************************************/
	if(setref>0 && nevents >0) {
		jj=0;
		for(ii=1;ii<nevents;ii++) {
			if((epeak[ii]-epeak[jj])>=setref) {
				jj++;
				estart[jj]=estart[ii];
				epeak[jj]=epeak[ii];
				estop[jj]=estop[ii];
			}
		}
		nevents=jj+1;
	}

	/********************************************************************************/
	/* APPLY PRE/POST FILTER - exclude events from the beginng/end of the input */
	/********************************************************************************/
	if(setpre>0 || setpost>0) {
		setpost= n1-setpost;
		for(ii=jj=0;ii<nevents;ii++) {
			if(epeak[ii]>setpre && epeak[ii]<setpost) {
				estart[jj]=estart[ii];
				epeak[jj]=epeak[ii];
				estop[jj]=estop[ii];
				jj++;
		}}
		nevents=jj;
	}

	/********************************************************************************/
	/* OUTPUT - and calculate peak value and normalized peak value */
	/********************************************************************************/
	if(setverb==1) fprintf(stderr,"\tFound %ld events\n\n",nevents);
	printf("event	start	peak	stop	pval	npval\n");
	for(ii=0;ii<nevents;ii++) {
		printf("%ld	%ld	%ld	%ld	%g	%g\n",
		ii,estart[ii],epeak[ii],estop[ii],data0[epeak[ii]],datanorm[epeak[ii]]);
	}


	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
	/* if padding had been applied, re-adjust data0 before freeing */
	if(npad>0) data0-= npad;
	/* free memory */
	if(data0!=NULL) free(data0);
	if(datanorm!=NULL) free(datanorm);
	if(estart!=NULL) free(estart);
	if(epeak!=NULL) free(epeak);
	if(estop!=NULL) free(estop);
	/* exit */
	exit(0);
}
