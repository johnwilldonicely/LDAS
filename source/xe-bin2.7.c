#define thisprog "xe-bin2.c"
#define TITLE_STRING thisprog" v 7: 27.November.2017 [JRH]"
#define MAXLINELEN 1000
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS>stats signal_processing</TAGS>

v 7: 27.November.2017 [JRH]
	- switch to using %f for output to avoid precision loss with exponential notation
v 7: 5.April.2015 [JRH]
	- minor mod to harmonize functions across programs - use xf_bin2 instead of xf_bin1 (virtually identical)
v 6: 14.August.2012 [JRH]
	- bugfix: single non-binned "orphan samples" at the end of the input are now output instead of ignored
	- change behaviour to match xe-bin1 - that is, ALL input lines contribute to an output
		- non-numerical values (except INF) are now converted to NAN instead of the sample being ignored
	- NAN values do not affect output unless all values in a bin become NAN
		- in this case the output for the bin is NAN
	- any INF values in a bin result in the output for the bin being INF, EXCEPT for the minimum value if peak-detection is selected (-p 1)
v 5: 10.August.2012 [JRH]
	- add option to output binned sums instead of means
	- add option to output values corresponding to the end of each time window, in addition to the beginning or middle
	- changed the way mid- or end-bin time output is handled due to simplification of binning functions
		(the functions now output the time at the start of each window by default)
v 4: 17.July.2012 [JRH]
	- bugfix: fixed error when scanning "-" or "." to a number - previously this failed but the character was still read. Replace with 2-step read - fisrst scan to string, then scan to number
v 3: 13.July.2011 [JRH]
	- switched to using xf_binpeak4 for peak detection
		- faster min/max detection rather than deviations from mean in each window
		- outputs start and middle of each time-window rather than duplicating the start-time
v 2: 22.May.2011 [JRH]
	- switched to using xf_binpeak3 for peak detection
	- this will outputmin and max for each window
v 1: 21.May.2011 [JRH]
 	- added option to detect peaks instead of the mean in each window
 /


/* external functions start */
long xf_bin2_d(double *time, double *data , long n, double winwidth, int out);
long xf_binpeak4(double *time, double *data , long n, double winwidth);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[MAXLINELEN],line[MAXLINELEN],word[MAXLINELEN],*pline,*pcol;
	long int i,j,k,n=0;
	int v,w,x,y,z,sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program specific variables */
	long n2=0;
	double sum=0.0,time_zero=0.0,time_start=0.0,time_elapsed=0.0,interval,halfwin,winend;
	double *time=NULL, *data=NULL;
	/* arguments */
	int setmid=0, setpeak=0, setsum=0;
	double winwidth=1.0;

	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Bin data by averaging consecutive samples in non-overlapping windows\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: 2-column data file or stdin in format <time> <value>\n");
		fprintf(stderr,"OPTIONS: defaults in []...\n");
		fprintf(stderr,"	-t width of the time-window [%g]\n",winwidth);
		fprintf(stderr,"	-m output time at start of each window (0) middle(1) or end(2) [%d])\n",setmid);
		fprintf(stderr,"	-p detect peaks (1) instead of the mean (0) in each window [%d])\n",setpeak);
		fprintf(stderr,"		: output 2 points (min, max) per window\n");
		fprintf(stderr,"	-s output sums (1) instead of means (0) [%d])\n",setsum);
		fprintf(stderr,"		: only one of -p or -s can be set to 1\n");
		fprintf(stderr,"NOTES: \n");
		fprintf(stderr,"	: missing or invalid time values will result in inconsistent bin start-times\n");
		fprintf(stderr,"	: all non-numeric data (except INF) are converted to NAN\n");
		fprintf(stderr,"	: non-numeric data do not contribute to bin sums/averages\n");
		fprintf(stderr,"	: if all data in a bin are non-numeric, the output for the bin is NAN\n");
		fprintf(stderr,"	: if any data in a bin are INF, the output for the bin is INF (but see below)\n");
		fprintf(stderr,"	: the minimum output value for peak-detect binning is unaffected by INF\n");
		fprintf(stderr,"EXAMPLE:\n");
		fprintf(stderr,"	cut -f 1,2 data.txt | %s stdin -t 0.5 -m 0 -p 0\n",thisprog);
		fprintf(stderr,"OUTPUT: [time] [average or peak]\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ COMMAND-LINE ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-t")==0) 	{ winwidth=atof(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-m")==0) 	{ setmid=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-p")==0) 	{ setpeak=atoi(argv[i+1]); i++;}
			else if(strcmp(argv[i],"-s")==0) 	{ setsum=atoi(argv[i+1]); i++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(winwidth<=0){fprintf(stderr,"\n--- Error[%s]: averaging window (%g) must be >0\n\n",thisprog,winwidth);exit(1);}
	if(setmid!=0&&setmid!=1&&setmid!=2) {fprintf(stderr,"\n--- Error[%s]: -m (%d) must be 0,1 or 2\n\n",thisprog,setmid);exit(1);}
	if(setpeak!=0&&setpeak!=1) {fprintf(stderr,"\n--- Error[%s]: -p (%d) must be 0 or 1\n\n",thisprog,setpeak);exit(1);}
	if(setsum!=0&&setsum!=1) {fprintf(stderr,"\n--- Error[%s]: -s (%d) must be 0 or 1\n\n",thisprog,setsum);exit(1);}
	if(setsum==1&&setpeak==1) {fprintf(stderr,"\n--- Error[%s]: cannot set both -p and -s to 1\n\n",thisprog);exit(1);}

	halfwin=winwidth/2.0;

	/* STORE DATA - FROM FILE OR STANDARD-INPUT IF FILENAME=STDIN */
	n=0; sum=0.0; time_elapsed=0.0; time_start=0.0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		aa=bb=NAN;
		z=sscanf(line,"%lf %lf",&aa,&bb);

		if(isfinite(aa)) {
			if(z==1) bb=NAN;
			if((time=(double *)realloc(time,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			if((data=(double *)realloc(data,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			time[n]=aa;
			data[n]=bb;
			n++;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(n<1){fprintf(stderr,"\n--- Error[%s]: no valid numerical input\n\n",thisprog);exit(1);}
	if(n==1) {
		if(setmid==0) printf("%f\t%g\n",time[0],data[0]);
		if(setmid==1) printf("%f\t%g\n",(time[0]+halfwin),data[0]);
		if(setmid==2) printf("%f\t%g\n",(time[0]+winwidth),data[0]);
		exit(0);
	}

	//for(i=0;i<n;i++) printf("%g %g\n",time[i],data[i]);

	/*CALCULATE AMOUNT TO ADD TO EACH WINDOW START-TIME TO OUTPUT THE END OF THE WINDOW INSTEAD - THIS ASSUMES REGULAR SAMPLE INTERVALS */
	interval=time[1]-time[0];
	winend=winwidth-interval;
	if(winwidth<interval) winend=0.0;

	/* BIN THE ARRAY AND RECORD THE NEW SHORTER LENGTH (n2) */
	if(setpeak==0 && setsum==0) { n2=xf_bin2_d(time,data,n,winwidth,2); }
	if(setpeak==0 && setsum==1) { n2=xf_bin2_d(time,data,n,winwidth,1); }
	if(setpeak==1) { n2=xf_binpeak4(time,data,n,winwidth); }

	if(n2==-1) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	if(n2==-2) {fprintf(stderr,"\n--- Error[%s]: window is too big for length of dataset\n\n",thisprog);exit(1);}

	/* OUTPUT THE BINNED TIME AND DATA ARRAYS */
	if(setmid==0) for(i=0;i<n2;i++) printf("%f\t%g\n",time[i],data[i]);
	if(setmid==1) for(i=0;i<n2;i++) printf("%f\t%g\n",(time[i]+halfwin),data[i]);
	if(setmid==2) for(i=0;i<n2;i++) printf("%f\t%g\n",(time[i]+winend),data[i]);

	free(time);
	free(data);
	exit(0);

}
