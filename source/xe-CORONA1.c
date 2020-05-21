#define thisprog "xe-CORONA"
#define TITLE_STRING thisprog" 20.May.2020 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define N 20
/*
<TAGS> CORONAVIRUS </TAGS>

v 1: 20.May.2020 [JRH]

*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_getkeycol(char *line1, char *d1, char *keys1, char *d2, long *nkeys1, char *message);
int xf_filter_bworth1_d(double *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_d);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int vector[] = {1,2,3,4,5,6,7},z;
	long ii,jj,kk,nn,maxlinelen=0,nlines;
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char keys[]="Country,Date,Cases,Deaths,GeoID,Code,Pop,Continent\0";
	long *ikeys=NULL,*keycols=NULL,nkeys;

	int sizeofdeaths1;
	long *iword=NULL,*deaths2=NULL,*pdeaths2,nwords,ipeak=-1,setmindeaths=0,setmaxdays=356,istart=-1,istop=-1,overrun;
	double *days1=NULL,*deaths1=NULL,*pdays1,*pdeaths1,dpeak,result_d[8];
	double intercept1,intercept2,slope1,slope2;

	/* arguments */
	char *infile=NULL,*setcountry=NULL;
	int setverb=0;
	double setlow=0,sethigh=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Template program source-code\n");
		fprintf(stderr,"USAGE: %s [in] [country] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[country]: name of country to analyse\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-mindeaths: (start) minimum cumulative deaths [%ld]\n",setmindeaths);
		fprintf(stderr,"	-maxdays: (stop) maximum days post-peak [%ld]\n",setmaxdays);
		fprintf(stderr,"	-low: low frequency limit, 0=NONE [%g]\n",setlow);
		fprintf(stderr,"	-high: high frequency limit, 0=NONE [%g]\n",sethigh);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	- \n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	sizeofdeaths1= sizeof(*deaths1);


	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	setcountry= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-mindeaths")==0)  setmindeaths=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-maxdays")==0) setmaxdays=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)  setlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}


	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	nlines=nn=nkeys=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* assign keycol numbers: requires *line,*iword,nwords,pword  ...and... *keys,*ikeys,nkeys*,keycol[nkeys] */
		if(nlines==0) {
			keycols= xf_getkeycol(line,"\t",keys,",",&nkeys,message);
			if(keycols==NULL) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
			//TEST for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld\n",ii,keycols[ii]);exit(0);
		}

		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		/* select on Country */
		if(strcmp(setcountry,line+iword[keycols[0]])!=0) { nlines++; continue; }
		//TEST CORRECT SELECTION: printf("%ld,%s,%s\n",keycols[0],setcountry,line+iword[keycols[0]]);

		/* load deaths */
		if(sscanf(line+iword[keycols[3]],"%lf",&aa)!=1) aa=NAN;
		deaths1= realloc(deaths1,(nn+1)*sizeofdeaths1);
		if(deaths1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		deaths1[nn]= aa;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: Country \"%s\" not found in \"%s\"inflie\n\n",thisprog,setcountry,infile);exit(1);};

	if(setverb==999) printf("nn= %ld nkeys= %ld\n",nn,nkeys);
	if(setverb==999) printf("keys= %s\n",keys);
	if(setverb==999) for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld\n",ii,keycols[ii]);
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths1[%ld]= %g\n",ii,deaths1[ii]);


	/********************************************************************************/
	/* INITIALISE ARRAYS FOR DAYS (DOUBLE, FOR CORRELATION) and DEATHS2 (LONG, CUMULATIVE) */
	/********************************************************************************/
	days1= calloc(nn,sizeof(*days1));
	if(days1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	deaths2= calloc(nn,sizeof(*deaths2));
	if(deaths2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) days1[ii]= (double)ii;

	/********************************************************************************/
	/* GET THE CUMULATIVE DEATHS */
	/********************************************************************************/
	for(ii=kk=0;ii<nn;ii++) if(isfinite(deaths1[ii])) {kk+= (long)deaths1[ii]; deaths2[ii]=kk;}
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths2[%ld]= %ld\n",ii,deaths2[ii]);

	/********************************************************************************/
	/* APPLY THE FILTER TO THE ENTIRE DATASET */
	/********************************************************************************/
	z= xf_filter_bworth1_d(deaths1,nn,7.0,setlow,sethigh,sqrt(2),message);
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths1_filt[%ld]= %ld\n",ii,deaths2[ii]);

	/********************************************************************************/
	/* FIND THE PEAK */
	/********************************************************************************/
	dpeak= deaths1[0];
	for(ii=0;ii<nn;ii++) if(deaths1[ii]>dpeak) { dpeak=deaths1[ii]; ipeak=ii; }

	/********************************************************************************/
	/* IDENTIFY THE RELEVANT CHUNK OF DATA BETWEEN setmindeaths and setmaxdays */
	/********************************************************************************/
	kk= ipeak+setmaxdays+1;
	for(ii=0;ii<nn;ii++) {
		if(istart<0 && deaths2[ii]>=setmindeaths) istart=ii;
		if(ii<=kk) istop=ii;
	}
	if(setverb==999) fprintf(stderr,"istart: %ld ipeak: %ld dpeak: %g\n",istart,ipeak,dpeak);

	/********************************************************************************/
	/* CALCULATE THE START SLOPE  */
	/********************************************************************************/
	pdays1= (days1+istart);
	pdeaths2= (deaths2+istart);
	pdeaths1= (deaths1+istart);
	kk=(1+ipeak-istart);
	if(setverb==999) for(ii=0;ii<kk;ii++) printf("RISING[%ld]: %g\t%g\t%ld\n",ii,pdays1[ii],pdeaths1[ii],pdeaths2[ii]);
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	intercept1= result_d[0];
	slope1= result_d[1];

	/********************************************************************************/
	/* CALCULATE THE STOP SLOPE  */
	/********************************************************************************/
	pdays1= (days1+ipeak);
	pdeaths2= (deaths2+ipeak);
	pdeaths1= (deaths1+ipeak);

	kk=(setmaxdays+1); // stop-sample (not included)
	overrun= (ipeak+kk)-nn; // number of excess days being requested
	if(overrun>0) kk-= overrun;

	if(setverb==999) for(ii=0;ii<kk;ii++) printf("FALLING[%ld]: %g\t%g\t%ld\n",ii,pdays1[ii],pdeaths1[ii],pdeaths2[ii]);
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	intercept2= result_d[0];
	slope2= result_d[1];


	for(ii=istart;ii<istop;ii++) printf("%ld\t%g\t%ld\n",ii,deaths1[ii],deaths2[ii]);
	for(ii=ii;ii<(nn+overrun);ii++) printf("%ld\tnan\tnan\n",ii);

printf("Y= %g + X * %g\n",intercept1,slope1);
printf("Y= %g + X * %g\n",intercept2,slope2);



	goto END;
	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(deaths1!=NULL) free(deaths1);
	exit(0);
}
