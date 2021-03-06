#define thisprog "xe-CORONA1"
#define TITLE_STRING thisprog" 20.May.2020 [JRH]"
#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
/*  define and include required (in this order!) for time functions */
#define __USE_XOPEN // required specifically for strptime()
#include <time.h>


/*
??? TODO:
	- use time functions to express time in actual days-from-start based on date
		- instead of assuming record-number = day!
	- allow time-0 to be current day, peak day, day-of-mindeaths

<TAGS> epidemiology </TAGS>

v 1: 20.May.2020 [JRH]

*/



/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
long *xf_getkeycol(char *line1, char *d1, char *keys1, char *d2, long *nkeys1, char *message);
long xf_interp3_d(double *data, long ndata);
int xf_filter_bworth1_d(double *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
int xf_smoothgauss1_d(double *original, size_t arraysize,int smooth);
double xf_correlate_simple_d(double *x, double *y, long nn, double *result_d);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,message[MAXLINELEN];
	int vector[] = {1,2,3,4,5,6,7},z;
	long ii,jj,kk,nn,maxlinelen=0,nlines;
	float a,b,c;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;

	/* program-specific variables */
	char setkeys[]="Country,Date,Cases,Deaths,GeoID,Code,Pop,Continent\0",countrycode[8];
	char outfile[64];
	long *ikeys=NULL,*keycols=NULL,nkeys;

	int sizeofcases0,sizeofdeaths0;
	long *iword=NULL,*date0=NULL,*cases0=NULL,*deaths0=NULL,*deaths2=NULL,*pdeaths2,nwords;
	long istart=-1,istop=-1,imax=-1,itrough2=-1,i50,i25,overrun,maxdays;
	float *weeks1=NULL;
	double *days1=NULL,*cases1=NULL,*deaths1=NULL,*pdays1,*pdeaths1,popm=0.0,cmin,cmax,dmin,dmax,smooth,result_d[8];

	long lastdeaths2,slope2deaths2;
	double lastcases1,slope2cases1,lastdeaths1,slope1deaths1=NAN,slope2deaths1=NAN,dtot1,dtot2;

	/* time/date management variables */
	char timestring1[256],timestring2[256];
	time_t t1,t2;
	struct tm *tstruct1,*tstruct2;
	// intialise t1 and tstruct1 - this avoids using malloc or memset
	t1=t2= time(NULL);
	tstruct1=tstruct2= localtime(&t1);

	/* arguments */
	char *infile=NULL,*setcountry=NULL;
	int setverb=0,setpad=0,setnormd=0,setnormc=1,setout=1,setpeak2=0;
	long setmindeaths=10;
	double setsmooth=0.0,setmaxweeks=0.0;

	sprintf(outfile,"temp_%s.txt",thisprog);

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Analyse daily cases and deaths in an epidemiological dataset\n");
		fprintf(stderr,"- data: https://opendata.ecdc.europa.eu/covid19/casedistribution/csv\n");
		fprintf(stderr,"- must be downloaded & processed by xs-CORONA1\n");
		fprintf(stderr,"Required input columns: \n");
		fprintf(stderr,"    %s\n",setkeys);
		fprintf(stderr,"USAGE: %s [in] [country] [options]\n",thisprog);
		fprintf(stderr,"    [in]: file name or \"stdin\"\n");
		fprintf(stderr,"    	- ECDC data pre-processed by xs-CORONA1\n");
		fprintf(stderr,"    [country]: name of country to analyse\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"    -mindeaths: (start) minimum cumulative deaths [%ld]\n",setmindeaths);
		fprintf(stderr,"    -maxweeks: (stop) maximum weeks post-peak (0=total) [%g]\n",setmaxweeks);
		fprintf(stderr,"    -pad: add data if maxweeks exceeds data length (0=NO 1=YES) [%d]\n",setpad);
		fprintf(stderr,"    -smooth: weeks over which to smooth data (0=NONE) [%g]\n",setsmooth);
		fprintf(stderr,"        - uses Butterworth filter, high-cut= 1/(smooth*2)\n");
		fprintf(stderr,"    -normd: normalise deaths to population (millions) (0=NO 1=YES) [%d]\n",setnormc);
		fprintf(stderr,"    -normc: normalise cases to the deaths range (0=NO 1=YES) [%d]\n",setnormc);
		fprintf(stderr,"    -peak2: truncate data if a second larger peak is found (0=NO 1=YES) [%d]\n",setpeak2);
		fprintf(stderr,"    -out: output format [%d]\n",setout);
		fprintf(stderr,"        1: Day Cases Deaths DeathsSum\n");
		fprintf(stderr,"        2: Var Day Count\n");
		fprintf(stderr,"    -verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"    %s data.txt\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"    stdout: chunk of data as per -out above\n");
		fprintf(stderr,"    %s:\n",outfile);
		fprintf(stderr,"        code: 3-letter country code\n");
		fprintf(stderr,"        popm: country population, in millions\n");
		fprintf(stderr,"        start: day (from start of records) -mindeaths passed\n");
		fprintf(stderr,"        tmax: days from start to dmax\n");
		fprintf(stderr,"        t50:  days from tmax to 50%% dmax\n");
		fprintf(stderr,"        t25:  days from tmax to 25%% dmax\n");
		fprintf(stderr,"        cmax: max number of cases (after smoothing)\n");
		fprintf(stderr,"        dmax: max number of deaths (after smoothing)\n");
		fprintf(stderr,"        dtot: total cumulative deaths\n");
		fprintf(stderr,"        s1: best-fit linear slope, start to tmax\n");
		fprintf(stderr,"        s2: best-fit linear slope, tmax to maxweeks\n");
		fprintf(stderr,"        country: name of the input country\n");
		fprintf(stderr,"\n");
		exit(0);
	}

//???
	// // make a new tstruct1 and t1, perhaps from a string read from a file
	// snprintf(timestring1,32,"2021/01/19 20:50:00");
	// strptime(timestring1,"%Y/%m/%d %H:%M:%S", tstruct1); // convert string to broken-down-time (Y/M/D etc)
	// t1 = mktime(tstruct1);  // convert broken-down-time to seconds
	// fprintf(stderr,"\tstring: %s	time: %ld\n",timestring1,t1); // output
	//
	// t1+= 301; // add 5 minutes and 1 second
	// tstruct1= localtime(&t1); // convert seconds to broken-down-time (opposite of mktime)
	// strftime(timestring1,sizeof(timestring1),"%Y/%m/%d %H:%M:%S",tstruct1); // convert broken-down-time to string (opposite of strptime)
	// fprintf(stderr,"\tstring: %s	time: %ld\n",timestring1,t1); // output
	//
	// exit(0);






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
			else if(strcmp(argv[ii],"-maxweeks")==0) setmaxweeks=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-smooth")==0) setsmooth=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-pad")==0) setpad= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-normd")==0) setnormd= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-normc")==0) setnormc= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-peak2")==0) setpeak2= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setout= atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb!=0 && setverb!=1 && setverb!=999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setpad!=0 && setpad!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -pad [%d] must be 0 or 1\n\n",thisprog,setpad);exit(1);}
	if(setnormd!=0 && setnormd!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -normd [%d] must be 0 or 1\n\n",thisprog,setnormd);exit(1);}
	if(setnormc!=0 && setnormc!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -normc [%d] must be 0 or 1\n\n",thisprog,setnormc);exit(1);}
	if(setout!=1 && setout!=2) { fprintf(stderr,"\n--- Error [%s]: invalid -out [%d] must be 1 or 2\n\n",thisprog,setout);exit(1);}
	if(setpeak2!=0 && setpeak2!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -peak2 [%d] must be 0 or 1\n\n",thisprog,setpeak2);exit(1);}


	if(setsmooth!=0) smooth= 1.0/(setsmooth*2.0) ; else smooth=0.0;
	maxdays= (long)(setmaxweeks*7.0);

	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	- assumes the first line is the tab-delimited header defining the column-names
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	nlines=nn=nkeys=0;
	sizeofcases0= sizeof(*cases0);
	sizeofdeaths0= sizeof(*deaths0);
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}

		/* assign keycol numbers: setkeys[]= Country Date Cases Deaths GeoID Code Pop Continent */
		if(nlines==0) {
			keycols= xf_getkeycol(line,"\t",setkeys,",",&nkeys,message);
			if(keycols==NULL) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
			if(setverb==999) for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld setkeys=%s\n",ii,keycols[ii],setkeys);
		}

		/* parse the line */
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};

		/* if it's not the right country (keycols[0]), continue */
		if(strcmp(setcountry,line+iword[keycols[0]])!=0) { nlines++; continue; }
		//TEST CORRECT SELECTION: printf("%ld,%s,%s\n",keycols[0],setcountry,line+iword[keycols[0]]);

		/* store the population and country-code (do this once only)  */
		if(nn==0) {
			snprintf(countrycode,4,"%s",line+iword[keycols[5]]);
			sscanf(line+iword[keycols[6]],"%ld",&kk);
			popm= (double)kk/1000000.0;
		}

// ??? ADD THE DATE (LONG INT)


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

	if(nn==0) {fprintf(stderr,"\n--- Error[%s]: Country \"%s\" not found in \"%s\"infile\n\n",thisprog,setcountry,infile);exit(1);};
	if(setverb==999) printf("nn= %ld nkeys= %ld\n",nn,nkeys);
	if(setverb==999) printf("setkeys= %s\n",setkeys);
	if(setverb==999) for(ii=0;ii<nkeys;ii++) printf("%ld: keycols=%ld\n",ii,keycols[ii]);
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths0[%ld]= %ld\n",ii,deaths0[ii]);

	/********************************************************************************/
	/* INITIALISE EXTRA ARRAYS  */
	/********************************************************************************/
	/* days: double-version of line-number */

// ??? NO! THIS NEEDS TO BE DERIVED FROM DATE

	days1= malloc(nn*sizeof(*days1));
	if(days1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) days1[ii]= (double)ii;

	/* cases: double-version  */
	cases1= malloc(nn*sizeof(*cases1));
	if(cases1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) cases1[ii]= (double)(cases0[ii]);

	/* deaths: double-version  */
	deaths1= malloc(nn*sizeof(*deaths1));
	if(deaths1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nn;ii++) deaths1[ii]= (double)(deaths0[ii]);

	/* cumulative deaths - long integer */
	deaths2= malloc(nn*sizeof(*deaths2));
	if(deaths2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=kk=0;ii<nn;ii++) { if(deaths0[ii]>0) kk+= deaths0[ii]; deaths2[ii]= kk; }

	/* weeks-fractions - to help with later plotting  */
	weeks1= calloc(nn,sizeof(*weeks1));
	if(weeks1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths1[%ld]= %g\n",ii,deaths1[ii]);
	if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths2[%ld]= %ld\n",ii,deaths2[ii]);


	/********************************************************************************
	DATA-CLEAN
	- remove any "corrections" to cases or deaths (negative numbers) which may appear in some datasets
	/********************************************************************************/
	for(ii=0;ii<nn;ii++) {if(cases1[ii]<0) cases1[ii]=NAN; if(deaths1[ii]<0) deaths1[ii]=NAN;}
	ii= xf_interp3_d(cases1,nn);
	ii= xf_interp3_d(deaths1,nn);

	/********************************************************************************/
	/* FIND CHUNK OF DATA PART-1 : start (based on deaths2 vs. setmindeaths) */
	/********************************************************************************/
	/* find the start */
	istart= -1;
	for(ii=0;ii<nn;ii++) { if(deaths2[ii]>=setmindeaths) { istart=ii; break; } }
	if(istart==-1) {fprintf(stderr,"\n--- Error[%s]: -mindeaths threshold (%ld) never reached\n\n",thisprog,setmindeaths);exit(1);}
	if(setverb==999) printf("istart= %ld\n",istart);


	/********************************************************************************/
	/* FIXED PRE-FILTER TO DETECT EXTRA PEAKS  - 2-week filtering */
	/* - this will readjust "nn" to the trough just before the second peak, if found */
	/********************************************************************************/
	if(setpeak2==1) {
		z= xf_filter_bworth1_d(deaths1,nn,7.0,0.0,.5,sqrt(2),message);
		jj=kk= 0;
		itrough2= nn; // default stop based on second-peak is the whole dataset
		aa=bb=cc=dd= deaths1[istart];
		for(ii=istart;ii<nn;ii++) {
			cc= deaths1[ii];
			if(cc>bb && bb<aa) jj= ii;  // index to most recent trough
			if(cc<bb && bb>aa) {
				kk++; // peak-counter
				if(kk==1) dd= cc; // save amplitude of first peak
				if(kk>=2 && cc>dd) {
					fprintf(stderr,"--- Warning: larger second peak detected at day %ld\n",ii);
					itrough2=jj;
					break;
				}
			}
			aa= bb; bb= cc;
		}
		/* restore the original death1 array for re-filtering */
		for(ii=0;ii<nn;ii++) deaths1[ii]= (double)(deaths0[ii]);
		/* adjust the sample-count */
		if(itrough2<nn) nn= itrough2;
	}


	/********************************************************************************/
	/* NORMALISE DEATHS1 TO POPULATION (100,000)
	/********************************************************************************/
	if(setnormd==1) {
		for(ii=0;ii<nn;ii++) deaths1[ii]/= popm;
	}

	/********************************************************************************/
	/* APPLY THE FILTER TO CASES1 and DEATHS1 */
	/********************************************************************************/
	if(smooth!=0) {
		z= xf_filter_bworth1_d(cases1,nn,7.0,0.0,smooth,sqrt(2),message);
		if(z!=0) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
		z= xf_filter_bworth1_d(deaths1,nn,7.0,0.0,smooth,sqrt(2),message);
		if(z!=0) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }
		// z= xf_smoothgauss1_d(cases1,nn,smooth);
		// z= xf_smoothgauss1_d(deaths1,nn,smooth);
		if(setverb==999) for(ii=0;ii<nn;ii++) printf("cases1_smoothed[%ld]= %g\n",ii,cases1[ii]);
		if(setverb==999) for(ii=0;ii<nn;ii++) printf("deaths1_smoothed[%ld]= %g\n",ii,deaths1[ii]);
	}

	/********************************************************************************/
	/* FIND CHUNK OF DATA PART-2 : peak and stop (setmaxweeks) */
	/********************************************************************************/
	/* find the deaths peak */
	dmax= deaths1[istart];
	for(ii=istart;ii<nn;ii++) { if(deaths1[ii]>dmax) { dmax=deaths1[ii]; imax=ii; } }
	/* if unset, adjust maxdays to the actual data */
	if(maxdays==0) maxdays= (nn-imax)-1;
	/* determine the number of excess days being requested */
	overrun= (imax+maxdays+1)-nn ;
	/* find the stop (unincluded sample for loops) */
	istop= imax+maxdays+1;
	if(istop>nn) istop= nn;

	/* find the other min/max values between istart and istop */
	cmin=cmax= cases1[istart];
	dmin= deaths1[istart];
	for(ii=istart;ii<istop;ii++) {
		if(deaths1[ii]<dmin) dmin= deaths1[ii];
		if(cases1[ii]>cmax) cmax= cases1[ii];
		if(cases1[ii]<cmin) cmin= cases1[ii];
	}

	if(setverb==999) printf("istart= %ld imax= %ld istop= %ld\n",istart,imax,istop);
	if(setverb==999) for(ii=istart;ii<istop;ii++) printf("CHUNK[%ld]: %g\n",ii,deaths1[ii]);

	/********************************************************************************/
	/* BUILD THE WEEKS ARRAY, CENTRED ON THE PEAK */
	/********************************************************************************/
	for(ii=0;ii<nn;ii++) weeks1[ii]= (double)(ii-imax)/7.0;
	/* expand memory if required */
	if(overrun>0) {
		kk= nn+overrun;
		weeks1= realloc(weeks1,kk*sizeof(*weeks1));
		if(weeks1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		for(ii=ii;ii<kk;ii++) weeks1[ii]= (double)(ii-imax)/7.0;
	}

	/********************************************************************************/
	/* NORMALISE CASES */
	/********************************************************************************/
	if(setverb==999) printf("cmin= %g cmax=%g dmin= %g dmax= %g\n",cmin,cmax,dmin,dmax);
	if(setnormc==1) {
		cc= cmax-cmin; // cases range
		dd= dmax-dmin; // deaths range - multiply cases (scaled 0-1) by this
		if(cc==0.0) {fprintf(stderr,"\n--- Error[%s]: cannot normalise cases, cases range=0\n\n",thisprog);exit(1);}
		if(dd==0.0) {fprintf(stderr,"\n--- Error[%s]: cannot normalise cases, deaths range=0\n\n",thisprog);exit(1);}
		for(ii=0;ii<nn;ii++) {
			aa= cases1[ii]-cmin; if(aa!=0.0) aa/= cc; // aa= cases ranged 0-1, ready to adjust to deaths range
			cases1[ii]= (aa*dd) + dmin;
	}}

	/********************************************************************************/
	/* CALCULATE THE RISING DEATH-SLOPE  */
	/********************************************************************************/
	/* best fit for entire rising phase */
	pdays1= (days1+istart);
	pdeaths1= (deaths1+istart);
	kk= (imax-istart)+1;
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	slope1deaths1= result_d[1];


	/********************************************************************************/
	/* CALCULATE THE FALLING DEATH-SLOPE */
	/********************************************************************************/
	pdays1= (days1+imax);
	pdeaths1= (deaths1+imax);
	kk= (istop-imax);
	aa= xf_correlate_simple_d(pdays1,pdeaths1,kk,result_d);
	slope2deaths1= result_d[1];

	if(setverb==999) { for(ii=0;ii<kk;ii++) printf("FALLING[%ld]: %g\t%g\n",ii,pdays1[ii],pdeaths1[ii]) ; printf("falling_slope=%g\n",slope2deaths1);}

	/********************************************************************************/
	/* CALCULATE THE DAYS (POST-MAX) to 50% and 25% reduction in deaths  */
	/********************************************************************************/
	aa= 0.5*dmax;
	bb= 0.25*dmax;
	i50=i25=-1;
	for(ii=imax;ii<istop;ii++) if(deaths1[ii]<=aa) {i50= ii; break;}
	for(ii=imax;ii<istop;ii++) if(deaths1[ii]<=bb) {i25= ii; break;}
	if(i50>0) i50-=imax;
	if(i25>0) i25-=imax;
	if(setverb==999) printf("%g %ld %g	%g %ld %g\n",aa,i50,deaths1[i50],bb,i25,deaths1[i25]);



	/********************************************************************************/
	/* CALCULATE DTOT1 (PRE-PEAK) and DTOT2 (POST-PEAK) */
	/********************************************************************************/
	dtot1=dtot2= 0.0;
	for(ii=istart;ii<imax;ii++) dtot1+= deaths1[ii];
	for(ii=imax;ii<istop;ii++) dtot2+= deaths1[ii];

	/********************************************************************************/
	/* OUTPUT  */
	/********************************************************************************/
	/* open output file for saving regression data  */
	if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: unable to open file \"%s\" for writing\n\n",thisprog,outfile);exit(1);}
	fprintf(fpout,"code\tpopm\tstart\ttmax\tt50\tt25	cmax\tdmax\tdtot1\tdtot2	s1\ts2	country\n");
	fprintf(fpout,"%s\t%.3f\t%ld\t%ld\t%ld\t%ld	%.0f\t%.0f\t%.0f\t%.0f	%.3f\t%.3f	%s\n",
		countrycode,popm,istart,(imax-istart),i50,i25,
		cmax,dmax,dtot1,dtot2,
		slope1deaths1,slope2deaths1,setcountry
	);
	fclose(fpout);

	if(overrun>0) {
		lastcases1= cases1[nn-1];
		lastdeaths1= deaths1[nn-1];
		lastdeaths2= deaths2[nn-1];
		slope2cases1= cases1[nn-1]-cases1[nn-3];
		slope2deaths1= deaths1[nn-1]-deaths1[nn-3];
		slope2deaths2= deaths2[nn-1]-deaths2[nn-3];
	}


	if(setout==1) {
		printf("Day\tCases\tDeaths\tDeathsSum\tDeltaWeeks\n");
		for(ii=istart;ii<istop;ii++) { printf("%ld\t%g\t%g\t%ld\t%g\n",ii,cases1[ii],deaths1[ii],deaths2[ii],weeks1[ii]); }
		if(overrun>0 && setpad==1) {
			for(ii=ii;ii<(nn+overrun);ii++) {
				lastcases1+= slope2cases1;
				lastdeaths1+= slope2deaths1;
				lastdeaths2+= slope2deaths2;
				if(lastcases1<0) lastcases1=0;
				if(lastdeaths1<0) lastdeaths1=0;
				if(lastdeaths2<0) lastdeaths2=0;
				printf("%ld\t%g\t%g\t%ld\t%g\n",ii,lastcases1,lastdeaths1,lastdeaths2,weeks1[ii]);
			}
		}
	}
	if(setout==2) {
		printf("Var\tDay\tCount\tDeltaWeeks\n");
		for(ii=istart;ii<istop;ii++){
			printf("Cases\t%ld\t%g\t%g\n",ii,cases1[ii],weeks1[ii]);
			printf("Deaths\t%ld\t%g\t%g\n",ii,deaths1[ii],weeks1[ii]);
		}
		if(overrun>0 && setpad==1) {
			for(ii=ii;ii<(nn+overrun);ii++) {
				lastcases1+= slope2cases1;
				lastdeaths1+= slope2deaths1;
				if(lastcases1<0) lastcases1=0;
				if(lastdeaths1<0) lastdeaths1=0;
				printf("c\t%ld\t%g\t%g\n",ii,lastcases1,weeks1[ii]);
				printf("d\t%ld\t%g\t%g\n",ii,lastdeaths1,weeks1[ii]);
	}}}

	/********************************************************************************/
	/* CLEANUP AND EXIT */
	/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(ikeys!=NULL) free(ikeys);
	if(keycols!=NULL) free(keycols);

	if(date0!=NULL) free(date0);
	if(cases1!=NULL) free(cases1);
	if(deaths0!=NULL) free(deaths0);

	if(days1!=NULL) free(days1);
	if(deaths2!=NULL) free(deaths2);
	if(weeks1!=NULL) free(weeks1);
	exit(0);
}
