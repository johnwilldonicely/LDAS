#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-correlate"
#define TITLE_STRING thisprog" v 2: 18.April.2016 [JRH]"
#define MAXLINELEN 1000

/*
<TAGS>stats</TAGS>

CHANGES:

v 2: 17 April.2016 [JRH]
	- major re-write to make this the standard low-memory correlation solution
	- switch to modern error-handling & syntax
	- removed unnecessary legacy functions
	- removed options related to invoking gnuplot

v 1.3: JRH, 26.September.2010
	-added ability to set x- and y-ranges

*/

/* external functions start */
float xf_prob_F(float F,int df1,int df2);
/* external functions end */


int main(int argc, char *argv[]) {

	/* general variables */
	char line[MAXLINELEN],*pline,*pcol,message[MAXLINELEN];
	long ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee, result_d[64];
	FILE *fpin;

	/* program-specific variables */
	long linesread=0,dfa,dfb;
	double xx,yy,b0,b1,SUMx,SUMy,SUMx2,SUMy2,SUMxy,MEANx,MEANy,SSx,SSy,SDx,SDy,SSreg,SSres,SPxy,SE,PIP,r,r2,r2adj,F,prob;


	/* arguments */
	char infile[256];
	int setverb=0;
	long setcolx=1,setcoly=2;
	double invalid=NAN,setxmin=NAN,setxmax=NAN;

	SUMx=SUMy=SUMxy=SUMx2=SUMy2=SSx=SSy=0.00;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Perform Pearson's correlation analysis on an input\n");
		fprintf(stderr,"	- on-the-fly analysis (no memory overhead)\n");
		fprintf(stderr,"	- non-numeric values ignored\n");
		fprintf(stderr,"	- requires at least 4 valid data-points\n");
		fprintf(stderr,"	- \"r\" will be \"0\" for data describing vertical/horizonatal lines\n");
		fprintf(stderr,"	- \"F\" will be \"99\" for nearly-perfect correlations\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [filename] [arguments]\n",thisprog);
		fprintf(stderr,"		[filename]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-cx column containing independent variable [%ld]\n",setcolx);
		fprintf(stderr,"	-cy column containing dependent variable [%ld]\n",setcoly);
		fprintf(stderr,"	-xmin minimum value of independent value to include [%g]\n",setxmin);
		fprintf(stderr,"	-xmax maximum value of independent value to include [%g]\n",setxmax);
		fprintf(stderr,"	-invalid numerical data entries to omit [%g]\n",invalid);
		fprintf(stderr,"	-verb sets verbosity (0=one-line, 1=headered, 2=full report[%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -cx 2 -cy 3 -invalid -1\n",thisprog);
		fprintf(stderr,"	cut -f 2,3 temp.txt | %s stdin\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	-verb 0-1: r,r2,dfa,dfb,F,prob,slope,intercept\n");
		fprintf(stderr,"	-verb 2  : verbose summary\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/********************************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-cx")==0)      setcolx=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-cy")==0)      setcoly=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-xmin")==0)     setxmin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-xmax")==0)     setxmax=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-invalid")==0) invalid=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)    setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}


	/********************************************************************************/
	/* READ THE DATA - SPECIFIC COLUMNS HOLD DATA - CALCULATE SOME STATS ON THE FLY */
	/********************************************************************************/
	if(setverb==2) {
		printf("\n");
		printf("Data file: %s\n",infile);
		printf("Correlate columns %ld and %ld\n",setcolx,setcoly);
		printf("Treat \"%g\" as an invalid data point\n",invalid);
		printf("-----------------------------------------------------------------\n");
	}
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n*** %s [ERROR: file \"%s\" not found]\n\n",thisprog,infile); exit(1);}

	nn=linesread=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		linesread++;
		if(line[0]=='#') {continue;}
		pline=line; colmatch=2; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n"))!=NULL;col++) {
			pline=NULL;
			if(col==setcolx && sscanf(pcol,"%lf",&xx)==1) colmatch--; // store value - check if input was actually a number
			if(col==setcoly && sscanf(pcol,"%lf",&yy)==1) colmatch--; // store value - check if input was actually a number
		}
		if( colmatch!=0 || !isfinite(xx) || !isfinite(yy) ) { continue; }
		if( isfinite(invalid) ) { if(xx==invalid || yy==invalid) continue; }
		if( (isfinite(setxmin) && xx <setxmin) || (isfinite(setxmax) && xx>setxmax)) { continue;}

		SUMx +=xx;
		SUMy +=yy;
		SUMx2 += xx*xx;
		SUMy2 += yy*yy;
		SUMxy += xx*yy;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/********************************************************************************/
	/* MAKE SURE THERE WERE AT LEAST 4 VALID DATA POINTS   */
	/********************************************************************************/
	if(nn<4) {
		if(setverb==0) printf("-	-	-	-	-	-	-\n");
		else fprintf(stderr,"\n\t*** %s [WARNING: less than 4 valid data points - no correlation possible]\n\n",thisprog);
		exit(0);
	}

	/********************************************************************************/
	/* CALCULATE BASIC STATISTICS  */
	/********************************************************************************/
	MEANx = SUMx/nn;
	MEANy = SUMy/nn;
	SSx   = SUMx2-(pow(SUMx,2)/nn);
	SSy   = SUMy2-(pow(SUMy,2)/nn);
	SPxy  = SUMxy-((SUMx*SUMy)/nn);
	SDx   = sqrt(SSx/(double)(nn-1));
	SDy   = sqrt(SSy/(double)(nn-1));
	dfa   = 1;
	dfb   = nn-2;


	/********************************************************************************/
	/* CALCULATE R AND REGRESSION COEFFICIENTS b1 (SLOPE) AND b0 (INTERCEPT)  */
	/* - apply corrections for vertical or horizontal lines */
	/********************************************************************************/
	/* if data describes horizontal line... */
	if(SSy==0.0) {
		r=0;
		b1=0.0;   // slope
		b0=MEANy; // y-intercept
		fprintf(stderr,"*** %s [WARNING: data defines a horizontal line]\n",thisprog);
	}
	/* if data describes vertical line (or single point) ... */
	else if(SSx==0.0) {
		r=0;
		b1=NAN;
		b0=NAN;
		fprintf(stderr,"*** %s [WARNING: data defines a vertical line]\n",thisprog);
	}
	/* otherwise, if there is variability in both x and y... */
	else {
		r = SPxy/(sqrt(SSx)*sqrt(SSy));
		b1= SPxy/SSx;
		b0= (SUMy/nn)-(b1*(SUMx/nn));
	}

	/* now calculate the remaining principal statistics */
	r2    = r*r;
	r2adj = 1.0-((1.0-r2)*(double)(nn-1)/(double)(nn-2));
	F     = (r2*dfb)/(1-r2);
	SE    = sqrt(SSy/(nn-1))*sqrt((1-r2adj)*((nn-1)/(nn-2)));
	PIP   = 1-sqrt(1-r2);
	SSreg = SSy*r2;
	SSres = SSy*(1-r2);

	/********************************************************************************/
	/* CALCULATE PROBABILITY */
	/********************************************************************************/
	/* do not pass extreme values to the probability function - just assume it's a perfect correlation */
	if(r>=.999999||r<=-.999999) {
		F=99.0;
		prob=0.0;
	}
	/* make sure the F-value is non-zero, there are valid degrees-of-freedom,and there is variance in x */
	else {
		if(F>0 && dfa>0 && dfb>0 && SDx!=0) {
			prob = xf_prob_F(F,dfa,dfb); /* call the external probability function */
		}
		else {
			prob=NAN;
		}
	}


	/********************************************************************************/
	/* PRINT RESULTS */
	/********************************************************************************/
	if(setverb==1) {
		printf("r	r2	dfa	dfb	F	prob	slope	inter\n");
	}
	if(setverb<2) {
		printf("%.4f	%.4f	%ld	%ld	%.4f	%.4f	%g	%g\n",r,r2,dfa,dfb,F,prob,b1,b0);
		}
	if(setverb>=2) {
		printf("Lines read: %ld\n", linesread);
		printf("N = %ld\n",nn);
		printf("X: Mean %g  sum %g	std.dev. %g\n",MEANx,SUMx,SDx);
		printf("Y: Mean %g  sum %g	std.dev. %g\n",MEANy,SUMy,SDy);
		printf("\n");
		printf("SS-regression = %.4f, df = 1\n",SSreg);
		printf("SS-residual   = %.4f, df = %ld\n",SSres,(nn-2));
		printf("SS-total      = %.4f, df = %ld\n",SSy,(nn-1));
		printf("\n");
		printf("Pearson's r= %.4f, r2= %.4f, F(1,%ld)= %.4f, p= %.4f\n",r,r2,(nn-2),F,prob);
		printf("\n");
		printf("Regression equation: Y = %f + %f * x\n",b0,b1);
		printf("\n");
		printf("Adjusted r2 (unbiased estimate of population r2) = %f\n",r2adj);
		printf("Std error of estimate (Root Mean Square Error)   = %.4f\n",SE);
		printf("Proportional improvement in prediction (PIP)     = %f\n",PIP);
		printf("-----------------------------------------------------------------\n");
		printf("\n");
	}



	exit(0);

}
