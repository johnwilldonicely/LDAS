#define thisprog "xe-bin3.c"
#define TITLE_STRING thisprog" 4.July.2021 [JRH]"
#define MAXLINELEN 1000
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
<TAGS> signal_processing transform </TAGS>

4.July.2021 [JRH]
	- simplified (if not the speediest) binning program with options for mean, median, peak
 /


/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_bin1b_d(double *data1, long *setn, long *setz, double setbinsize, char *message);
long xf_binpeak1_d(double *data1,long n, double binsize, char *message);
double xf_percentile2_d(double *data, long nn, double setper, char *message);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char *line=NULL,message[256];
	int w,x,y,z;
	long ii,jj,kk,mm,nn,maxlinelen=0;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program specific variables */
	long *iword=NULL,nwords,sizeofdata;
	double *data1=NULL,*pdata,binstart,binlimit;
	/* arguments */
	char *infile=NULL;
	int setout=1;
	long setzero= 0;
	double setbin=1.0;

	/********************************************************************************
	PRINT INSTRUCTIONS IF THERE IS NO INPUT SPECIFIED
	********************************************************************************/
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"Bin data1 in non-overlapping windows\n");
		fprintf(stderr,"USAGE: %s [input] [options]\n",thisprog);
		fprintf(stderr,"    [input]: single-column data1 file or stdin\n");
		fprintf(stderr,"OPTIONS: defaults in []...\n");
		fprintf(stderr,"    -bin binsize (samples - can be fractional) [%g]\n",setbin);
		fprintf(stderr,"    -out output format 1(mean) 2(median) 3(peak) [%d])\n",setout);
		fprintf(stderr,"NOTES: \n");
		fprintf(stderr,"    - non-numeric input will produce an error\n");
		fprintf(stderr,"    - NAN and INF do not contribute to binned results\n");
		fprintf(stderr,"    - median function does not use variable binsizes (unlike mean and peak)\n");
		fprintf(stderr,"EXAMPLE:\n");
		fprintf(stderr,"    cut -f 2 data1.txt | %s stdin -bin 100 -out 3\n",thisprog);
		fprintf(stderr,"OUTPUT: the approriately downsampled data1\n");
		fprintf(stderr,"--------------------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(1);
	}

	/* READ COMMAND-LINE ARGUMENTS */
	infile= argv[1];
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-bin")==0) { setbin= atof(argv[++ii]); }
			else if(strcmp(argv[ii],"-out")==0) { setout= atoi(argv[++ii]); }
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setout<1||setout>3) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) - must be 1-3\n\n",thisprog,setout);exit(1);}
	if(setbin<=0){fprintf(stderr,"\n--- Error[%s]: invalid -bin (%g) - averaging window must be >0\n\n",thisprog,setbin);exit(1);}


	/********************************************************************************
	STORE DATA - ASSUME WE DON'T KNOW THE LENGTH OF EACH INPUT LINE
	********************************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	sizeofdata= sizeof(*data1);
	nn=mm=0;
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		mm++; // line counter for error-reporting
		iword= xf_lineparse2(line,"\t",&nwords);
		if(nwords<0) {fprintf(stderr,"\n--- Error[%s]: lineparse function encountered insufficient memory\n\n",thisprog);exit(1);};
		if(nwords<1) continue;

		aa= NAN;
		z= sscanf(line+iword[0],"%lf",&aa);
		if(z!=1) {fprintf(stderr,"\n--- Error[%s]: input contains non-numeric data on line %ld\n\n",thisprog,mm);exit(1);};

		data1= realloc(data1,(nn+1)*sizeofdata);
		if(data1==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		data1[nn]= aa;
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//TEST:	for(ii=0;ii<nn;ii++) printf("%ld\t%g\n",ii,data1[ii]);

	/********************************************************************************
	BIN THE DATA
	********************************************************************************/
	if(setbin==1) {
		z=1; // placeholder condition - no modification of data
	}
	else if(setout==1) {
		z= xf_bin1b_d(data1,&nn,&setzero,setbin,message);
		if(z<0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
	}
	else if(setout==2) {
		kk= (long)(setbin+0.5); // long version of binsize
		mm= 0; // counter for new number of bins
		binstart= 0.0;
		for(ii=0;ii<nn;ii++) {
			if(ii>=binstart) {
				pdata= (data1+ii);
				if(nn-ii)
				if((nn-ii)>=kk) {
					aa= xf_percentile2_d(pdata,kk,50.0,message);
				}
				else {
					aa= xf_percentile2_d(pdata,(nn-ii),50.0,message);
				}
				if(!isfinite(aa)) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
				data1[mm]= aa;
				mm++;
				binstart+= setbin; // readjust start
			}
			jj++;
		}
		nn= mm;
	}
	else if(setout==3) {
		jj= xf_binpeak1_d(data1,nn,setbin,message);
		if(jj<0) { fprintf(stderr,"\n\t%s/%s\n\n",thisprog,message); exit(1); }
		nn= jj;
	}

	else goto END;

	/********************************************************************************
	OUTPUT BINNED DATA
	********************************************************************************/
	for(ii=0;ii<nn;ii++) printf("%g\n",data1[ii]);


	/********************************************************************************
	CLEANUP & EXIT
	********************************************************************************/
END:
	if(line!=NULL) free(line);
	if(iword!=NULL) free(iword);
	if(data1!=NULL) free(data1);
	exit(0);

}
