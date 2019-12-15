#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define thisprog "xe-density1"
#define TITLE_STRING thisprog " v2: 30.September.2018 [JRH]"
#define MAXLINELEN 1000


/*
<TAGS>signal_processing</TAGS>

v2: 30.September.2018 [JRH]
	- bugfix treament of last bin
	- simplify code, update variable names and *alloc conventions

v 29.November.2016 [JRH]
	- bugfix: output format 2 was not calculating average (identical to output format 1, counts)

v 1: 5.April.2016 [JRH]
	- add check to bin-range for fixed-width bins to ensure that last bin is not undersampled
		- adjusts counts in the last bin to compensate for adjusted bin-size

v 1: 14.February.2016 [JRH]
	- drop the use of nextafter() in calculation of binsize
		- leads to peculiar allocation to bins - does not preserve an exact zero
		- was there ever a really good reason for this?

v 1: 12.January.2016 [JRH]
	- used nexafter() function instead of 0.999999 in division
	- clarified instructions

v 1: 8.January.2012 [JRH]
*/


/* external functions start */
void xf_norm1_d(double *data,long N,int normtype);
int xf_smoothgaussd(double *original,long arraysize,int smooth);
/* external functions end */

int main(int argc, char *argv[]) {

	char *infile,line[MAXLINELEN];
	long ii,jj,kk,bin,nn=0,binmax,*densitycount=NULL;
	int x,y,z,sizeofdouble=sizeof(double);
	float a,b,c;
	double aa,bb,cc,dd;
	double *xdata=NULL,*ydata=NULL,*density=NULL;
	double min,max,range;
	double binwidth,binwidth_inv;

	/* arguments */
	int setnorm=-1,setformat=1,setout=1,setxsmooth=0,setverb=0;
	long setbintot=100;
	double setmin=NAN,setmax=NAN,setbinwidth=-1.0,settrim=0.0;
	FILE *fpin;


	if(argc<2) 	{
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate the density (binned sums) in a 1-dimensional data series\n");
		fprintf(stderr,"USAGE: %s [input] [options] \n",thisprog);
		fprintf(stderr,"	[input]: input file or \"stdin\")\n");
		fprintf(stderr,"VALID OPTIONS (defaulst in []):\n");
		fprintf(stderr,"	-f : input format: 1=<time>  2=<time><value> [%d]\n",setformat);
		fprintf(stderr,"		1: density= total lines for each <time>\n");
		fprintf(stderr,"		    missing values result in zero\n");
		fprintf(stderr,"		2: density= sum|mean of <values> for each <time>\n");
		fprintf(stderr,"		    missing values result in NAN\n");
		fprintf(stderr,"	-b : total number of bins [%ld]\n",setbintot);
		fprintf(stderr,"	-w : binwidth (overrides -b) in units of <time> (-1=auto) [%g]\n",setbinwidth);
		fprintf(stderr,"	-n : normalize data: -1=no, 0=0-1 range, 1=z-scores[%d]\n",setnorm);
		fprintf(stderr,"	-s : Gaussian smoothing factor (samples) to apply to bins [%d]\n",setxsmooth);
		fprintf(stderr,"	-min : minimum time-value to include [unset]\n");
		fprintf(stderr,"	-max : maximum time-value to include [unset]\n");
		fprintf(stderr,"	-trim : trim-threshold (0-1) for last bin (0=NONE) [%g]\n",settrim);
		fprintf(stderr,"	    - i.e. proportion of last bin at which the highest time\n");
		fprintf(stderr,"	      must be, to avoid rejecting this bin. The need can arise\n");
		fprintf(stderr,"	      because acquired data often includes a time-stamp at a\n");
		fprintf(stderr,"	      cutoff duration (eg. 60sec). If the binsize is 1sec,\n");
		fprintf(stderr,"	      the bin starting at 60sec will be undersampled and\n");
		fprintf(stderr,"	      should probably not be included\n");
		fprintf(stderr,"	-out : output (1=count, 2=count/unit-time)1 [%d]\n",setout);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLE: %s temp.txt -min 0 -w 1 -trim 0.75\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* ASSIGN ARGUMENTS TO VARIABLES */
	infile=argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-f")==0)     setformat= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0)     setbintot= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-w")==0)     setbinwidth= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0)     setnorm= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0)     setxsmooth= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)   setmin= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-max")==0)   setmax= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-trim")==0)  settrim= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setformat!=1&&setformat!=2) {fprintf(stderr,"\n--- Error[%s]: invalid -f (%d) - must be 1 or 2\n\n",thisprog,setformat);exit(1);}
	if(setmin>=setmax) {fprintf(stderr,"\n--- Error[%s]: -min (%g) must be less than -max (%g)\n\n",thisprog,setmin,setmax);exit(1);}
	if(setbintot<1){fprintf(stderr,"\n--- Error[%s]: -b (%ld) must be >0\n\n",thisprog,setbintot);exit(1);}
	if(settrim<0 || settrim>=1) { fprintf(stderr,"\n--- Error [%s]: invalid -trim [%g] must be >=0 and <1\n\n",thisprog,settrim);exit(1);}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}

	/********************************************************************************
	STORE THE DATA
	********************************************************************************/
	nn=0;
	/* temporarily define limits for data-read */
	if(isfinite(setmin)) min= setmin; else min= -DBL_MAX;
	if(isfinite(setmax)) max= setmax; else max=  DBL_MAX;
	/* open the file */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	if(setformat==1) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf",&aa)!=1) continue;
			if(isfinite(aa) && isfinite(bb)) {
				if(aa<min) continue;
				if(aa>max) continue;
				xdata= realloc(xdata,(nn+1)*sizeofdouble);
				if(xdata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				xdata[nn]= aa;
				nn++;
	}}}
	if(setformat==2) {
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%lf %lf",&aa,&bb)!=2) continue;
			if(isfinite(aa) && isfinite(bb) && isfinite(cc)) {
				if(aa<min) continue;
				if(aa>max) continue;
				xdata= realloc(xdata,(nn+1)*sizeofdouble); if(xdata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				ydata= realloc(ydata,(nn+1)*sizeofdouble); if(ydata==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				xdata[nn]= aa;
				ydata[nn]= bb;
				nn++;
	}}}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn<1) {fprintf(stderr,"\n--- Error[%s]: input (%s) is empty, non-numeric, or fails to meet min/max criteria\n\n",thisprog,infile);exit(1);}
	if(setverb==999) {
		if(setformat==1) for(ii=0;ii<nn;ii++) printf("%ld\t%g\n",ii,xdata[ii]);
		if(setformat==2) for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%g\n",ii,xdata[ii],ydata[ii]);
	}

	/********************************************************************************
	DETERMINE min,max (IF NOT MANUALLY SET) & range
	********************************************************************************/
	if(!isfinite(setmin)) {
		min= xdata[0];
		for(ii=1;ii<nn;ii++) if(xdata[ii]<min) min= xdata[ii];
	}
	if(!isfinite(setmax)) {
		max= xdata[0];
		for(ii=1;ii<nn;ii++) if(xdata[ii]>max) max= xdata[ii];
		max= nextafter(max,DBL_MAX);
	}
	range= max-min;
	if(setverb==999) fprintf(stderr,"min= %.20f\nmax= %.20f\nrange= %.20f\n",min,max,range);

	/********************************************************************************
	DETERMINE BIN-WIDTH AND BINTOT
	********************************************************************************/
	if(setbinwidth>0.0) {
		binwidth= setbinwidth; // use the user-defined bin-width
		binmax= (long)(range/binwidth); // highest bin-number
		setbintot= binmax+1; // total bins
		/* reject the last bin if the max value is not at sufficiently through it (bin will be under-sampled) */
		if(settrim>0) {
			aa= nextafter(setbintot*binwidth,-DBL_MAX); // lowest possible value in last bin
			bb= aa - ((1.0-settrim)*binwidth); // threshold for inclusion of samples in last bin
			if(setverb==999) fprintf(stderr,"settrim= %g\nbinwidth=%.20f\nlowestpossible=%.20f\nthreshold=%.20f\n",settrim,binwidth,aa,bb);

			if(range<bb) { binmax--; setbintot--; }
		}
	}
	else {
		binwidth = range/(double)setbintot;
		binmax= setbintot-1;
	}
	binwidth_inv= 1.0/binwidth; /* avoids double float division below - should speed things up a bit */
	if(setverb==999) {fprintf(stderr,"binmax= %ld\nbintot= %ld\nbinwidth_inv=%.20f\n",binmax,setbintot,binwidth_inv);}

	/********************************************************************************
	BUILD THE DENSITY ARRAY
	********************************************************************************/
	density= calloc((setbintot+1),sizeof(*density));
	densitycount= calloc((setbintot+1),sizeof(*densitycount));
	if(density==NULL || densitycount==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
	if(setformat==1) {
		for(ii=0;ii<nn;ii++) {
			aa= xdata[ii];
			bin= (long)((aa-min)*binwidth_inv);
			density[bin]++;
			densitycount[bin]++;
			//if(setverb==999) fprintf(stderr,"xdata[%ld]=%.20f  bin=%ld\n",ii,aa,bin);
	}}
	if(setformat==2) {
		for(ii=0;ii<nn;ii++) {
			bin= (long)((xdata[ii]-min)*binwidth_inv);
			density[bin]+= ydata[ii];
			densitycount[bin]++;
		}
		/* convert density-counts to the density-average per bin */
		if(setout==2){
			for(ii=0;ii<setbintot;ii++) {
				if(densitycount[ii]>0) density[ii]= density[ii]/densitycount[ii];
				else density[ii]=0;
			}
		}
	}

	/* SMOOTH IF REQUIRED */
	if(setxsmooth>0) {
		z= xf_smoothgaussd(density,setbintot,setxsmooth);
		if(z!=0) {fprintf(stderr,"\n--- Error[%s]: xf_smoothgaussd function encountered insufficient memory\n\n",thisprog);exit(1);}
	}

	/* NORMALISE IF REQUIRED */
	if(setnorm==0 || setnorm==1) xf_norm1_d(density,setbintot,setnorm);

	/* PRINT RESULTS TO STDOUT */
	aa= min;
	for(ii=0;ii<setbintot;ii++) {
		printf("%.12g\t%f\n",aa,density[ii]);
		aa+= binwidth;
	}

	/* CLEAN UP */
	free(xdata);
	if(setformat==2) free(ydata);
	free(density);
	free(densitycount);
}
