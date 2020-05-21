#define thisprog "xe-CORONA"
#define TITLE_STRING thisprog" 20.May.2020 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define N 20
/*
<TAGS> epidemiology </TAGS>

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
	char outfile[64];
	long *ikeys=NULL,*keycols=NULL,nkeys;

	int sizeofcases0,sizeofdeaths0;
	long *iword=NULL,*cases0=NULL,*deaths0=NULL,*deaths2=NULL,*pdeaths2,nwords,ipeak=-1,setmindeaths=0,setmaxdays=356,istart=-1,istop=-1,cmin,cmax,overrun;
	float *weeks1=NULL;
	double *days1=NULL,*cases1=NULL,*deaths1=NULL,*pdays1,*pdeaths1,dmin,dmax,result_d[8];
	double intercept1,intercept2,slope1,slope2,week;

	/* arguments */
	char *infile=NULL,*setcountry=NULL;
	int setverb=0,setpad=0,setnormc=0,setnormd=0,setout=1;
	double setlow=0,sethigh=0;

	sprintf(outfile,"temp_%s.txt",thisprog);

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Analyse daily cases and deaths in an epidemiological dataset\n");
		fprintf(stderr,"Required input columns: \n");
		fprintf(stderr,"	%s\n",keys);
		fprintf(stderr,"USAGE: %s [in] [country] [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[country]: name of country to analyse\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-mindeaths: (start) minimum cumulative deaths [%ld]\n",setmindeaths);
		fprintf(stderr,"	-maxdays: (stop) maximum days post-peak [%ld]\n",setmaxdays);
		fprintf(stderr,"	-low: low frequency limit, 0=NONE [%g]\n",setlow);
		fprintf(stderr,"	-high: high frequency limit, 0=NONE [%g]\n",sethigh);
		fprintf(stderr,"	-normd: normalise deaths to 0-1 range (0=NO 1=YES) [%d]\n",setnormd);
		fprintf(stderr,"	-normc: normalise cases to 0-1 range (0=NO 1=YES) [%d]\n",setnormc);
		fprintf(stderr,"	-pad: add sample-and-hold data if maxdays exceeds data length (0=NO 1=YES) [%d]\n",setpad);
		fprintf(stderr,"	-out: output format [%d]\n",setout);
		fprintf(stderr,"		1: Day Cases Deaths DeathsSum\n");
		fprintf(stderr,"		2: Var Day Count\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	%s: basic stats on the curves\n",outfile);
		fprintf(stderr,"	stdout: chunk of data: basic stats on the curves\n,outfile");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	setcountry= argv[2];
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-mindeaths")==0)  setmindeaths=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-maxdays")==0) setmaxdays=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)  setlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pad")==0) setpad= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-normc")==0) setnormc= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-normd")==0) setnormd= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setpad!=0 && setpad!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -pad [%d] must be 0 or 1\n\n",thisprog,setpad);exit(1);}
	if(setnormc!=0 && setnormc!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -normc [%d] must be 0 or 1\n\n",thisprog,setnormc);exit(1);}
	if(setnormd!=0 && setnormd!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -normd [%d] must be 0 or 1\n\n",thisprog,setnormd);exit(1);}
	if(setout!=1 && setout!=2) { fprintf(stderr,"\n--- Error [%s]: invalid -out [%d] must be 1 or 2\n\n",thisprog,setout);exit(1);}


	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	nlines=nn=nkeys=0;
	sizeofcases0= sizeof(*cases0);
	sizeofdeaths0= sizeof(*deaths0);
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

		/* load cases */
		if(sscanf(line+iword[keycols[2]],"%ld",&kk)!=1) kk=-1;
		cases0= realloc(cases0,(nn+1)*sizeofcases0);
		if(cases0==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		cases0[nn]= kk;

		/* load deaths */
		if(sscanf(line+iword[keycols[3]],"%ld",&kk)!=1) kk=-1;
		deaths0= realloc(deaths0,(nn+1)*sizeofdeaths0);
		if(deaths0==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		deaths0[nn]= kk;

		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: Country \"%s\" not found in \"%s\"inflie\n\n",thisprog,setcountry,infile);exit(1);};

	if(setverb==999) printf("nn= %ld nkeys= %ld\n",nn,nkeys);
	if(setverb==999) printf("keys= %s\n",keys);
	if(setverb==999) for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld\n",ii,keycols[ii]);
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths0[%ld]= %ld\n",ii,deaths0[ii]);

	/********************************************************************************/
	/* INITIALISE EXTRA ARRAYS  */
	/********************************************************************************/
	/* days: double-version of line-number */
	days1= calloc(nn,sizeof(*days1));
	if(days1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) days1[ii]= (double)ii;

	/* cases: double-version  */
	cases1= calloc(nn,sizeof(*cases1));
	if(cases1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) cases1[ii]= (double)(cases0[ii]);

	/* deaths: double-version  */
	deaths1= calloc(nn,sizeof(*deaths1));
	if(deaths1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) deaths1[ii]= (double)(deaths0[ii]);

	/* cumulative deaths - long integer */
	deaths2= calloc(nn,sizeof(*deaths2));
	if(deaths2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=kk=0;ii<nn;ii++) { if(deaths0[ii]>0) {kk+= deaths0[ii]; deaths2[ii]= kk;} }

	/* weeks-fractions - to help with later plotting  */
	weeks1= calloc(nn,sizeof(*weeks1));
	if(weeks1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths2[%ld]= %ld\n",ii,deaths2[ii]);

	/********************************************************************************/
	/* APPLY THE FILTER TO THE ENTIRE DATASET */
	/********************************************************************************/
	if(setlow!=0||sethigh!=0) {

		z= xf_filter_bworth1_d(cases1,nn,7.0,setlow,sethigh,sqrt(2),message);
		z= xf_filter_bworth1_d(deaths1,nn,7.0,setlow,sethigh,sqrt(2),message);

		if(setverb==999) for(ii=0;ii<nn;ii++) printf("cases1_filt[%ld]= %g\n",ii,cases1[ii]);
		if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths1_filt[%ld]= %g\n",ii,deaths1[ii]);
	}


	/********************************************************************************/
	/* IDENTIFY THE RELEVANT CHUNK OF DATA BETWEEN setmindeaths and setmaxdays */
	/********************************************************************************/
	/* find the start */
	istart= -1;
	for(ii=0;ii<nn;ii++) { if(deaths2[ii]>=setmindeaths) { istart=ii; break; } }
	if(istart==-1) {fprintf(stderr,"\n--- Error[%s]: -mindeaths threshold (%ld) never reached\n\n",thisprog,setmindeaths);exit(1);}
	/* find the peak */
	dmax= deaths1[istart];
	for(ii=istart;ii<nn;ii++) { if(deaths1[ii]>dmax) { dmax=deaths1[ii]; ipeak=ii; } }
	/* find the stop */
	istop= ipeak+setmaxdays+1;
	if(istop>nn) istop= nn;
	overrun= (ipeak+setmaxdays+1)-nn ; // number of excess days being requested

	if(setverb==999) printf("istart= %ld ipeak= %ld istop= %ld\n",istart,ipeak,istop);

	/* find the min/max values within the specified range  */
	cmax=cmin= cases1[istart];
	dmin= deaths1[istart];
	for(ii=istart;ii<istop;ii++) {
		if(deaths1[ii]<dmin) dmin= deaths1[ii];
		if(cases1[ii]>cmax) cmax= cases1[ii];
		if(cases1[ii]<cmin) cmin= cases1[ii];
	}

	if(setverb==999) for(ii=istart;ii<istop;ii++) printf("CHUNK[%ld]: %g\n",ii,deaths1[ii]);

	/********************************************************************************/
	/* BUILD THE WEEKS ARRAY, CENTRED ON THE PEAK */
	/********************************************************************************/
	for(ii=0;ii<nn;ii++) weeks1[ii]= (double)(ii-ipeak)/7.0;
	/* expand memory if required */
	if(overrun>0) {
		kk= nn+overrun;
		weeks1= realloc(weeks1,kk*sizeof(*weeks1));
		if(weeks1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=ii;ii<kk;ii++) weeks1[ii]= (double)(ii-ipeak)/7.0;
	}



	/********************************************************************************/
	/* NORMALISE CASES AND DEATHS */
	/********************************************************************************/
	if(setnormc==1) {
		bb= cmax-cmin;
		if(bb==0.0) {fprintf(stderr,"\n--- Error[%s]: cannot normalise deaths, range=0\n\n",thisprog);exit(1);}
		for(ii=0;ii<nn;ii++) {
			aa= 100 * (cases1[ii]-cmin);
			if(aa!=0.0) cases1[ii]= aa/bb; else cases1[ii]= 0.0;
	}}
	if(setnormd==1) {
		bb= dmax-dmin;
		if(bb==0.0) {fprintf(stderr,"\n--- Error[%s]: cannot normalise deaths, range=0\n\n",thisprog);exit(1);}
		for(ii=0;ii<nn;ii++) {
			aa= 100 * (deaths1[ii]-dmin);
			if(aa!=0.0) deaths1[ii]= aa/bb; else deaths1[ii]= 0.0;
	}}


	/********************************************************************************/
	/* CALCULATE THE START SLOPE  */
	/********************************************************************************/
	pdays1= (days1+istart);
	pdeaths1= (deaths1+istart);
	kk= (ipeak-istart)+1;
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	intercept1= result_d[0];
	slope1= result_d[1];

	if(setverb==999) { pdeaths2= (deaths2+istart); for(ii=0;ii<kk;ii++) printf("RISING[%ld]: %g\t%g\t%ld\n",ii,pdays1[ii],pdeaths1[ii],pdeaths2[ii]); }

	/********************************************************************************/
	/* CALCULATE THE STOP SLOPE  */
	/********************************************************************************/
	pdays1= (days1+ipeak);
	pdeaths1= (deaths1+ipeak);
	kk= (istop-ipeak);
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	intercept2= result_d[0];
	slope2= result_d[1];

	if(setverb==999) for(ii=0;ii<kk;ii++) { pdeaths2= (deaths2+ipeak); printf("FALLING[%ld]: %g\t%g\t%ld\n",ii,pdays1[ii],pdeaths1[ii],pdeaths2[ii]); }


	/********************************************************************************/
	/* OUTPUT  */
	/********************************************************************************/
	/* open output file for saving regression data  */
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to open file \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	fprintf(fpout,"index_start= %ld\n",istart);
	fprintf(fpout,"index_peak= %ld\n",ipeak);
	fprintf(fpout,"index_stop= %ld\n",(istop-1));
	fprintf(fpout,"peak_deaths= %g\n",deaths1[ipeak]);
	fprintf(fpout,"Regression_rising: intercept= %g slope= %g\n",intercept1,slope1);
	fprintf(fpout,"Regression_falling: intercept= %g slope= %g\n",intercept2,slope2);
	fclose(fpout);

	if(setout==1) {
		printf("Day\tCases\tDeaths\tDeathsSum\tDeltaWeeks\n");
		for(ii=istart;ii<istop;ii++) { 	printf("%ld\t%g\t%g\t%ld\t%g\n",ii,cases1[ii],deaths1[ii],deaths2[ii],weeks1[ii]); }
		if(overrun>0 && setpad==1) {
			aa= cases1[nn-1];
			bb= deaths1[nn-1];
			kk= deaths2[nn-1];
			for(ii=ii;ii<(nn+overrun);ii++) { printf("%ld\t%g\t%g\t%ld\t%g\n",ii,aa,bb,kk,weeks1[ii]); }
		}
	}
	if(setout==2) {
		printf("Var\tDay\tCount\tDeltaWeeks\n");
		for(ii=istart;ii<istop;ii++){
			printf("Cases\t%ld\t%g\t%g\n",ii,cases1[ii],weeks1[ii]);
			printf("Deaths\t%ld\t%g\t%g\n",ii,deaths1[ii],weeks1[ii]);
		}
		if(overrun>0 && setpad==1) {
			aa= cases1[nn-1];
			bb= deaths1[nn-1];
			for(ii=ii;ii<(nn+overrun);ii++) {
				printf("c\t%ld\t%g\t%g\n",ii,aa,weeks1[ii]);
				printf("d\t%ld\t%g\t%g\n",ii,bb,weeks1[ii]);
	}}}

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(ikeys!=NULL) free(ikeys);
	if(keycols!=NULL) free(keycols);
	if(days1!=NULL) free(days1);
	if(cases0!=NULL) free(cases0);
	if(cases1!=NULL) free(cases1);
	if(deaths0!=NULL) free(deaths0);
	if(deaths2!=NULL) free(deaths2);
	if(weeks1!=NULL) free(weeks1);
	exit(0);
}
