#define thisprog "xe-repeated1"
#define TITLE_STRING thisprog" v 5.May.2019 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
<TAGS>file screen</TAGS>

v 5.May.2019 [JRH]
*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
void xf_linereadblock1(long *params,double *keycurr,double *keyprev,long *keycol,double *rep1,double *dat1,FILE *fpin,char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_d(const void *a, const void *b);
double xf_samplefreq1_d(double *time1, long n1, char *message);
long xf_interp3_d(double *data, long ndata);
int xf_filter_bworth1_d(double *X, size_t nn, float sample_freq, float low_freq, float high_freq, float res, char *message);
long xf_bin2_d(double *time, double *data , long n, double winwidth, int out);
long xf_norm3_d(double *data,long ndata,int normtype,long start,long stop,char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *line=NULL,*pword=NULL,message[MAXLINELEN];
	long ii,jj,kk,ll,mm,nn,maxlinelen=0,prevlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	char *words=NULL,**keyword=NULL;
	long nwords=0,*iword=NULL,*keycol=NULL,params[8],repcol=-1,datcol=-1,nkeys=0,ndata=0,status=0;
	long nstart=-1,nstop=-1;
	double *keycurr=NULL,*keyprev=NULL,*rep1=NULL,*dat1=NULL;
	/* arguments */
	char *infile=NULL,*setrep=NULL,*setdat=NULL,*setkeys=NULL;
	int setinterp=0,setnorm=-1,setverb=0;
	long setmax=1000;
	float setsampfreq=-1; // sample frequency of input (samples/s)
	float setlow=0.0,sethigh=0.0;  // filter cutoffs
	float setresonance=1.4142; // filter resonance setting ()
	double setn1=0.0,setn2=1.0,setbin=0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED
	********************************************************************************/
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Process repeated-measures data in blocks where key-values don't change\n");
		fprintf(stderr,"- input should be a multi-column table with a header-line\n");
		fprintf(stderr,"- one column holds the repeated measure (eg time), another the data\n");
		fprintf(stderr,"- key-columns (eg \"subject\") define blocks of repeats\n");
		fprintf(stderr,"- leading blank-lines and comments (#) will be ignored\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE: %s [in] [keys] [rep] [dat]   [options]\n",thisprog);
		fprintf(stderr,"	[in]: file name or \"stdin\"\n");
		fprintf(stderr,"	[keys]: CSV list of column-names for static keys\n");
		fprintf(stderr,"		- equivalence from line-to-line defines the data block\n");
		fprintf(stderr,"	[rep]: column-name for repeated-measure variable\n");
		fprintf(stderr,"	[dat]: column-name for data\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"OPTIONS (defaults in []):\n");
		fprintf(stderr,"	-interp: interpolate invalid values (0=NO 1=YES) [%d]\n",setinterp);
		fprintf(stderr,"	-max: max number of replicates in a block [%ld]\n",setmax);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES 999=DEBUG) [%d]\n",setverb);
		fprintf(stderr,"butterworth filter options:\n");
		fprintf(stderr,"	-low: low frequency limit, 0=SKIP [%g]\n",setlow);
		fprintf(stderr,"	-high: high frequency limit, 0=SKIP [%g]\n",sethigh);
		fprintf(stderr,"normalization options:\n");
		fprintf(stderr,"	-norm: normalization type (0=SKIP, see xe-norm3): [%d]\n",setnorm);
		fprintf(stderr,"	-n1: normalization zone (rep units) [%g]\n",setn1);
		fprintf(stderr,"	-n2: end of normalization zone (rep units) [%g]\n",setn2);
		fprintf(stderr,"binning options:\n");
		fprintf(stderr,"	-bin: time (rep) over which to average data (0=SKIP) [%g]\n",setbin);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt \"subject,group,day  time volts\"\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	columns [key1] [key2]... [rep] [dat]\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item
	********************************************************************************/
	infile= argv[1];
	setkeys= argv[2];
	setrep= argv[3];
	setdat= argv[4];

	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-max")==0)  setmax= atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0) setverb= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-interp")==0) setinterp= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-low")==0)  setlow=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-high")==0) sethigh=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-norm")==0) setnorm= atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-n1")==0)   setn1= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-n2")==0)   setn2= atof(argv[++ii]);
			else if(strcmp(argv[ii],"-bin")==0)  setbin= atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error [%s]: invalid command line argument [%s]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setmax<1) { fprintf(stderr,"\n--- Error [%s]: invalid -max [%ld] must be at least 1\n\n",thisprog,setmax);exit(1);}
	if(setverb!=0 && setverb!=1 && setverb != 999) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0,1, or 999\n\n",thisprog,setverb);exit(1);}
	if(setinterp!=0 && setinterp!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -interp [%d] must be 0 or 1\n\n",thisprog,setinterp);exit(1);}
	if(setlow<0) {fprintf(stderr,"\n--- Error[%s]: low frequency cutoff (-low %g) must be >= 0 \n\n",thisprog,setlow);exit(1);};
	if(sethigh<0) {fprintf(stderr,"\n--- Error[%s]: high frequency cutoff (-high %g) must be >= 0 \n\n",thisprog,sethigh);exit(1);};

	/* BUILD THE LIST OF keyword */
	/* key is an array of pointers to portions of setkeys, and hence to the list of keyword stored in argv[] */
	if((keyword=(char **)realloc(keyword,(strlen(setkeys)*sizeof(char *))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	/* build a list if indices to the start of each word in the list, and convert the commas to NULLS ('\0') */
	iword= xf_lineparse2(setkeys,",",&nkeys);
	/* point each key to the portion of setkeys containing a keyword */
	for(ii=0;ii<nkeys;ii++) keyword[ii]= &setkeys[iword[ii]];
	/* assign sufficient memory for key columns */
	if((keycol=(long *)realloc(keycol,(nkeys)*sizeof(long)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(ii=0;ii<nkeys;ii++) keycol[ii]=-1;

	/* ALLOCATE MEMORY FOR OTTHER VARIABLES */
	keycurr= malloc(nkeys*sizeof(*keyprev));
	keyprev= malloc(nkeys*sizeof(*keyprev));
	rep1= malloc(setmax*sizeof(*rep1));
	dat1= malloc(setmax*sizeof(*dat1));
	if(keycurr==NULL||keyprev==NULL||rep1==NULL||dat1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}


	/********************************************************************************
	OPEN THE FILE AND READ THE HEADER
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0; repcol= datcol= -1; for(jj=0;jj<nkeys;jj++) keycol[jj]=-1;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(line[0]=='#' || strlen(line)<2) continue; /* skip blank or commented lines */
		if(nn++==0) {
			iword= xf_lineparse2(line,"\t",&nwords);
			for(ii=0;ii<nwords;ii++) {
				pword= line+iword[ii];
				if(strcmp(setrep,pword)==0) repcol= ii;
				if(strcmp(setdat,pword)==0) datcol= ii;
				for(jj=0;jj<nkeys;jj++) { if(strcmp(keyword[jj],pword)==0) keycol[jj]= ii; }
			}
			if(repcol==-1) {fprintf(stderr,"\n--- Error[%s]: no column \"%s\" in %s\n\n",thisprog,setrep,infile);exit(1);}
			if(datcol==-1) {fprintf(stderr,"\n--- Error[%s]: no column \"%s\" in %s\n\n",thisprog,setdat,infile);exit(1);}
			for(jj=0;jj<nkeys;jj++) { if(keycol[jj]==-1) {fprintf(stderr,"\n--- Error[%s]: no column \"%s\" in %s\n\n",thisprog,keyword[jj],infile);exit(1);}}
			break;
	}}

	if(setverb>0){
		fprintf(stderr,"repcol= %ld\n",repcol);
		fprintf(stderr,"datcol= %ld\n",datcol);
		for(jj=0;jj<nkeys;jj++) fprintf(stderr,"keycol[%ld]= %ld\n",jj,keycol[jj]);
	}


	/********************************************************************************
	OUTPUT THE HEADER
	********************************************************************************/
	for(jj=0;jj<nkeys;jj++) printf("%s\t",keyword[jj]);
	printf("%s\t%s\n",setrep,setdat);

	/********************************************************************************
	PROCESS THE DATA
	********************************************************************************/
	/* initialize the parameters for the read function */
	params[0]= -9;
	params[1]= 0;
	params[2]= nkeys;
	params[3]= repcol;
	params[4]= datcol;
	params[5]= setmax;
	nn= 0;
	while(++nn) {
		/* READ A BLOCK  */
		xf_linereadblock1(params,keycurr,keyprev,keycol,rep1,dat1,fpin,message);
		status= params[0];
		ndata= params[1];
		if(status<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

		/* INTERPOLATE INVALID DATA */
		if(setinterp==1) ii= xf_interp3_d(dat1,ndata);

		/* FILTER */
		if(setlow>0||sethigh>0) {
			/* get the sample-frequency, assuming repeated-measure is seconds */
			setsampfreq= xf_samplefreq1_d(rep1,ndata,message);
			if(setsampfreq==0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
			/* apply the filter  */
			z= xf_filter_bworth1_d(dat1,ndata,setsampfreq,setlow,sethigh,setresonance,message);
			if(z<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		}

		/* BIN THE DATA */
		if(setbin>0) ndata= xf_bin2_d(rep1,dat1,ndata,setbin,2);

		/* NORMALISE THE BLOCK OF DATA */
		if(setnorm>=0) {
			for(ii=0;ii<ndata;ii++) {if(rep1[ii]<=setn1) nstart=ii; if(rep1[ii]> setn2) {nstop=ii;break;}}
			// TEST: fprintf(stderr,"nstart:%ld\tnstop:%ld\n\n",nstart,nstop);
			jj= xf_norm3_d(dat1,ndata,setnorm,nstart,nstop,message);
			if(jj==-2) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
		}

		/* OUTPUT THE BLOCK */
		for(ii=0;ii<ndata;ii++) {
			for(jj=0;jj<nkeys;jj++) printf("%g\t",keyprev[jj]);
			printf("%f\t%f\n",rep1[ii],dat1[ii]);
		}
		printf("\n");

		if(status==0) break; // end of file
		if(status==1) for(ii=0;ii<nkeys;ii++) keyprev[ii]= keycurr[ii];
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST for(ii=0;ii<nn;ii++) printf("%g\t%g\n",xdatf[ii],ydatf[ii]);

goto END;

/********************************************************************************/
/* CLEANUP AND EXIT */
/********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(rep1!=NULL) free(rep1);
	if(dat1!=NULL) free(dat1);
	exit(0);
}
