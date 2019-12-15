#define thisprog "xe-histstats1"
#define TITLE_STRING thisprog" v 1.4: 9.November.2018 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
<TAGS>stats</TAGS>

v 1.4: 9.November.2018 [JRH]
	- update variable name conventions
	- resolve some "uninitialized variable" warnings

v 1.4: 20.January.2017 [JRH]
	- split AUC calculation into separate function xf_auc2_d
		- remove "correction" for flat line for this calculation
		- allow definition of offset from line joining ends of the curve

v 1.4: 3.March.2014 [JRH]
	- switch to new function xf_curvestats2_d
		- add output of xmin and ymin for trough-detection
		- more accurate AUC for when y-variance is zero or number of bins=1
	- add capability to de-trend the y-values before calculaing statistics

v 1.3: 19.December.2011 [JRH]
	- use new histogram function xf_histstats3d.c
	- includes proper calculation of area under the curve for discrete samples
	- outputs negative and positive AUC values as well (results for points below and above the curve, separately

v 1.2: 22.August.2011 [JRH]
	- add asymmetry measure to output
	- add option to output on single line
*/

/* external functions start */
int xf_detrend1_d(double *y, size_t nn, double *result_d);
int xf_auc2_d(double *curvex, double *curvey, size_t nn, int ref, double *result ,char *message);
int xf_curvestats2_d(double *histx, double *histy, size_t nn, double *result_d, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],*matchstring=NULL,*pline,*pcol,message[MAXLINELEN];
	long int ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd,result_d[16];
	FILE *fpin,*fpout;
	/* program-specific variables */
	int *count,colx=1,coly=2;
	double *xdat=NULL,*ydat=NULL;
	double xymin,xymax,ymin,ymax,auctot,aucneg,aucpos,median,com,bias;
	/* arguments */
	char *infile;
	int setformat=2, setdetrend=0, setref=0;
	double setxmin=NAN, setxmax=NAN;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate statistics on a histogram\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: 2-column input file or \"stdin\" in format [x],[y]\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-xmin: minimum x-value in histogram for inclusion (unset by default)\n");
		fprintf(stderr,"	-xmax: maximum x-value in histogram for inclusion (unset by default)\n");
		fprintf(stderr,"	-ref: reference for y-values used in AUC calculations [%d]\n",setref);
		fprintf(stderr,"		0: reference to zero (use original y-values)\n");
		fprintf(stderr,"		1: reference to line joining ends of curve\n");
		fprintf(stderr,"	-d: detrend before calculating statistics (0=NO, 1=YES) [%d]\n",setdetrend);
		fprintf(stderr,"	-f: output format (0=line, 1=line+header,2=keywords) [%d]\n",setformat);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	N: number of good x-y pairs\n");
		fprintf(stderr,"	XMIN: x-value corresponding with the lowest y-value\n");
		fprintf(stderr,"	YMIN: the lowest y-value\n");
		fprintf(stderr,"	XMAX: x-value corresponding with the highest y-value\n");
		fprintf(stderr,"	YMAX: the highest y-value\n");
		fprintf(stderr,"	AUC: area under the curve, using polygons formed by connecting values\n");
		fprintf(stderr,"	ANEG: the negative area under the curve\n");
		fprintf(stderr,"	APOS: the positive area under the curve\n");
		fprintf(stderr,"	MEDIAN: the x-value dividing the distributon into two equal halves\n");
		fprintf(stderr,"	COM: centre-of-mass = the avg.observation = sum(x*y) / (sum(y)\n");
		fprintf(stderr,"	BIAS: d2-score for y-values when x is +ive vs. -ive\n");
		fprintf(stderr,"NOTE:\n");
		fprintf(stderr,"	- for MEDIAN & COM, x/y-values are adjusted so all are positive\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-xmin")==0) setxmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xmax")==0) setxmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-ref")==0)  setref=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-d")==0)    setdetrend=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-f")==0)    setformat=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setformat<0||setformat>2) {fprintf(stderr,"\n--- Error[%s]: invalid output format (-f %d) - must be 0 or 1\n\n",thisprog,setformat); exit(1);}
	if(setdetrend!=0 && setdetrend!=1) {fprintf(stderr,"\n--- Error[%s]: detrend setting (-d %d) must be 0 or 1\n\n",thisprog,setdetrend); exit(1);}
	if(setref!=0 && setref!=1) {fprintf(stderr,"\n--- Error[%s]: ref setting (-ref %d) must be 0 or 1\n\n",thisprog,setref); exit(1);}

	/* STORE RAW DATA - SMALL NUMBER OF COLUMNS WITH KNOWN CONTENT */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==colx && sscanf(pcol,"%lf",&aa)==1) colmatch--; // store value - check if input was actually a number
			if(col==coly && sscanf(pcol,"%lf",&bb)==1) colmatch--; // store value - check if input was actually a number
		}
		if(colmatch!=0) continue;
		// ignore NaN, Inf, imaginary numbers
		if(!isfinite(aa)||!isfinite(bb)) continue;
		if(isfinite(setxmin)&&aa<setxmin) continue;
		if(isfinite(setxmax)&&aa>setxmax) continue;
		xdat= realloc(xdat,(nn+1)*sizeofdouble); if(xdat==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		ydat= realloc(ydat,(nn+1)*sizeofdouble); if(ydat==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		xdat[nn]=aa;
		ydat[nn]=bb;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn==0){fprintf(stderr,"\n--- Error[%s]: no valid numerical input\n\n",thisprog);exit(1);}

	// TEST: 	for(ii=0;ii<nn;ii++) printf("x: %g	y:%g\n",xdat[ii],ydat[ii]);exit(0);

	/* DE-TREND THE DATA */
	/* this can help detect peaks/medians etc better when linear trends are present in the histogram */
	if(setdetrend==1 && nn>1) xf_detrend1_d(ydat,(size_t)nn,result_d);

	/* CALCULATE AUC */


	/* CALCULATE THE STATISTICS */
	x= xf_curvestats2_d(xdat,ydat,nn,result_d,message);
	if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
	xymin=result_d[0];
	ymin=result_d[1];
	xymax=result_d[2];
	ymax=result_d[3];
	median=result_d[4];
	com=result_d[5];
	bias=result_d[6];

	x= xf_auc2_d(xdat,ydat,(size_t)nn,setref,result_d,message);
	if(x!=0) {fprintf(stderr,"\n--- Error[%s]: %s\n\n",thisprog,message); exit(1);}
	auctot=result_d[0];
	aucpos=result_d[1];
	aucneg=result_d[2];

	if(setformat==2) {
		printf("\n");
		printf("N %ld\n",nn);
		printf("XMIN %g\n",xymin);
		printf("YMIN %g\n",ymin);
		printf("XMAX %g\n",xymax);
		printf("YMAX %g\n",ymax);
		printf("AUC %g\n",auctot);
		printf("ANEG %g\n",aucneg);
		printf("APOS %g\n",aucpos);
		printf("MEDIAN %g\n",median);
		printf("COM %g\n",com);
		printf("BIAS %g\n",bias);
		printf("\n");
	}
	else {
		if(setformat==1) printf("N	XMIN	YMIN	XMAX	YMAX	AUC ANEG	APOS	MEDIAN	COM BIAS\n");
		printf("%ld	%g	%g	%g	%g	%g	%g	%g	%g	%g	%g\n",
			nn,xymin,ymin,xymax,ymax,auctot,aucneg,aucpos,median,com,bias);
	}

	free(xdat);
	free(ydat);
	exit(0);
	}
