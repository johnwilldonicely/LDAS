#define thisprog "xe-hist1"
#define TITLE_STRING thisprog" v 14: 9.November.2018 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>signal_processing stats</TAGS>

v 14: 9.November.2018 [JRH]
	- update variable name conventions
	- resolve some "uninitialized variable" warnings

v 14: 10.October.2016 [JRH]
	- bugfix: adjustment for labelling start-mid-end of bin needed to be double, not float

v 14: 30.August.2016 [JRH]
	- bugfix - need to set setbintot to zero if setwidth>0, but AFTER check for valid setbintot value

v 14: 6.July.2015 [JRH]
	- minor correction to instructions

v 13: 14.May.2013 [JRH]
	- bugfix: now report error if there is no input, unless a range is manually set

v 12: 14.August.2012 [JRH]
	- bugfix - replace use of fscanf with safer fgets, which limits the number of characters read

v 11: 18.July.2012 [JRH]
	- bugfix: fixed error when scanning "-" or "." to a number - previously this failed but the character was still read. Replace with 2-step read - fisrst scan to string, then scan to number
	- minor bugfix: -w option overrides -b now REGARDLESS of the order the arguments appear on the command line

v 10: 1.May.2012 [JRH]
	- minor bugfix in instructions

v 1.9: 31.March.2012 [JRH]
	- new argument -label allows setting the label for the x-bins (eg. start of bin instead of mind-point)

v 1.8: 24 August 2011 [JRH]
	- replace older "hux" histogram function with xf_hist1d
	- upgrade variables i,j,k, & n to long integers

v 1.7: JRH, 16 May 2011
	- bugfix: now ignores NaN and Inf

v.1.6: bugfix: now deals with datasets where all values are the same - creates a fake range if -l and -h have not been specified

v.1.5: bugfix: histograms now include values which match arguments "min" and "max"

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* external functions start */
long xf_hist1d(double *data, long n, double *histx, double *histy, int bintot, double min, double max, int format);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char *infile,line[MAXLINELEN];
	int w,x,y,z;
	long ii,jj,kk,nn;
	int sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *count,bin,bintot=0,setrange=0,outofrange=0;
	double *raw=NULL,*histx=NULL,*histy=NULL,min=0.0,max=0.0,range,binwidth,plotedge,inv_binwidth;
	/* arguments */
	int settype=1,setbinlabel=2,setbintot=25;
	double setlow=0.0,sethigh=0.0,setbinwidth=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Produces the values for a histogram\n");
		fprintf(stderr,"Input is a series of numbers - ideally one row or one column\n");
		fprintf(stderr,"Non-numeric and non-normal values (Nan and Inf) will be ignored\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\", reads all columns & rows\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []:\n");
		fprintf(stderr,"	-t(ype): 1(counts) 2(range 0-1) or 3(probability) [%d]\n",settype);
		fprintf(stderr,"	-b(ars) in histogram [%d]\n",setbintot);
		fprintf(stderr,"	-w(idth) of each bar, overrides \"-b\" [unset]\n");
		fprintf(stderr,"	-min sets bottom end of histogram scale [unset]\n");
		fprintf(stderr,"	-max sets upper end of histogram scale [unset]\n");
		fprintf(stderr,"	-label: bin-labels identify start(1) middle(2) or end(3) [%d]\n",setbinlabel);
		fprintf(stderr,"NOTE:\n");
		fprintf(stderr,"	- default outputs values for the middle of each bin\n");
		fprintf(stderr,"	- for integers this may produce seemingly unusual results\n");
		fprintf(stderr,"	- eg. for numbers 1-4, setting -w 1 produces 3 bins:\n");
		fprintf(stderr,"		1-2,2-3 & 3-4, and bin-labels 1.5,2.5 & 3.5\n");
		fprintf(stderr,"	- for integer-bins use -w 1 -max [max+1] -label 1 (e.g.below)\n");
		fprintf(stderr,"	\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	- decimal numbers of potential range 0-100:\n");
		fprintf(stderr,"		%s data.txt -min 0 -max 100\n",thisprog);
		fprintf(stderr,"	- integers ranging from 1-4:\n");
		fprintf(stderr,"		%s data.txt -w 1 -max 5 -label 1\n",thisprog);
		fprintf(stderr,"	- piping data to the program:\n");
		fprintf(stderr,"		cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: bin-label\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-t")==0)     settype= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0)     setbintot= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-label")==0) setbinlabel= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)     setbinwidth= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)   { setlow=1; min= atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-max")==0)   { sethigh=1; max= atof(argv[++ii]); }
			else {fprintf(stderr,"\a\n\t--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}


	if(settype<1||settype>3) {fprintf(stderr,"\a\n\t--- Error[%s]: invalid histogram type (-t %d) - must be 1,2 or 3\n\n",thisprog,settype);exit(1);}
	if(setbintot<1) {fprintf(stderr,"\a\n\t--- Error[%s]: invalid bin-total (-b %ld) - must be >1\n\n",thisprog,setbintot);exit(1);}
	if(setlow!=0 && sethigh!=0 && min>=max) {fprintf(stderr,"\a\n\t--- Error[%s]: minimum (-min %g) is not less than maximum (-max %g)\n\n",thisprog,min,max);exit(1);}
	if(setbinlabel<1||setbinlabel>3) {fprintf(stderr,"\a\n\t--- Error[%s]: -label (%d) invalid - must be 1,2 or 3\n\n",thisprog,setbinlabel);exit(1);}
	if(setbinwidth>0.0) setbintot=0;

	/* STORE RAW DATA - FROM FILE OR STANDARD-INPUT IF FILENAME=STDIN */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\a\n\t--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(sscanf(line,"%lf",&aa)==1 && isfinite(aa)) {
			raw= realloc(raw,(nn+1)*sizeofdouble);
			if(raw==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			raw[nn++]= aa;
	}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/* DETERMINE MIN AND MAX VALUES IF SETLOW=0 or SETHIGH=0 - DEAL WITH DATA OUT OF RANGE ETC */
	if(setlow==0 && sethigh==0) {
		if(nn<1) {fprintf(stderr,"\n--- Error[%s]: no input data - must manually set range if output is to be produced\n\n",thisprog);exit(1);};
		min=max=raw[0]; for(ii=0;ii<nn;ii++) {if(raw[ii]<min) min=raw[ii]; if(raw[ii]>max) max=raw[ii];}
		// deal with the possibility that all values are the same - create a fake range to fill
		if(min==max) {aa=raw[0]; if(aa==0.0) {min=aa-1.0;max=aa+1.0;} else {min=0.5*aa;max=1.5*aa;}}
	}
	else if(setlow==0)  { // max is set - determine appropriate minimum
		aa=raw[0];
		min=aa; for(ii=0;ii<nn;ii++) {if(raw[ii]<min) min=raw[ii];}
		if(min==max) {if(aa==0.0) {min=aa-1.0;} else {min=0.5*aa;}}
		if(min>max) {if(max==0.0) {min=max-1.0;} else {min=0.5*max;}}
	}
	else if(sethigh==0) { // min is set - determine approproate maximum
		aa=raw[0];
		max=aa; for(ii=0;ii<nn;ii++) {if(raw[ii]>max) max=raw[ii];}
		if(min==max) {if(aa==0.0) {max=aa+1.0;} else {max=1.5*aa;}}
		if(min>max) {if(min==0.0) {max=min+1.0;} else {max=1.5*min;}}
	}

	// NOW SET RANGE - THIS COMES AFTER ALL DECISIONS ABOUT AUTO OR PRESET MIN AND MAX
	range=max-min;

	/* CALCULATE TOTAL NUMBER OF BINS */
	if(setbintot>0) bintot=setbintot;
	else if(setbinwidth>0) { // if a specific binwidth is called for...
		aa=range/setbinwidth; // calculate exact (possibly non-integer) number of bins
		bb=aa-1.0*(int)(range/setbinwidth); // calculate remainder
		bintot=(int)aa;	// first assume that aa was an integer (the modulus, bb, would be zero)
		if(bb!=0) { // if not then get then add a bin and adjust the range accordingly
			bintot++; cc=setbinwidth/2.0; min-=cc; max+=cc; range=max-min;
		}
	}


	/* ASSIGN MEMORY FOR HISTOGRAM */
	histx= calloc((bintot+1),sizeofdouble); if(histx==NULL) {fprintf(stderr,"\a\n\t--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	histy= calloc((bintot+1),sizeofdouble); if(histy==NULL) {fprintf(stderr,"\a\n\t--- Error[%s]: insufficient memory\n",thisprog);exit(1);}

	/* FILL THE HISTOGRAM */
	kk = xf_hist1d(raw,nn,histx,histy,bintot,min,max,settype);

	/* ALTER THE LABELS IF REQUIRED */
	aa= range/(bintot*2); // determine the half-width of the bins
	if(setbinlabel==1) for(ii=0;ii<bintot;ii++) histx[ii]-=aa;
	if(setbinlabel==3) for(ii=0;ii<bintot;ii++) histx[ii]+=aa;

	/* OUTPUT HISTOGRAM VALUES */
	for(ii=0;ii<bintot;ii++) printf("%g	%g\n",histx[ii],histy[ii]);

	if(raw!=NULL) free(raw);
	if(histx!=NULL) free(histx);
	if(histy!=NULL) free(histy);
	exit(0);
}
