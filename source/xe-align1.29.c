#define thisprog "xe-align1"
#define TITLE_STRING thisprog" v 29: 1.August.2017 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/* <TAGS> dt_block signal_processing transform filter stats </TAGS> */


/*
v 29: 24.October.2017 [JRH]
	- sdthresh applies a threshold for inclusion of individual blocks based on the std.dev in the signal
v 29: 12.October.2017 [JRH]
	- allow any one block to be rejected without exiting with an error
v 29: 3.August.2017 [JRH]
		- bugfix: undo mistake in previous change where sampfreq calculation occurs too late in processing
v 29: 1.August.2017 [JRH]
	- bugfix: npad had been un-initialized, leading to memory corruption when data1 pointer was restored to it's original value before freeing memory, if no padding had been applied i nthe first place
	- added check for indices being in range in loop for start/stop alignment

v 29: 17.July.2017 [JRH]
	- bugfix: calculate normalization value for when index1b=index2 (ie. no presample period) - previously, this caused an error
	- exit with warning if no start signals found

v 29: 27.January.2017 [JRH]
	- switch to assumption that sample-rate is constant when defining indices to presample period, normalization period, and duration for each block
	- add pre-binning calculation of AUC, output to a temp file
	- changed normalization to omit sample-zero from average

v 29: 9.January.2017 [JRH]
	- bugfix decimal precision calculation by xf_precision_d
		- needed to allow max decimal precision due to issues precisely representing floating-point numbers

v 29: 22.September.2016 [JRH]
	- bugfix related to changed behaviour of xf_bin1b_f
		- now returns -1 (not zero) on error
		- return value does not reflect modified item-count - this is updated in the n2 argument

v 29: 8.April.2015 [JRH]
	- allow binning at fractions of a second, but prevent binning at < samplerate
	- switch to zero-aligned binning of data - there will always be a "zero" output time
		- uses function xf_bin3_d, which deals with fractional binning and assumes a constant sample-rate
		- this ensures that most of the timestamps will correspond across data for multipe blocks and across subjects
		- data guaranteed not to exceed -pre and -dur bounds - partial bins are combined with the previous ones
		- allows binning at fractions of a second, but prevent binning at < samplerate
	- sample-rate calculated from interval between last 1000 samples (takes the median)
		- hence inclusion of functions xf_samplefreq1_d and xf_compare1_d
	- switch data from double to float to save memory
	- data is automatically interpolated if non-finite numbers are found
		- NAN and INF are not multiplied by -1 anymore, if -flip is set to 1
	- include option to Butterworth filter data
		- data is de-meaned and padded initially by default
		- mean is added back after filtring only if no other normalization is performed

v 28: 19.February.2014 [JRH]
	- elimintate -verb 2 - no need
	- eliminate output of nblocks and std.dev to temporary file
		- nblocks can be extracted from the stderr output of -verb 1
		- std.dev for the presample period is never used

v 27: 30.January.2014 [JRH]
	- identified that sometimes the rounding function fails to eliminate time output with too-high precision
		- to fix this the "precision" variable has been introduced and the function xf_precision_c
		- the function calculates decimal precision required directly from the value typed for the -bin argument
		- hence there is no conversion to a float to determine precision, and the precision is consequently acurate
		- this precision is then applied at the print stage to ensure output time values are not overly precise

v 26: 22.November.2013 [JRH]
	- include inversion option to flip the signal (-flip)

v 25: 8.November.2013 [JRH]
	- improved rounding function xf_round2_d to round entire array instead of 1 sample at a time
	- set rounding to

v 24: 29.May.2013 [JRH]
	- bugfix: rounding loop (below) used "i" as a counter, affecting outer loop when there is more than one data block  - now fixed

v 23: 15.May.2013 [JRH]
	- include rounding step when binning data - ensures timestamps are multiples of the binning factor
		- previously double-precision calculations meant unusual decimal precision numbers could be produced

v 22: 21.March.2013 [JRH]
	- bugfix - time blocks were consistently ending one sample past the designated time

v 21: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."

v 20: 10.August.2012 [JRH]
	- update to call for function xf_bin2, since a new version of this function does not require as many arguments

v 19: 19.July.2012 [JRH]
	- bugfix: added 0.000000001 to block start and end times to compensate for rounding error with double-precision math

v 18: 5.July.2012 [JRH]
	- bugfix: since v14, -norm 1 and -norm2 had not been functional for presample normalization periods that were not tiny
	- bugfix: -norm 0 now also works properly (though never used)

v 17: 2.May.2012 [JRH]
	- bugfix: fixed-duration blocks used to terminate one time-sample too early
	- bugfix: if stopword AND duration were undefined, the program would take the next comment as a new start signal, regardless of it's content
		- now undefined stop-signal results in copying startword to stopword

v 16: 19.April.2012 [JRH]
	- minor correction to on-screen instructions

v 15: 30.November.2011 [JRH]"
	- new option -pn to set a specific presample period for normalization which may be shorter than the total presample period

v 14: 16.November.2011 [JRH]"
	- bugfix - sometimes a started block was wrongly invalidated by a following blank line
	- now a blank line cancels all previously initiated but unterminated blocks unless duration is set, in which case all start signals generate a block
	- improved warning (not error) reporting for -pre and -dur settings which preclude blocks near the start and end of the file, respectively

v 13: 16.August.2011 [JRH]"
	- added functionality - user can now select first and last blocks to use
	- blocks will be numbered as usual but only used blocks will be reported

v 12: 12.August.2011 [JRH]
	- bugfix: gap-detection from version 11 failed if fixed-duration blocks were specified
		- completely rewrote and tested this section of the code - now ok

v 11: 26.July.2011 [JRH]
	- gaps in the comment file record now trigger invalidation of any preceeding block which had begun

v 10: 20.July.2011 [JRH]
	- bugfix: first sample in binned output array seems to be inaccurate for blocks beyond block-1
		- use distinct counter for altered (binned) array (n3) - not sure if this will resolve
		- also run additional tests on the xf_bin2 function - reset "sum" variable when function is first called - probably fixes problem

v 9: 12.July.2011 [JRH]
	- name-change from lilly-align1 to ldas-align1
	- drop output of pre-sample RMS-power
	- removed declarations of unused functions

v 8: 7.July.2011 [JRH]
	- bugfix - now properly corrected call to xf_rms1

v 7: 6.July.2011 [JRH]
	- bugfix: now passes correct variable for window-size to function xf_rms1

v 6: 13.June.2011 [JRH]
	- bugfix: now prints instructions if comment file is not specified (previously: segmentation fault)
	- option to send verbose report to file

v 5: 26.May.2011 [JRH]
	- output statistics on average pre- and post- power
v 3 [JRH]: 10.May.2011
	- allow two types of normalization:
		1: to the Start -signal datapoint
		2: to the average data value in the pre-sample period
v 2 [JRH]: 9.May.2011
	- add binning function to aligned data
*/

/* external functions start */
long xf_interp3_f(float *data, long ndata);
float *xf_padarray3_f(float *data, long nn, long npad, int type, char *message);
int xf_filter_bworth1_f(float *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
size_t xf_bin1b_f(float *data, size_t *setn, size_t *setz, double setbinsize, char *message);
int xf_precision_d(double number,int max);
double xf_samplefreq1_d(double *time1, long n1, char *message);
int xf_compare1_d(const void *a, const void *b);
int xf_stats2_f(float *data, long n, int varcalc, float *result_f);
double xf_rms1_f(float *input, long nn, char *message);
int xf_auc1_f(float *curvey, long nn, double interval, int ref, double *result ,char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256], outfile[256],line[MAXLINELEN],message[MAXLINELEN],*pline,*pcol;
	long int i,j,k,l,n;
	int v,w,x,y,z,col,colmatch,setcolmatch;
	int sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd,result_d[32];
	FILE *fpin,*fpout;

	/* program-specific variables */
	char comment[MAXLINELEN];
	int previnvalid=0,baddata=0;
	long n2=0,npad=0,rejectcount=0;
	long sampspre=0,sampsnorm=0,sampsdur=0; //sample-numbers for defining pre, normalization, and duration, based on sample-frequency
	long nstart=0,nstop=0,usedblocks=0,index1,index1b,index2,index3; // counters for number of start and stop blocks
	size_t zero; // placeholder for indx marking time zero in each block
	float *data1=NULL,*data2=NULL,sampfreq,sampint,flipval=1.0,sdtotal,sdthresh=0.0,sdblock;
	double *time=NULL,*start=NULL,*stop=NULL;
	double timezero,sum,mean,sumofsq,stddev,unterminated=-1.0,binsize;

	/* arguments */
	char cmtfile[256],startword[266],stopword[256];
	int setnorm=0,setflip=0,setsz=1,setverb=1, setambig=0, setstop=0,setfirst=0,setlast=-1,setprecision=-1;
	float low_freq=0.0,high_freq=0;  // filter cutoffs
	float set_sdthresh=0,set_rmsthresh=0; // thresholds for rejecting blocks based on standard-deviations or RMS power
	double setpre=0.0,setpn=-1.0,setdur=0.0,setbin=0;

	sprintf(startword,"start");
	sprintf(stopword,"stop");
	sprintf(outfile,"temp_%s.txt",thisprog);

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Align data to start/stop signals in a separate file\n");
		fprintf(stderr,"MEMORY_REQUIREMENTS: 4*double (32 bytes) for every <time><data> pair\n");
		fprintf(stderr,"USAGE: %s [input] [cfile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: data file or \"stdin\" in format <time> <data>\n");
		fprintf(stderr,"	[cfile]: comment file of format <time> <comments>\n");
		fprintf(stderr,"VALID OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-low: low frequency limit [%g]\n",low_freq);
		fprintf(stderr,"	-high: high frequency limit [%g]\n",high_freq);
		fprintf(stderr,"	-start: keyword [cfile] signalling start a data block [%s]\n",startword);
		fprintf(stderr,"		NOTE: consecutive starts force insertion of an stop\n");
		fprintf(stderr,"		NOTE: an exact match is not required\n");
		fprintf(stderr,"		e.g.: -start \"A\" will detect \"A_on\" and \"A_off\"\n");
		fprintf(stderr,"	-stop: keyword in comment signalling stop of a data block [%s]\n",stopword);
		fprintf(stderr,"		NOTE: if undefined, blocks run to next start or end of file\n");
		fprintf(stderr,"	-dur: fixed duration of output after each start [0, i.e. unset]\n");
		fprintf(stderr,"		NOTE: if set, this overrides -stop \n");
		fprintf(stderr,"		NOTE: blank lines in [cfile] will invalidate unfinished blocks\n");
		fprintf(stderr,"	-pre: time before block-starts to also include [%g]\n",setpre);
		fprintf(stderr,"	-pn:  time before block start for normalization [default: same as -pre]\n");
		fprintf(stderr,"	-norm: normalization style for each block [%d]\n",setnorm);
		fprintf(stderr,"		0: none\n");
		fprintf(stderr,"		1: subtract sample from time-zero (start)\n");
		fprintf(stderr,"		2: subtract the mean of the -pn period (excludes start)\n");
		fprintf(stderr,"		3: convert to a Z-score based on -pn\n");
		fprintf(stderr,"	-rmshresh: RMS threshold for block-rejection (0=NONE) [%g]\n",set_rmsthresh);
		fprintf(stderr,"	-sdthresh: std.dev. threshold for block-rejection (0=NONE) [%g]\n",set_sdthresh);
		fprintf(stderr,"		+ive: an absolute std.dev threshold\n");
		fprintf(stderr,"		-ive: a multiple of the std.dev spanning all blocks\n");
		fprintf(stderr,"	-first: first block to use (numbered from zero) [%d]\n",setfirst);
		fprintf(stderr,"	-last:  last block to use (-1=to the last one) [%d]\n",setlast);
		fprintf(stderr,"	-sz: sample at -pre becomes zero in output (0=NO 1=YES) [%d]\n",setsz);
		fprintf(stderr,"	-bin: size (secs) of time-bins used to average data (0=none) [%g]\n",setbin);
		fprintf(stderr,"		NOTE: if -pre is not a multiple of -bin, time values \n");
		fprintf(stderr,"		      will be adjusted so that time zero is output,\n");
		fprintf(stderr,"		      but consequently the first aligned time may\n");
		fprintf(stderr,"		      be different from -pre (0-centred time averaging)\n");
		fprintf(stderr,"	-flip: flip data (multiply by -1) (0=NO 1=YES) [%d]\n",setflip);
		fprintf(stderr,"	-verb: verbosity of reporting (0=none,1=report to stderr) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s sub001.dat sub001.cmt -start \"pump on\"\n",thisprog);
		fprintf(stderr,"	cat my.dat | %s stdin temp.cmt -start \"ON\" -dur 5\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- [block] [time-in-block] [datavalue]\n");
		fprintf(stderr,"	- file %s (AUC for each block)\n",outfile);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(cmtfile,"%s",argv[2]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-low")==0)   low_freq=atof(argv[++i]);
			else if(strcmp(argv[i],"-high")==0)  high_freq=atof(argv[++i]);
			else if(strcmp(argv[i],"-start")==0) sprintf(startword,"%s",(argv[++i]));
			else if(strcmp(argv[i],"-stop")==0)  { sprintf(stopword,"%s",(argv[++i])); setstop=1;}
			else if(strcmp(argv[i],"-pre")==0)   setpre=atof(argv[++i]);
			else if(strcmp(argv[i],"-pn")==0)    setpn=atof(argv[++i]);
			else if(strcmp(argv[i],"-sz")==0)    setsz=atoi(argv[++i]);
			else if(strcmp(argv[i],"-dur")==0)   setdur=atof(argv[++i]);
			else if(strcmp(argv[i],"-norm")==0)  setnorm=atoi(argv[++i]);
			else if(strcmp(argv[i],"-bin")==0)   setbin=atof(argv[++i]);
			else if(strcmp(argv[i],"-sdthresh")==0)  set_sdthresh=atof(argv[++i]);
			else if(strcmp(argv[i],"-rmsthresh")==0) set_rmsthresh=atof(argv[++i]);
			else if(strcmp(argv[i],"-first")==0) setfirst=atoi(argv[++i]);
			else if(strcmp(argv[i],"-last")==0)  setlast=atoi(argv[++i]);
			else if(strcmp(argv[i],"-flip")==0)  setflip=atoi(argv[++i]);
			else if(strcmp(argv[i],"-verb")==0)  setverb=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setpn==-1.0) setpn=setpre;
	if(setpn>setpre) {fprintf(stderr,"\n--- Error[%s]: -pn (%g) must not exceed -pre (%g)\n\n",thisprog,setpn,setpre);exit(1);};
	if(setnorm<0||setnorm>3) {fprintf(stderr,"\n--- Error[%s]: -norm (%d) must be 0,1,2 or 3\n\n",thisprog,setnorm);exit(1);};
	if(setsz!=0&&setsz!=1) {fprintf(stderr,"\n--- Error[%s]: -pz (%d) must be 0 or 1\n\n",thisprog,setsz);exit(1);};
	if(setflip!=0&&setflip!=1) {fprintf(stderr,"\n--- Error[%s]: -flip (%d) must be 0 or 1\n\n",thisprog,setflip);exit(1);};
	if(setverb!=0&&setverb!=1) {fprintf(stderr,"\n--- Error[%s]: -verb (%d) must be 0 or 1\n\n",thisprog,setverb);exit(1);};

	/* check if start/stop signals could be the same */
	if(setstop==0) { sprintf(stopword,startword); setambig=1; } /* stopword is undefined but functionally treated as if they are the same */
	if(strstr(startword,stopword)!=NULL) setambig=1; /* stopword is in startword - when you find start, you have also found a stop */
	if(strstr(stopword,startword)!=NULL) setambig=1; /* startword is in stopword - finding start does not guarantee finding stop */

	/* define flip multiplier for input data */
	if(setflip==1) flipval=-1.0;
	else flipval=1.0;

	/******************************************************************************/
	/* STORE THE TIME & DATA VALUES IN MEMORY - FOR SPEED, SEPARATE LOOP IF DATA IS FLIPPED */
	/******************************************************************************/
	n=0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf %f",&aa,&b)!=2) continue;
		time= realloc(time,(n+1)*sizeof(*time));
		data1= realloc(data1,(n+1)*sizeof(*data1));
		if(time==NULL | data1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		time[n]=aa;
		if(!isfinite(b)) {
			data1[n]=b;
			baddata=1;
		}
		else data1[n]= b * flipval;
		n++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/*************************************************************************************************
	ALLOCATE MEMORY FOR TEMPORARY DATA ARRRAY - ALLOW FOR LARGEST POSSIBLE BLOCK SIZE
	*************************************************************************************************/
	data2= realloc(data2,(n*sizeof(*data2)));
	if(data2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/*************************************************************************************************
	INTERPOLATE THE DATA IF NECESSARY
	*************************************************************************************************/
	if(baddata==1) xf_interp3_f(data1,(size_t)n);


	/*************************************************************************************************
	DETERMINE SAMPLING RATE - ASSUMING FIXED SAMPLE INTERVALS
	- also make sure bin-size is appropriate for sample rate
	- set time-output precision based on either the bin size or the sample interval
	*************************************************************************************************/
	k=n; if(n>1000) k=1000;
	sampfreq= xf_samplefreq1_d(time,k,message);
	if(sampfreq==0.0) { fprintf(stderr,"\n*** [%s]: %s \n",thisprog,message); exit(1); }
	sampint= 1.0/sampfreq;
	/* make sure bin factor is not too small */
	if(setbin!=0 && setbin<(1.0/sampfreq)) { fprintf(stderr,"\n--- Error[%s]: bin size %g is too small for input samplerate %g\n",thisprog,setbin,sampfreq); exit(1);}
	/* set precision for the output */
	if(setbin>0.0) setprecision= xf_precision_d(setbin,8);
	else setprecision= xf_precision_d(sampint,8);


	/*************************************************************************************************
	FILTER THE DATA (resonance set to sqrt(2)
	- apply padding
	- apply de-meaning before filtering
	- only re-mean if no normalization is to be used
	*************************************************************************************************/
	npad= 0;
	if(low_freq>0.0 || high_freq>0.0) {

		// de-mean to reduce straight-line artefacts in filtered output
		sum= 0.0;
		for(i=0;i<n;i++) sum+=data1[i];
		mean= sum/(double)n;
		for(i=0;i<n;i++) data1[i]-= mean;

		// pad the data with the mean for the first and last [npad] values - [npad] determined by low-pass setting set to 200, or set to [n]
		npad= 0;
		if(high_freq>0) npad= (size_t)(sampfreq/high_freq);
		if(npad<200) npad= 200;
		if(npad>n) npad= n;
		n2= n+npad+npad;
		data1= xf_padarray3_f(data1,n,npad,3,message);
		if(data1==NULL) {fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);exit(1);}

		// filter padded data
		i= xf_filter_bworth1_f(data1,n2,sampfreq,low_freq,high_freq,1.4142,message);
		if(i<0) { fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message); exit(1);}

		// unpad (shift pointer forward - faster than copying
		// just remember to restore pointer to original position before freeing memory)
		data1= (data1+npad);

		// only re-mean if no normalization is to be used
		if(setnorm==0) for(i=0;i<n;i++) data1[i]+=mean;
	}


	/*************************************************************************************************
	DETERMINE NUMBER OF SAMPLES CORRESPONDING TO THE PRE, PN (NORM) AND DUR SETTINGS
	*************************************************************************************************/
	sampspre=  (long) ((double)sampfreq * setpre);
	sampsnorm= (long) ((double)sampfreq * setpn);
	sampsdur=  (long) ((double)sampfreq * setdur); // this will be zero if setdur is not set


	/*************************************************************************************************/
	/* PRINT PRE-READ REPORT TO STDERR: DISPLAYS ON SCREEN BUT WON'T BE REDIRECTED TO FILES */
	/*************************************************************************************************/
	if(setverb>0) {
		fprintf(stderr,"	on_signal: %s\n",startword);
		if(setstop==1) fprintf(stderr,"	off_signal: %s\n",stopword);
		else fprintf(stderr,"	off_signal: [undefined]\n");
		fprintf(stderr,"	presample: %g\n",setpre);
		fprintf(stderr,"	binning: %g\n",setbin);
		fprintf(stderr,"	low-frequency cutoff: %g\n",low_freq);
		fprintf(stderr,"	high-frequency cutoff: %g\n",high_freq);
		fprintf(stderr,"	flip data: "); if(setflip==0) fprintf(stderr,"no\n"); else fprintf(stderr,"yes\n");
		fprintf(stderr,"	normalization_presample: %g\n",setpn);
		if(setdur>0)   fprintf(stderr,"	block_duration: %g\n",setdur);
		if(setnorm==0) fprintf(stderr,"	normalized: no\n");
		if(setnorm==1) fprintf(stderr,"	normalized: sample-zero\n");
		if(setnorm==2) fprintf(stderr,"	normalized: presample average\n");
		if(setnorm==3) fprintf(stderr,"	normalized: presample z-score\n");
		fprintf(stderr,"	total_samples: %ld\n",n);
		fprintf(stderr,"	use blocks ");
		if(setfirst==-1) fprintf(stderr,"0 to ");
		else fprintf(stderr,"%d to ",setfirst);
		if(setlast==-1) fprintf(stderr,"[end]\n");
		else fprintf(stderr,"%d\n",setlast);
		if(set_sdthresh!=0) fprintf(stderr,"	sd noise-rejection threshold: %g std.dev\n",set_sdthresh);
		if(set_rmsthresh!=0) fprintf(stderr,"	RMS noise-rejection threshold: %g std.dev\n",set_sdthresh);
	}


	/******************************************************************************/
	/* READ THE START/STOP SIGNALS */
	/******************************************************************************/
	if((fpin=fopen(cmtfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,cmtfile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		pline=line; colmatch=2;
		for(col=1;(pcol=strtok(pline,"\t\n"))!=NULL;col++) { // only tabs and newlines can be field separators
			pline=NULL;
			if(col==1) { if(sscanf(pcol,"%lf",&aa)==1) colmatch--; }
			if(col==2) { sprintf(comment,"%s",pcol); colmatch--; }
		}
		//if(colmatch==0) fprintf(stderr,"\n%g %s\n",aa,comment);else fprintf(stderr,"BLANK\n");

		/* CHECK FOR BLANK LINES OR LINES WHICH DON'T CONTAIN A TIME AND A COMMENT - INVALIDATES ANY CURRENTLY RUNNING (UNFINISHED) BLOCK */
		if(colmatch!=0) {
			if(nstart>0 && nstart>nstop && previnvalid==0 && setdur<=0) {
				nstart--;
				nstop=nstart;
				previnvalid=1;
			}
			continue;
		}
		else previnvalid=0;

		/* A. CHECK FOR PRESENCE OF START KEYWORD */
		if(strstr(comment,startword)!=NULL) {

			if((aa-setpre)<time[0])   { fprintf(stderr,"--- Warning[%s]: block-start at time %g cannot accommodate -setpre [%g] - omitted\n",thisprog,aa,setpre); rejectcount++; continue; }
			if((aa+setdur)>time[n-1]) { fprintf(stderr,"--- Warning[%s]: block-start at time %g exceeds data - omitted\n",thisprog,aa); rejectcount++; continue; }

			/* for fixed-duration blocks, always start a new block and set end */
			if(setdur>0) {
				if((start=(double *)realloc(start,(nstart+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				if((stop=(double *)realloc(stop,(nstop+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				start[nstart++]= aa;
				stop[nstop++]= aa+setdur;
				continue;
			}
			/* if previous blocks were concluded normally, start a new block */
			else if(nstart==nstop) {
				//fprintf(stderr,"	new block %d start\n",nstart);
				if((start=(double *)realloc(start,(nstart+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
				start[nstart++]= aa;
				continue;
			}
			/* otherwise, if we are already in an unfinished block...*/
			else if(nstart>nstop) {
				/* if this could also be a stop signal, finish last block and start a new one */
				if(setambig==1) {
					//fprintf(stderr,"	old block %d finish, new block %d start\n",nstop,nstart);
					if((start=(double *)realloc(start,(nstart+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
					if((stop=(double *)realloc(stop,(nstop+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
					start[nstart++]=aa;
					continue;
				}
				/* otherwise just update the time of the current block but don't start a new block */
				else {
					//fprintf(stderr,"	restart old block %d \n",nstart);
					start[(nstart-1)]=aa;
					continue;
				}
			}
		}

		/* B. IF NOT A START-KEYWORD, CHECK FOR PRESENCE OF STOP-KEYWORD, IF BLOCK-DURATION WAS NOT SET */
		if(strstr(comment,stopword)!=NULL && setdur<=0) {

			if((aa)>time[n-1]) { fprintf(stderr,"--- Warning[%s]: block-stop at time %g exceeds data - omitted\n",thisprog,aa); rejectcount++; continue;}

			//fprintf(stderr,"	old block %d finish\n",nstop);
			if((stop=(double *)realloc(stop,(nstop+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			stop[nstop++]=aa;
			if(nstop>nstart) nstop--; /* ignore consecutive block-stop signals or stops before the first start */
		}
	}
	fclose(fpin);
	if(nstart<1) { fprintf(stderr,"--- Warning[%s]: no start-signals matching %s found in comment file %s\n",thisprog,startword,cmtfile); exit(0);}
	/* if a block had started and end of file is reached, invalidate that block */
	if(nstart>nstop) { nstart=nstop; unterminated=start[nstart]; }// save the start-time of what would have been the last block
	/* determine last block to use */
	if(setlast==-1) setlast=(nstart-1);
	//TEST: for(i=0;i<nstart;i++) fprintf(stderr,"%g	%g	%ld	%ld\n",start[i],stop[i],nstart,nstop);
	//TEST: fprintf(stderr,"baddata=%d\n",baddata);

	/*************************************************************************************************
	ESTIMATE THE STANDARD-DEVIATION IN THE SIGNAL SPANNING ALL BLOCKS (INCLUDES TIME IN BETWEEN)
	*************************************************************************************************/
	if(set_sdthresh!=0) {
		if(set_sdthresh>0) sdthresh= set_sdthresh;
		else {
			aa=start[0];
			bb=stop[(nstart-1)];
			for(index2=0;index2<n;index2++) { if(time[index2]>=aa) {k=index2; break; }}
			index1=  index2-sampspre; // start of block (zero-setpre)
			index3=  index2+sampsdur+1;  // stop-sample for block (not included)
			if(index1<0||index1>=n) { fprintf(stderr,"\n\t--- Error [%s]: block %ld pre-index (%ld) is out of range\n\n",thisprog,i,index1); exit(1);}
			if(index3<0||index3>=n) { fprintf(stderr,"\n\t--- Error [%s]: block %ld stop-index (%ld) is out of range\n\n",thisprog,i,index3); exit(1);}
			xf_stats2_f((data1+index1),(sampspre+sampsdur),1,result_f);
			sdtotal= result_f[2];
			sdthresh= -1.0*set_sdthresh*sdtotal;
			if(setverb>0) fprintf(stderr,"	std.dev (all-blocks): %g\n",sdtotal);
		}
		if(setverb>0) fprintf(stderr,"	std.dev rejection-threshold: %g\n",sdthresh);
	}


	/*************************************************************************************************
	ALIGN, NORMALIZE, BIN AND OUTPUT THE DATA
	- read each block - scan entire record for data falling within this block
	- this is the only strategy which allows for the possibility of overlapping blocks
	- we must consider the possibility of overlapping blocks because of the pre-sample and duration settings
	*************************************************************************************************/
	if(setverb>0) fprintf(stderr,"	Blocks used:\n");

	/* open output file for capturing AUC */
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to open file \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	fprintf(fpout,"block\tauc\taucpos\taucneg\n");

	/* for each data block */
	for(i=0;i<nstart;i++) {

		if(i<setfirst) continue;
		else if(i>setlast) break;
		else usedblocks++;

		/* SCAN THE RECORD FOR DATA AT COMMENT INDEX POINTS */
		/* note that typically - but not always, due to numerical imprecision - the number of samples betwen indices should be the same */
		index1=index1b=index2=index3=0;
		/* find index2 (time-zero for this block) */
 		k=-1;
		aa=start[i];
 		for(index2=0;index2<n;index2++) { if(time[index2]>=aa) {k=index2; break; }}
 		if(k==-1) { fprintf(stderr,"\n\t--- Error [%s]: no timestamp >= start[%ld] (%g)\n\n",thisprog,i,aa); exit(1);}
 		/* define other indices */
 		index1=  index2-sampspre;  // start of block (zero-setpre)
 		index1b= index2-sampsnorm; // stop of normalization period (zero-setpn)
 		index3=  index2+sampsdur+1;  // stop-sample for block (not included)
		if(index3>n) index3=n;

		/* CHECK THAT DERIVED INDICES (NOT INDEX2) ALL FALL BETWEEN ZERO AND N-1) */
		if(index1<0||index1>=n) { fprintf(stderr,"\n\t--- Error [%s]: block %ld pre-index (%ld) is out of range\n\n",thisprog,i,index1); exit(1);}
		if(index1b<0||index1b>=n) { fprintf(stderr,"\n\t--- Error [%s]: block %ld normalization-index (%ld) is out of range\n\n",thisprog,i,index1b); exit(1);}
		if(index3<0||index3>=n) { fprintf(stderr,"\n\t--- Error [%s]: block %ld stop-index (%ld) is out of range\n\n",thisprog,i,index3); exit(1);}


		/* NOISE-REJECTION: */
		/* 1. CHECK THAT WHOLE-BLOCK STANDARD DEVIATION THRESHOLD IS NOT EXCEEDED */
		if(set_sdthresh!=0) {
			xf_stats2_f((data1+index1),(sampspre+sampsdur),1,result_f);
			sdblock= result_f[2];
			if(sdblock>sdthresh) {
				fprintf(stderr,"\t--- Warning [%s]: block %ld dropped: std.dev= %g\n",thisprog,i,sdblock);
				rejectcount++;
				continue;
			}
		}
		/* 2. CHECK THAT WHOLE-BLOCK RMS THRESHOLD IS NOT EXCEEDED */
		if(set_rmsthresh!=0) {
			aa= xf_rms1_f((data1+index1),(sampspre+sampsdur),message);
			if(aa>set_rmsthresh) {
				fprintf(stderr,"\t--- Warning [%s]: block %ld dropped: RMS= %g\n",thisprog,i,aa);
				rejectcount++;
				continue;
			}
		}

		/* DEFINE MODIFIER FOR TIMES OUTPUT: ALIGN TO START SIGNAL OR ALIGN TO SIGNAL-MINUS-PRESAMPLE */
		if(setsz==0)  timezero=time[index2]; /* new time-zero - start of block */
		else if(setsz==1) timezero=time[index1]; /* alternatively, zero can begin at the pre-sample timepoint */

		/* CALCULATE MEAN & STD.DEV IN PRE PERIOD. NOTE: THIS DOES NOT INCLUDE ZERO */
		n2=0; sum=0.0; sumofsq=0.0;
		if(index2>index1b) { for(j=index1b;j<index2;j++) { cc=data1[j]; if(isfinite(cc)) { sum+=cc; sumofsq+=cc*cc; n2++; }}}
		else { cc=data1[index2]; if(isfinite(cc)) { sum+=cc; sumofsq+=cc*cc; n2++; }}
		if(n2<1) {
			fprintf(stderr,"\t--- Warning[%s]: no valid data in pre-sample interval for block %ld: invalidated\n",thisprog,i);
			rejectcount++;
			continue;
		}
		else if(sum==0.0) mean=0.0;
		else mean=sum/n2;
		/* calculate standard deviation  */
		if(n2>1) stddev= sqrt((double)((n2*sumofsq-(sum*sum))/(double)(n2*(n2-1))));
		else stddev=1.1;

		/* NORMALIZE AND STORE THE DATA IN A TEMPORARY ARRAYS (DATA2, N2) */
		n2=0;
		if(setnorm==0) { for(j=index1;j<index3;j++) data2[n2++]=data1[j]; }
		if(setnorm==1) {
			cc=data1[index2];
			if(!isfinite(cc)) {
				fprintf(stderr,"\t--- Warning[%s]: invalid data at start of block %ld: block will not be output\n",thisprog,i);
				rejectcount++;
				continue;
			}
			for(j=index1;j<index3;j++) data2[n2++]=data1[j]-cc;
		}
		if(setnorm==2) { for(j=index1;j<index3;j++) data2[n2++]=data1[j]-mean; }
		if(setnorm==3) { for(j=index1;j<index3;j++) data2[n2++]=(data1[j]-mean)/stddev; }

		if(setverb>0) fprintf(stderr,"\t\tblock: %ld	start: %.4f	stop: %.4f	dur: %.4f\n",i,start[i],stop[i],(stop[i]-start[i]));

		/* CALCULATE THE AUC AND WRITE TO FILE */
		x= xf_auc1_f((data2+sampspre),(n2-sampspre),(double)sampint,0,result_d,message);
		if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		else fprintf(fpout,"%ld\t%lf\t%lf\t%lf\n",i,result_d[0],result_d[1],result_d[2]);

		/* BIN THE DATA AND OUTPUT - FORMAT: [block=i] [time] [data] */
		zero=(index2-index1);
		if(setbin>0) {
			binsize=(setbin*sampfreq); // bin-size in samples - may be fractional
			/* bin the data - n2 WILL BE UPDATED */
			x= xf_bin1b_f(data2,&n2,&zero,binsize,message);
			if(x==-1) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message);exit(1);}
			/* output */
			aa=binsize*sampint; // interval between bins, in seconds
			if(zero==0) bb=0.0; else bb=(double)(zero)*(-1.0)*aa; // initial timestamp
			for(j=0;j<n2;j++) { printf("%03ld\t%.*f\t%.4f\n",i,setprecision,bb,data2[j]); bb+=aa; }
		}
		else {
			if(zero==0) bb=0.0; else bb=(double)(zero)*(-1.0)*sampint; // initial timestamp
			for(j=0;j<n2;j++) { printf("%03ld\t%.*f\t%.4f\n",i,setprecision,bb,data2[j]); bb+=sampint; }
		}
	} /* end of block loop for(i=0;i<nstart;i++) */

	/* CLOSE AUC OUTPUT FILE */
	fclose(fpout);
	/* REPORT IF THE LAST BLOCK WAS UNTERMINATED AND THEREFORE DROPPED */
	if(unterminated>=0) fprintf(stderr,"\t--- Warning[%s]: block started at time %g is unterminated and was dropped\n",thisprog,unterminated);
	/* RESTORE POINTER TO BEGINNING OF DATA1 */
	data1= (data1-npad);

	if(setverb>0) {
		fprintf(stderr,"\n");
		fprintf(stderr,"	total_blocks: %ld\n",nstart);
		fprintf(stderr,"	rejected_blocks: %ld\n",rejectcount);
		fprintf(stderr,"	kept_blocks: %ld\n",(nstart-rejectcount));
		fprintf(stderr,"	kept_percent: %.3f\n",(100*(double)(nstart-rejectcount)/(double)nstart));
	}

	/* FREE MEMORY AND EXIT */
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(time!=NULL) free(time);
	if(start!=NULL) free(start);
	if(stop!=NULL) free(stop);

	if(setverb>0) fprintf(stderr,"\n");
	exit(0);

	}
