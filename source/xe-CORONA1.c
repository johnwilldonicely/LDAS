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
	long ii,jj,kk,nn,maxlinelen=0;
	float a,b,c;
	double aa,bb,cc;
	FILE *fpin,*fpout;
	/* program-specific variables */
	int sizeofdeaths1;
	long *iword=NULL,nwords;
	double *days1=NULL,*deaths1=NULL,result_d[8];
	long *deaths2=NULL;
	/* arguments */
	char *infile=NULL,*setcountry=NULL;
	int setverb=0;

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
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}


// ??? SETUP FOR IDENTIFYING KEY-COLUMNS - could be put in a function
long *ikeys=NULL,nkeys,*keycols=NULL;
char keys[]="Country,Date,Cases,Deaths,GeoID,Code,Pop,Continent\0";

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=nkeys=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

printf("%s\n",line);

		/* assign keycol numbers: requires *line,*iword,nwords,pword  ...and... *keys,*ikeys,nkeys*,keycol[nkeys] */
		if(nn==0) {
			keycols= xf_getkeycol(line,"\t",keys,",",&nkeys,message);
			if(keycols==NULL) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
			//TEST for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld\n",ii,keycols[ii]);exit(0);
		}

		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		/* select on Country */
		if(strstr(setcountry,line+iword[keycols[0]])!=0) { nn++; continue; }

//printf("%ld,%s,%s\n",keycols[0],setcountry,line+iword[keycols[0]]);

		/* load deaths */
		if(sscanf(line+iword[keycols[2]],"%lf",&aa)!=1) aa=NAN;
		deaths1= realloc(deaths1,(nn+1)*sizeofdeaths1);
		if(deaths1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		deaths1[nn]= aa;

		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST	for(ii=0;ii<nn;ii++) printf("deaths1[%ld]= %g\n",ii,deaths1[ii]);
	//TEST	for(ii=0;ii<nkeys;ii++) printf("%ld: key=%s keycols=%ld\n",ii,keys+ikeys[ii],keycols[ii]);exit(0);


	days1= calloc(nn,sizeof(*days1));
	if(days1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	deaths2= calloc(nn,sizeof(*deaths2));
	if(deaths2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	for(ii=0;ii<nn;ii++) days1[ii]= (double)ii;

	/********************************************************************************/
	/* GET THE CUMULATIVE DEATHS */
	/********************************************************************************/
	for(ii=kk=0;ii<nn;ii++) if(isfinite(deaths1[ii])) {kk+= (long)deaths1[ii]; deaths2[ii]=kk;}
	//TEST	for(ii=0;ii<nn;ii++) printf("deaths2[%ld]= %ld\n",ii,deaths2[ii]);



	/********************************************************************************/
	/* APPLY THE FILTER  */
	/********************************************************************************/

	for(ii=0;ii<nn;ii++) printf("deaths1[%ld]= %ld\n",ii,deaths2[ii]);

	z= xf_filter_bworth1_d(deaths1, nn, 7.0, 0, 0.5, sqrt(2), message);
	//TEST
	for(ii=0;ii<nn;ii++) printf("deaths1_filt[%ld]= %ld\n",ii,deaths2[ii]);

	/********************************************************************************/
	/* FIND THE PEAK */
	/********************************************************************************/
long ipeak=-1;
double dpeak= deaths1[0];

	for(ii=0;ii<nn;ii++) if(deaths1[ii]>dpeak) { dpeak=deaths1[ii]; ipeak=ii; }

	/********************************************************************************/
	/* IDENTIFY THE RELEVANT CHUNK OF DATA BETWEEN setdeathmin and setdaymax */
	/********************************************************************************/
long setdeathmin=0,setdaymax=10,istart=-1,istop=-1;

	kk= ipeak+setdaymax+1;
	for(ii=0;ii<nn;ii++) {
		if(istart<0 && deaths2[ii]>=setdeathmin) istart=ii;
		if(ii<=kk) istop=ii;
	}
	//TEST 	for(ii=0;ii<nn;ii++) printf("%ld\t%g\t%ld\n",ii,deaths1[ii],deaths2[ii]);
	//TEST 	for(ii=istart;ii<istop;ii++) printf("%ld\t%g\t%ld\n",ii,deaths1[ii],deaths2[ii]);
fprintf(stderr,"%ld_%g\n",ipeak,dpeak);

	/********************************************************************************/
	/* CALCULATE THE START SLOPE  */
	/********************************************************************************/
double *pdays1= (days1+istart);
long *pdeaths2= (deaths2+istart);
double *pdeaths1= (deaths1+istart);
kk=(1+ipeak-istart);
for(ii=0;ii<kk;ii++) printf("%g\t%g\t%ld\n",pdays1[ii],pdeaths1[ii],pdeaths2[ii]);
printf("istart=%ld\n",istart);

	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
double intercept1= result_d[0];
double slope1= result_d[1];


	/********************************************************************************/
	/* CALCULATE THE START SLOPE  */
	/********************************************************************************/
pdays1= (days1+ipeak);
pdeaths2= (deaths2+ipeak);
pdeaths1= (deaths1+ipeak);
kk=(1+setdaymax); //??? need to test for out of bounds
for(ii=0;ii<kk;ii++) printf("%g\t%g\t%ld\n",pdays1[ii],pdeaths1[ii],pdeaths2[ii]);

	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
double intercept2= result_d[0];
double slope2= result_d[1];

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
