#define thisprog "xe-fftpow2"
#define TITLE_STRING thisprog" v 6: 11.September.2018 [JRH]"
#define MAXLINELEN 1000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "kiss_fftr.h"

/*
<TAGS>signal_processing spectra</TAGS>

REFERENCES:
http://www.ni.com/white-paper/4278/en/
http://zone.ni.com/reference/en-XX/help/371361B-01/lvanlsconcepts/compute_amp_phase_spectrums/
http://zone.ni.com/reference/en-XX/help/371361B-01/lvanlsconcepts/convert_to_log_units/

v 6: 11.September.2018 [JRH]
	- minor fix to unequal-block-size detection (warning only)
	- allow -1 to auto-set frequency minimum or maximum - remove redundant -setmin/max variables

v 6: 17.November.2017 [JRH]
	- change windowing to ensure entirety of each data block is spanned
	- if last window(s) extend beyond the end of the block, adjust accordingly
	- this may duplicate FFT results at the end of the block but improves consistency ni the number of output windows

v 6: 17.July.2017 [JRH]
	- add header to ASCII mean-spectrum output

v 6: 31.August.2016 [JRH]
	- apply bugfix to AUC output  - needed to calculate index-boundaries for bands AFTER reading arguments and defining window size

v 6: 30.August.2016 [JRH]
	- change output sample-number for output-type 2 - first sample in window, not the mid-window sample
		- this means the output is a little predictive
		- however, it also means that the timestamps start from a logical "zero"

v 6: 25.July.2016 [JRH]
	- add output style for timestamp (sample) + AUC for delta,theta,beta, and gamma bands
	- use long ints instead of size_t for counter variables - more robust if -ive numbers are needed or possible
	- fix mis-reporting of window overlap when overlap is 0%
	- remove error for providing odd window-sizes for FFT - just add 1 to the value

v 6: 18.July.2016 [JRH]
	- update block definition to use .ssp files

v 6: 29.May.2016 [JRH]
	- bugfix: proper reporting of max frequency out now, corrected for frequency-resolution
	- use function xf_taperhann_d to calculate taper
	- drop setnwin_f variable - really don't need it

v 6: 13.May.2016 [JRH]
	- correct calculation and reporting of minimum frequency to account for frequency resolution
	- fix and simplify verbose reporting to enable extraction of parameters from stderr using xe-getkey
	- drop support for BINX format - switch to binary flat-file support
	- add control over binary output (-binout 0=ASCII, 1=double)
	- update variable names for long to ii,jj,kk etc

v 6: 15.November.2015 [JRH]
	- fix path to kiss_fftr.h - functions subfolder has been retired

v 5: 7.January.2015 [JRH]
	- make default setstep=1, not 2, and error if setstep<1

v 4: 11.November.2014 [JRH]
	- simplify taper order modification (no need for "auto" since we have dropped multi-taper)
	- bugfix:
		- previously, large files authomatically generated very small minimum frequency values
		- consequently the FFT window had to be very large and memory problems could occur
		- now the default minimum is 0.1Hz if the data length permits, or higher as required


v 3: 10.November.2014 [JRH]
	- slight correction to dB output: add 1 before doing log transform to make sure all values are positive

v 2: 24.October.2014 [JRH]
	- restored ability to output amplitude converted to decibels

v 1: 15 September 2014 [JRH]
	- derived from xe-fftpow1.30.c
	- remove multitaper option-
		- it is of dubious utility with  multiply-sampled data and affects amplitude output
		- removal eliminates several functions, no need for intermediate ampall variable
	- rename variables:
		- tapers to taper
		- buffer to window (FFT window) - in keeping with xe-fftcoh1
		- amp to spect, to better reflect that output can be amplitude, RMS, or RMS2
		- remove unused morenbuff variable
	- reduce output options to amplitude, RMS, and RMS2 (no more decibels, as this is a ratio and only suitable for power change, really)
	- window file now holds block start/stop sample pairs
		- average spectra are first averaged within each block
		- matrix spectra are output with comment-separators and will need to be averaged by xe-matrixavg2


*/

/* external functions start */
void *xf_readbin3_v(char *infile, off_t *params, off_t *start, off_t *toread, char *message);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
double *xf_taperhann_d(long setn, int setmean, float setpow, char *message);
long xf_interp3_f(float *data, long ndata);
float xf_round1_f(float input, float setbase, int setdown);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
// NOTE: the following function declarations are commented out to avoid re-initialization in kiss headers,
// They are included here only so xs-progcompile (which won't detect that they are commented out) will include them during compilation
/*
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
*/
/* external functions end */


int main(int argc, char *argv[]) {

	/* general-use variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	int q,r,s,t,x,y,z,stat=0;
	size_t sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	long ii,jj,kk,ll,mm,nn;
	float a,b,c,d,e,f;
	double aa,bb,cc,dd,ee,ff,gg,twopi=2.0*M_PI;
	FILE *fpin,*fpout;

	/* program-specific variables */
	int datasize=0;
	long halfnwin,partnwin,blocksize,windexmem,nbad=0;
	long window,wintot=0,*windex=NULL,block,blocktot=1,*blockstart=NULL,*blockstop=NULL;
	float *buff2=NULL,*data=NULL,*pdata,*freq=NULL,*Fval=NULL,sum=0.0,mean,sample_interval=0.0F,scaling1;
	double *taper=NULL,*spect=NULL,*spectmean=NULL,*spectmean2=NULL,ar,ai,freqres;

	long nbands,*bandA2=NULL,*bandZ2=NULL;
	double *bandA1=NULL,*bandZ1=NULL;

	// binary file read variables
	void *datavoid=NULL;
	off_t params[3],readstart[1],readn[1],ntowrite;
	unsigned char *p0=NULL;
	unsigned short *p2=NULL;
	unsigned int *p4=NULL;
	unsigned long *p6=NULL;
	char *p1=NULL;
	short *p3=NULL;
	int *p5=NULL;
	long *p7=NULL;
	float *p8=NULL;
	double *p9=NULL;

	/* arguments */
	char *setblockfile=NULL;
	int indexa=-1,indexb=-1,setnwin=-1,setstep=1,setout=0,setverb=0,setorder=1,setgauss=0,setunits=0;
	int settaper=1,setmean=1,setdatatype=-1,setbinout=0;
	long setstart=0,setntoread=0; /// currently undefined - set for whole-file read
	float setsfreq=1000.0,minfreq=-1.0,maxfreq=-1.0;
	double time;

	sprintf(outfile,"stdout");

	/* PRE-DEFINE PROTOTYPE FREQUENCY BANDS */
	nbands=4;
	if((bandA1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandZ1= (double*)calloc(nbands,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandA2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	if((bandZ2= (long*)calloc(nbands,sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);};
	bandA1[0]=0.5;  bandZ1[0]=4.0; // delta - Buzsaki
	bandA1[1]=4.0;  bandZ1[1]=12.0; // theta - Whishaw
	bandA1[2]=13.0; bandZ1[2]=30.0; // beta - Magill (20Hz mean)
	bandA1[3]=30.0; bandZ1[3]=100.0; // gamma (mixed)


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Use KISS-FFT functions to produce a power spectrum density output\n");
		fprintf(stderr,"Requires input from a file or standard input, one column only\n");
		fprintf(stderr,"FFT is performed on data windows which can overlap\n");
		fprintf(stderr,"Power at each frequency is then averaged for entire file\n");
		fprintf(stderr,"Produces two columns: freq, power\n");
		fprintf(stderr,"USAGE:	%s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: binary or single-column ASCII input, filename or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS (defaults in [])\n");
		fprintf(stderr,"	-dt: data type [%d]\n",setdatatype);
		fprintf(stderr,"		-1  : ASCII\n");
		fprintf(stderr,"		0-9 : uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-sf: sampling frequency (Hz) of the input [%g]\n",setsfreq);
		fprintf(stderr,"	-min: min freq. [-1= default= 2*sf/n or 0.1, whichever is higher]\n");
		fprintf(stderr,"	-max: max freq. [-1= default= sf/2]\n");
		fprintf(stderr,"	-w: samples per FFT window (must be even, -1= scaled to min) [%d]\n",setnwin);
		fprintf(stderr,"		* longer window = detailed output, low temporal resolution\n");
		fprintf(stderr,"		* frequency resolution = sample_frequency / window_size\n");
		fprintf(stderr,"	-s: steps for the sliding window to span one window length [%d]\n",setstep);
		fprintf(stderr,"		* e.g. if -w 8 -s 4, the window moves by 8/4=2 samples per FFT\n");
		fprintf(stderr,"		* NOTE: must be a factor of the window length\n");
		fprintf(stderr,"	-scrf: screening file (.ssp) for defining blocks of data [unset]\n");
		fprintf(stderr,"		* NOTE: use only to define large blocks of data (minutes)\n");
		fprintf(stderr,"		* if unset, single block for entire input is assumed\n");
		fprintf(stderr,"		* -o 0: output's mean spectrum for all windows and all blocks\n");
		fprintf(stderr,"		* -o 1: each matrix will have a block-header\n");
		fprintf(stderr,"	-m: mean-correct data in each window (0=NO, 1=YES) [%d]\n",setmean);
		fprintf(stderr,"	-t: taper type: 0=none, 1=Hann [%d]\n",settaper);
		fprintf(stderr,"	-p: power to raise the taper, higher values increase slope [%d]\n",setorder);
		fprintf(stderr,"	-g: apply Gaussian smoothing to output (avg.spectrum only) [%d]\n",setgauss);
		fprintf(stderr,"		* note: -g must be 0 (none) or an odd number 3 or larger)\n");
		fprintf(stderr,"	-u: set the units for spectrum output [%d]\n",setunits);
		fprintf(stderr,"		0=peak amplitude (PA)= 2x sqrt(FFTreal^2 + FFTimag^2)/windowsize\n");
		fprintf(stderr,"		1=PA expressed as decibels\n");
		fprintf(stderr,"		2=RMS .......= PA x (sqrt(2)/2) \n");
		fprintf(stderr,"		3=power......= RMS-squared\n");
		fprintf(stderr,"	-v: set verbosity to quiet (0) report (1) or taper-only (-1) [%d]\n",setverb);
		fprintf(stderr,"	-o: output format (0-3) [%d]\n",setout);
		fprintf(stderr,"		0= spectrum averaged across windows and blocks\n");
		fprintf(stderr,"		1= matrix of values, row= time (window) column=frequency\n");
		fprintf(stderr,"		2= like (1) but with timestamp in 1st column\n");
		fprintf(stderr,"			- time is sample-number relative to input start\n");
		fprintf(stderr,"			- this takes into account block-reading (-scrf option)\n");
		fprintf(stderr,"		3= like (2) but with AUC for freq. bands instead of full spectrum\n");
		fprintf(stderr,"			delta = %g - %g Hz\n",bandA1[0],bandZ1[0]);
		fprintf(stderr,"			theta = %g - %g Hz\n",bandA1[1],bandZ1[1]);
		fprintf(stderr,"			beta  = %g - %g Hz\n",bandA1[2],bandZ1[2]);
		fprintf(stderr,"			gamma = %g - %g Hz\n",bandA1[3],bandZ1[3]);
		fprintf(stderr,"	-binout: binary (64-bit double) output (0=NO 1=YES) [%d]\n",setbinout);
		fprintf(stderr,"		* NOTE: binary output mean spectra do not include frequencies\n");
		fprintf(stderr,"		* NOTE: binary output matrices do not include block separators\n");
		fprintf(stderr,"		* NOTE: not available for -o 3\n");
		fprintf(stderr,"EXAMPLES: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	%s data.txt -t bin -sf 1500 \n",thisprog);
		fprintf(stderr,"	cat data.bin | %s stdin -t asc \n",thisprog);
		fprintf(stderr,"OUTPUT: \n");
		fprintf(stderr,"	if -o 0:  <frequency> <power>\n");
		fprintf(stderr,"	if -o 1:  spectral matrix, row=window (time), column=frequency\n");
		fprintf(stderr,"	if -o 2:  <time> <spectrum>\n");
		fprintf(stderr,"	if -o 3:  <time> <deltapower> <thetapower> <betapower> <gammapower>\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0)     setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)   setblockfile=argv[++ii];
			else if(strcmp(argv[ii],"-sf")==0)     setsfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)    minfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)    maxfreq=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)      setnwin=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)      setstep=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-t")==0)      settaper=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0)      setorder=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-g")==0)      setgauss=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-u")==0)      setunits=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-m")==0)      setmean=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0)      setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-binout")==0) setbinout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0)      setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
 	/* CHECK OPTIONAL ARGUMENTS */
	if(setsfreq>0.0) sample_interval=1.0F/setsfreq;
	else { fprintf(stderr,"\n--- Error [%s]: bad sample freq. (%g) must be >0\n\n",thisprog,setsfreq);exit(1);}
	if(setnwin<0&&setnwin!=-1) { fprintf(stderr,"\n--- Error [%s]: -b [%d] must be -1 (auto) or a positive number\n\n",thisprog,setnwin);exit(1);}
	if(setout<0||setout>3) { fprintf(stderr,"\n--- Error [%s]: bad output setting [-o %d]:  should be 0-3\n\n",thisprog,setout);exit(1);}
	if(setverb<0&&setverb!=-1) { fprintf(stderr,"\n--- Error [%s]: -v [%d] must be -1 or >= 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setmean<0&&setmean!=-1) { fprintf(stderr,"\n--- Error [%s]: -m [%d] must be 0 or 1\n\n",thisprog,setmean);exit(1);}
	if(settaper!=0&&settaper!=1) { fprintf(stderr,"\n--- Error [%s]: -t [%d] must be 0 or 1\n\n",thisprog,settaper);exit(1);}
	if(setorder<0&&setorder!=-1) { fprintf(stderr,"\n--- Error [%s]: -p [%d] must be -1 or >= 0 or 1\n\n",thisprog,setorder);exit(1);}
	if((setgauss<3&&setgauss!=0)||(setgauss%2==0 && setgauss!=0)) { fprintf(stderr,"\n--- Error [%s]: -g [%d] must be 0 or an odd number 3 or larger\n\n",thisprog,setgauss);exit(1);}
	if(setunits<0||setunits>3) { fprintf(stderr,"\n--- Error [%s]: -u [%d] must be 0,1,or 2\n\n",thisprog,setunits);exit(1);}
	if(setstep<1) { fprintf(stderr,"\n--- Error [%s]: -s [%d] must be >0\n\n",thisprog,setstep);exit(1);}
	if(setbinout!=0&&setbinout!=1) { fprintf(stderr,"\n--- Error [%s]: -binout [%d] must be 0 or 1\n\n",thisprog,setbinout);exit(1);}
	/* check data-type options and set data size */
	if(setdatatype==0||setdatatype==1)      datasize=sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=sizeof(long);
	else if(setdatatype==8)                 datasize=sizeof(float);
	else if(setdatatype==9)                 datasize=sizeof(double);
	else if(setdatatype!=-1) {fprintf(stderr,"\n--- Error[%s]: data type (-dt %d) must be -1 (ASCII) or 0-9 (binary)\n\n",thisprog,setdatatype); exit(1);}


	/********************************************************************************/
	/********************************************************************************/
	/* STORE DATA - ASCII OR BINARY */
	/********************************************************************************/
	/********************************************************************************/
	nn=nbad=0;
	if(setverb>0) {
		fprintf(stderr,"%s\n",thisprog);
		fprintf(stderr,"	* Reading data... (%s)\n",infile);
	}
	if(setdatatype==-1) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
 			if(sscanf(line,"%f",&a)!=1 || !isfinite(a)) {a=NAN; nbad++;}
 			if((data=(float *)realloc(data,(nn+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
 			data[nn++]=a;
  		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
 	}
	else {
		/********************************************************************************
		READ THE DATA AS ONE BLOCK
		- note: error will result if stdin is used with binary input
		********************************************************************************/
		params[0]=datasize;
		params[1]=0; // headerbytes - assume there is no header
		params[2]=1; // read a single block
		params[3]=0; // initialize number of values read
		readstart[0]=setstart;
		readn[0]=setntoread;
		/* read the data */
		datavoid= xf_readbin3_v(infile,params,readstart,readn,message);
		if(datavoid==NULL) { fprintf(stderr,"\n\t--- Error[%s/%s]\n\n",thisprog,message); exit(1); }
		if(params[3]<1) {fprintf(stderr,"\n\t--- Error [%s]: file %s is empty\n",thisprog,infile);exit(1);}
		else nn= params[3];
		/* allocate memory for floating-point output */
		if((data=(float *)realloc(data,nn*sizeof(float)))==NULL) { fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); return(-1); }
		/* assign appropriate pointer and convert to float */
 		if(setverb>0) fprintf(stderr,"	* Converting to float - n=%ld...\n",nn);
 		if(setdatatype==0) { p0= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p0[ii]; if(!isfinite(data[ii])) nbad++; } // uchar
 		if(setdatatype==1) { p1= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p1[ii]; if(!isfinite(data[ii])) nbad++;} // char
 		if(setdatatype==2) { p2= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p2[ii]; if(!isfinite(data[ii])) nbad++; } // ushort
 		if(setdatatype==3) { p3= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p3[ii]; if(!isfinite(data[ii])) nbad++; } // short
 		if(setdatatype==4) { p4= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p4[ii]; if(!isfinite(data[ii])) nbad++; } // uint
 		if(setdatatype==5) { p5= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p5[ii]; if(!isfinite(data[ii])) nbad++; } // int
 		if(setdatatype==6) { p6= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p6[ii]; if(!isfinite(data[ii])) nbad++; } // ulong
 		if(setdatatype==7) { p7= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p7[ii]; if(!isfinite(data[ii])) nbad++; } // long
 		if(setdatatype==8) { p8= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p8[ii]; if(!isfinite(data[ii])) nbad++; } // float
 		if(setdatatype==9) { p9= datavoid; for(ii=0;ii<nn;ii++) data[ii]=(float)p9[ii]; if(!isfinite(data[ii])) nbad++; } // double
		free(datavoid);
	}

	/* check the integrity of the data */
	if(nn==0 || nbad==nn) {fprintf(stderr,"\n--- Error[%s]: no valid data in input\n\n",thisprog);free(data);exit(1);};
	/* interpolate if some data is bad */
	if(nbad>0) xf_interp3_f(data,nn);
	if(setverb>0) fprintf(stderr,"	* Complete: %ld data-points read\n",nn);
	//TEST: for(ii=0;ii<10;ii++) fprintf(stderr,"%ld\t%.6f\n",ii,data[ii]); exit(0);

	/********************************************************************************
	AUTO-DEFINE MIN & MAX FREQUENCY IF NECESSARY
	********************************************************************************/
	/* determine minimum frequency if not explicitly defined */
	if(minfreq<0) {
		minfreq=(2.0*setsfreq)/(double)nn;
		if(minfreq<0.1) minfreq=0.1; // prevent very large FFT windows which might cause memory issues, unless requested
	}
	/* set maximum frequency to the Nyquist frequency (1/2 sampling-rate) */
	if(maxfreq<0) maxfreq=setsfreq/2.0;
	if(maxfreq>(0.5*setsfreq)) { fprintf(stderr,"\n--- Error [%s]: -max [%g] cannot be greater 1/2 the  sample freq. [%g]\n\n",thisprog,maxfreq,setsfreq);exit(1);}
	if(minfreq<=0)             { fprintf(stderr,"\n--- Error [%s]: -min [%g] must greater than zero\n\n",thisprog,minfreq);exit(1);}
	if(minfreq>maxfreq)        { fprintf(stderr,"\n--- Error [%s]: -min [%g] must be less than -max [%g]\n\n",thisprog,minfreq,maxfreq);exit(1);}


	/********************************************************************************
	SET UP setnwin (WINDOW LENGTH) & partnwin (PART-WINDOW LENGTH DETERMINED BY SET-STEP)
	note that setnwin determines the frequency-resolution of the output and hence, the true minimum frequency
	********************************************************************************/
	/* calculate the number of samples representing the longest wavelength to be analyzed */
	a= setsfreq/minfreq;
	/* calculate optimal partnwin : the fraction of two wavelengths spanned by each window-shift */
	y= 2 * (int)(0.5+(a/setstep));
	/* calculate optimal total window length */
	z= y*setstep;
	if(z>nn) z-=2;
	/* now assign values to setnwin and partnwin - compare with optimal values */
	if(setnwin<=0) { setnwin=z, partnwin=y; }
	else {
		if(setnwin%2 != 0) { fprintf(stderr,"\t--- Warning [%s]: window length cannot be odd [-w %d]. Adjusting to %d\n",thisprog,setnwin,(setnwin+1)); setnwin++; }
		if(setnwin<z && setverb>0) { fprintf(stderr,"\t--- Warning [%s]: window [-w %d] < optimal length [%d] for the min.freq. [-min %g]\n",thisprog,setnwin,z,minfreq);}
		if(setnwin%setstep != 0) { fprintf(stderr,"\n--- Error [%s]: step size [-s %d] must be a factor of the window length [%d]\n\n",thisprog,setstep,setnwin);free(data);exit(1);}
		partnwin=setnwin/setstep;
	}
	/* set the frequency resolution of the output */
	freqres= setsfreq/(double)setnwin;
	/* apply correction to the minimum & maximum frequency */
	a=minfreq;
	if(a!=freqres) {
		if(a<freqres) minfreq=freqres;
		if(a>freqres) minfreq=freqres*(int)((double)a/(double)freqres);
		if(setverb>0) fprintf(stderr,"	* Minimum frequency adjusted from %g to %g\n",a,minfreq);
	}
	a=maxfreq;
	if(fmod(a,freqres)!=0) {
		maxfreq=freqres*(int)((double)a/(double)freqres);
		if(setverb>0) fprintf(stderr,"	* Maximum frequency adjusted from %g to %g\n",a,maxfreq);
	}


	/* DEFINE ADDITIONAL WINDOW SIZES */
	halfnwin=setnwin/2; // defines highest index in FFT result for which unique information can be obtained

	/* CALCULATE THE MIN AND MAX INDEX CORRESPONDING TO THE USER-SPECIFIED FREQUENCY RANGE */
	/* This controls which elements of the FFT results are calculated and output */
	indexa=(int)(minfreq*setnwin*sample_interval); if(indexa<1) indexa = 1;
	indexb=(int)(maxfreq*setnwin*sample_interval); if(indexb>=(setnwin/2)) indexb = (setnwin/2)-1;

	/* DEFINE SCALING FACTORS - size of data sent to FFT */
	scaling1=1.0/(float)setnwin; /* defining this way permits multiplication instead of (slower) division */

	/* ALLOCATE EXTRA MEMORY FOR DATA IN CASE DATA LENGTH IS NOT AN EVEN NUMBER OF WINDOWS */
	jj=nn+setnwin;
	if((data=(float *)realloc(data,(jj*sizeoffloat)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* pad this extra space with zeros */
	for(ii=nn;ii<jj;ii++) data[ii]=0.0;
	/* make sure there's enough data for at least one buffer! */
	if(nn<setnwin) {fprintf(stderr,"\n--- Error [%s]: number of data points [%ld] is less than window size [%d]\n\n",thisprog,nn,setnwin); free(data); exit(1);}


	/********************************************************************************/
	/* INITIALIZE KISS-FFT CONFIGURATION AND OTHER VARIABLES */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Initializing Kiss-FFT variables...\n");
	/* initialize kiss-fft variables: cfgr, ffta_a, fft_b */
	kiss_fftr_cfg cfgr = kiss_fftr_alloc( setnwin ,0,0,0 ); /* configuration structure: memory assigned using malloc - needs to be freed at end */
	kiss_fft_cpx fft[setnwin]; /* holds fft results: memory assigned explicitly, so does not need to be freed */
	/* allocate memory for working variables */
	if(setverb>0) fprintf(stderr,"	* Allocating memory...\n");
	if((freq=(float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds pre-caluclated frequencies associated with each FFT index
	if((buff2= (float*)calloc(setnwin,sizeof(float)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // buffer which is passed to the FFT function, copied from pdata
	if((spect= (double*)calloc(setnwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds amplitude of the FFT results
	if((spectmean= (double*)calloc(setnwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds per-block mean FFT results (power) from multiple buff2-s
	if((spectmean2= (double*)calloc(setnwin,sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error [%s]: insufficient memory\n\n",thisprog); exit(1);}; // holds grand total mean FFT results (power) from spectmean[]s

	/********************************************************************************/
	/* CREATE A MODIFIED HANN TAPER
	/* the modification is to normalize the mean value of the taper to 1, instead of the peak */
	/* this allows the taper to maintain the power in each window (normally a Hann taper cuts power by exactly 50%) */
	/* if no taper is to be applied, all taper values are set to "1" via the setpow argument */
	/********************************************************************************/
	if(settaper==0) setorder=0;
	taper= xf_taperhann_d(setnwin,1,setorder,message);
	if(taper==NULL) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	/* IF -v IS SET TO -1, JUST OUTPUT THE TAPER */
	if(setverb==-1) {
		for(ii=0;ii<setnwin;ii++) {printf("%ld	%g\n",ii,taper[ii]);}
		goto END1;
	}


	/********************************************************************************
	DEFINE FREQUENCY VALUES ASSOCIATED WITH FFT OUTPUT
	********************************************************************************/
	for(ii=indexa;ii<=indexb;ii++) {
			/* calculate frequency */
			a= (float) (((double)(ii)/(double)sample_interval) / (double)setnwin );
			/* round to 3 digits - NOTE: this will actually add a few zeros after the 3rd digit but may not exactly round it  - sufficient for printing however */
			freq[ii]= (float)xf_round1_f(a,0.001,0);
	}

	/********************************************************************************/
	/* SET UP THE BLOCK START-TIMES (FOR ALIGNING FFT TO BLOCKS OF DATA) */
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Defining blocks...\n");
	/* If no block time-file was specified, assume a single block spanning the whole record */
	if(setblockfile==NULL) {
		blocktot=1;
		if((blockstart=(long *)realloc(blockstart,1*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		if((blockstop=(long *)realloc(blockstop,1*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		blockstart[0]= 0;
		blockstop[0]= nn;
	}
	/* If a block time-file was specified, use the start times listed in it instead */
	else {

		/* read block file */
		blocktot = xf_readssp1(setblockfile,&blockstart,&blockstop,message);
		if(blocktot==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		//TEST_OUTPUT_BLOCK_TIMES: for(ii=0;ii<blocktot;ii++) { jj=blockstart[ii]; kk=blockstop[ii]; fprintf(stderr,"block %ld = [%ld]-[%ld]	dur=%ld\n",ii,jj,kk,(kk-jj)); }

		/* reject out-of-range blocks and blocks that are smaller than the FFT window size */
		mm= 0;
		for(ii=0;ii<blocktot;ii++) {
			jj= blockstart[ii];
			kk= blockstop[ii];
			if( jj>=0 && jj<=nn && kk>=0 && kk<=nn && (kk-jj)>=setnwin ) {
				blockstart[mm]= jj;
				blockstop[mm]= kk;
				mm++;
		}}
		if(mm<blocktot) {
			if(setverb>0) fprintf(stderr,"\t\t-Warning: %ld blocks dropped (out of range or smaller than FFT window)\n",(blocktot-mm));
			blocktot=mm;
		}
		if(blocktot==0) {fprintf(stderr,"\n--- Error[%s]: no valid blocks in %s\n\n",thisprog,setblockfile);exit(1);}

		/* flag if block sizes vary */
		z= 0;
		mm= blockstop[0]-blockstart[0];
		for(ii=1;ii<blocktot;ii++) {
			jj= blockstart[ii];
			kk= blockstop[ii];
			//TEST: fprintf(stderr,"%ld to %ld - mm=%ld  dur=%ld\n",blockstart[ii],blockstop[ii],mm,(kk-jj));
			if((kk-jj) != mm) z=1;
		}
		if(setverb>0 && setout==1 && z!=0) {fprintf(stderr,"\t\t--- Warning[%s]: block size varies. Number of matrix rows will vary as a consequence\n",thisprog);}

	}
	//TEST_OUTPUT_BLOCK_TIMES: for(ii=0;ii<blocktot;ii++) fprintf(stderr,"block %ld = [%ld]-[%ld]\n",ii,blockstart[ii],blockstop[ii]); exit(0);


	/********************************************************************************
	DEFINE FREQUENCY-BAND INDICES (DELTA,THETA,BETA,GAMMA)
	********************************************************************************/
	for(ii=0;ii<=nbands;ii++) {
		bandA2[ii]= (long)(bandA1[ii]*(double)setnwin*(double)sample_interval );
		bandZ2[ii]= (long)(bandZ1[ii]*(double)setnwin*(double)sample_interval);
		// TEST: printf("band:%ld	A1:%g	Z1:%g	setnwin:%g	sampint:%g	A2:%ld	Z2:%ld\n",ii,bandA1[ii],bandZ1[ii],(double)setnwin,(double)sample_interval,bandA2[ii],bandZ2[ii]);

		if(bandA2[ii]<indexa) bandA2[ii]=indexa;
		if(bandZ2[ii]<indexa) bandZ2[ii]=indexa;
		if(bandA2[ii]>indexb) bandA2[ii]=indexb;
		if(bandZ2[ii]>indexb) bandZ2[ii]=indexb;
	}
	//for(ii=0;ii<nbands;ii++) fprintf(stderr,"%d,%d,",bandA2[ii]-indexa,bandZ2[ii]-indexa);


	/********************************************************************************/
	/* OPEN OUTPUT FILE OR STREAM */
	/********************************************************************************/
	if(strcmp(outfile,"stdout")==0) {
		fflush(stdout);
		fpout=stdout;
	}
	else {
		if(setbinout==0) fpout=fopen(outfile,"w");
		else fpout=fopen(outfile,"wb");
		if(fpout==NULL) { fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile); exit(1);}
	}


	/*****************************************************************************
	/********************************************************************************/
	/* ANALYZE THE DATA  */
	/********************************************************************************/
	/********************************************************************************/
	if(setverb>0) fprintf(stderr,"	* Analyzing the data...\n");

	if(setout==3) printf("sample	delta	theta	beta	gamma\n");

	/* reset block and grand spectrum means */
	if(setout==0) for(ii=indexa;ii<=indexb;ii++) spectmean[ii]=spectmean2[ii]=0.0;

	for(block=0;block<blocktot;block++) {

		/* print block header to matrix output if required */
		if((setout==1||setout==2) && setbinout==0) printf("# block %ld\n",block);
		/* determine current block size */
		blocksize= blockstop[block]-blockstart[block];
		/* how many windows will fit span the current block? allow overrun but correct start-postions in next step */
		if(blocksize%partnwin==0) kk= blocksize/partnwin;
		else kk= (blocksize/partnwin)+1;
		/* reallocate memory if required */
		if(kk>wintot) {if((windex=(long *)realloc(windex,kk*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}}
		wintot= kk;
		/* set up the window start-times (windex) for each fft within a block  */
		for(ii=0;ii<wintot;ii++) {
			jj= blockstart[block]+(ii*partnwin);
			if((jj+setnwin)>blockstop[block]) jj= blockstop[block]-setnwin;
			windex[ii]= jj;
		}
		//TEST: for(window=0;window<wintot;window++) fprintf(stderr,"block:%ld size %ld\twindow:%ld\twindex:%ld\n",block,blocksize,window,windex[window]);

		for(window=0;window<wintot;window++) {

			/* set index to data */
			pdata= data+windex[window];
			/* calculate the mean-correction to window, if required */
			if(setmean==1) { sum=0; for(ii=0;ii<setnwin;ii++) sum+=pdata[ii]; mean=sum*scaling1;}
			else mean= 0.0;
			/* copy real data from pdata to buff2, and apply mean-correction + taper */
			for(ii=0;ii<setnwin;ii++) buff2[ii]= (pdata[ii]-mean) * taper[ii];


			/*********  call the fft function */
			kiss_fftr(cfgr,buff2,fft);


			/********* store the fftreal, fftimag & spectrum:  note that at present this must be done for the entire half-spectrum, not just indexa to indexb */
			if(setunits==0) { /* units= amplitude peak */
				aa=2.0 * scaling1;
				for(ii=0;ii<halfnwin;ii++) { ar= fft[ii].r; ai= fft[ii].i; spect[ii]= aa * sqrtf( ar*ar + ai*ai ); }
			}
			else if(setunits==1) { /* units= amplitude peak (dB) */
				aa=2.0 * scaling1;
				for(ii=0;ii<halfnwin;ii++) { ar= fft[ii].r; ai= fft[ii].i; spect[ii]= 20.0 *log10(1.0 + (aa * sqrtf( ar*ar + ai*ai ))); }
			}
			else if(setunits==2) { /* units= RMS */
				aa=sqrtf(2.0) * scaling1;
				for(ii=0;ii<halfnwin;ii++) { ar= fft[ii].r; ai= fft[ii].i; spect[ii]= aa * sqrtf( ar*ar + ai*ai ); }
			}
			else if(setunits==3) { /* units= RMS-Squared */
				aa= 2.0 * scaling1 * scaling1;
				for(ii=0;ii<halfnwin;ii++) { ar= fft[ii].r; ai= fft[ii].i; spect[ii]= aa * ( ar*ar + ai*ai ); }
			}


			 /* output option-0: build per-block mean amplitude spectrum */
			if(setout==0) {
				for(ii=indexa;ii<=indexb;ii++) spectmean[ii]+= spect[ii];
			}
			/* output option-1: amplitude spectrum for each window (matrix output) */
			else if(setout==1) {
				/* if input is ascii, output is ascii */
				if(setbinout==0) {
					fprintf(fpout,"%g",spect[indexa]);
					for(ii=(indexa+1);ii<=indexb;ii++) printf("\t%g",spect[ii]);
					printf("\n");
				}
				/* if input is binary, output is binary  */
				else {
					x= xf_writebin2_v("stdout",(void *)(spect+indexa),((indexb-indexa)+1),sizeof(double),message);
					if(x<0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				}
			}
			/* option-2: (ASCII only) as above but timestamp + entire spectrum */
			else if(setout==2) {
				// output the timestamp (sample-number relative to input sample-zero)
				printf("%ld",windex[window]);
 				for(ii=indexa;ii<=indexb;ii++) printf("\t%g",spect[ii]);
				printf("\n");
			}
			/* option-3: (ASCII only) column output for current window (time, AUC1, AUC2, AUC3, etc) */
			else if(setout==3) {
				// output the mid-window timestamp (sample-number relative to input sample-zero)
				printf("%ld",(windex[window]));
 				for(ii=0;ii<nbands;ii++) {
 					// calculate the sum of the values in the relevant portion of the spectrum
 					aa=0.0;	for(jj=bandA2[ii];jj<bandZ2[ii];jj++) aa+=spect[jj];
					// print the AUC (sum, corrected by the frequency-resolution in the spectrum)
					printf("\t%g",(aa*freqres));
				}
				printf("\n");
			}

		} /* END OF LOOP for(window=0;window<wintot;window++) */

		/* build grand spectrum mean and reset block spectrum mean to zero */
		if(setout==0) { bb=(double)wintot; for(ii=indexa;ii<=indexb;ii++) {spectmean2[ii]+= spectmean[ii]/bb; spectmean[ii]=0.0;}}

	} /* END OF LOOP for(block=0;block<blocktot;block++) */

	/********************************************************************************
	IF OUTPUT IS AVERAGE SPECTRUM, CALCULATE, SMOOTH, AND OUTPUT NOW
	********************************************************************************/
	/* if output is not a matrix, calculate the mean fft result across windows and  output the results */
	if(setout==0) {

		/* correct spectmean2 by number of blocks */
		b=(float)blocktot; for(ii=indexa;ii<=indexb;ii++) spectmean2[ii]/=b;
		/* smooth portion of fft output that power was actually calculated for */
		if(setgauss!=0) xf_smoothgauss1_d((spectmean2+indexa),(size_t)(indexb-indexa+1),setgauss);

		if(setbinout==0) {
			printf("freq	amp\n");
			for(ii=indexa;ii<=indexb;ii++) printf("%.04f\t%f\n",freq[ii], spectmean2[ii]);
		}
		else {
			x= xf_writebin2_v("stdout",(void *)(spectmean2+indexa),((indexb-indexa)+1),sizeof(double),message);
			if(x<0){ fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		}

	}

	/********************************************************************************
	CLOSE THE OUTPUT FILE IF REQUIRED
	********************************************************************************/
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

	/********************************************************************************/
	/* PRINT DIAGNOSTICS TO STDERR */
	/********************************************************************************/
	if(setverb>0) {
		// determine the units
		if(setunits==0) sprintf(message,"peak_amplitude");
		if(setunits==1) sprintf(message,"decibels");
		if(setunits==2) sprintf(message,"RMS");
		if(setunits==3) sprintf(message,"power (RMS-squared)");

		fprintf(stderr,"	output= %s\n",message);
		fprintf(stderr,"	minfreq= %g\n",minfreq);
		fprintf(stderr,"	maxfreq= %g\n",maxfreq);
		fprintf(stderr,"	index_min= %d\n",indexa);
		fprintf(stderr,"	index_max= %d\n",indexb);
		fprintf(stderr,"	tapers= %d\n",settaper);
		fprintf(stderr,"	smoothing= %d\n",setgauss);
		fprintf(stderr,"	units= %s\n",message);
		fprintf(stderr,"	input_length= %ld samples\n",nn);
		fprintf(stderr,"	sample_rate= %g\n",setsfreq);
		fprintf(stderr,"	duration= %g seconds\n",(nn/setsfreq));
		// calculate window overlap
		a=100*(1.0-1.0/setstep);
		fprintf(stderr,"	fft_window= %d samples\n",setnwin);
		fprintf(stderr,"	fft_overlap= %g %%\n",a);
		fprintf(stderr,"	fft_step= %d \n",setstep);
		fprintf(stderr,"	blocktot= %ld\n",blocktot);
		fprintf(stderr,"	windows_per_block= %ld\n",wintot);
		fprintf(stderr,"	frequency_resolution= %g Hz\n",freqres);
		fprintf(stderr,"	temporal_resolution= %g seconds\n",(partnwin/setsfreq));
		fprintf(stderr,"	temporal_shift= %g seconds\n",((setnwin/2.0)/setsfreq));
		fprintf(stderr,"\n");
	}


	/********************************************************************************/
	/* FREE MEMORY AND EXIT */
	/********************************************************************************/
END1:
	if(data!=NULL) free(data);
	if(buff2!=NULL) free(buff2);
	if(spect!=NULL) free(spect);
	if(spectmean!=NULL) free(spectmean);
	if(spectmean2!=NULL) free(spectmean2);
	if(taper!=NULL) free(taper);
	if(windex!=NULL) free(windex);
	if(blockstart!=NULL) free(blockstart);
	if(blockstop!=NULL) free(blockstop);
	if(cfgr!=NULL) free(cfgr);
	if(freq!=NULL) free(freq);

	exit(stat);

}
