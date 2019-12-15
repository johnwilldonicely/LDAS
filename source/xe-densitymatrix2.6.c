#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-densitymatrix2"
#define TITLE_STRING thisprog" v.6: 23.November.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/* <TAGS> transform filter </TAGS> */

/*
v.6: 23 November 2015 [JRH]
	- replace xf_resample1 function with two separate functions for cconsistency across programs of binning algorithms
		1. xf_bin1a_d
		2. xf_expand1_d
	- the old "resample" functions were prone to undersampling at the edges

v 5: 5.May.2013 [JRH]
	- update qsort USAGE to use external function xf_compare1_f

v 4: 7.March.2013 [JRH]
v 3: 15.February.2013 [JRH]
	- switch to using the built-in linux qsort program
	- resample each line so that there are the same number of columns for each row
	- that is, all matrices are now prefectly rectangular

v 2: 10.November.2012 [JRH]
	- actually version 1, derived from xe-ldas-coherogram2	- this version just generates an LDAS-style asymmetric coherence matrix for a single subject
	- input is a coherence file with columns for frequency, time and coherence
	- no requirement that there be the same number of times for each frequency

*/


/* external functions start */
double xf_bin1a_d(double *data, size_t *setn, size_t *setz, size_t setbins, char *message);
double *xf_expand1_d(double *data , long nn, long setn, char *message);
void xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_compare1_f(const void *a, const void *b);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],line[MAXLINELEN],*pline,*pcol,message[MAXLINELEN];
	long int i,j,k,l,m,n;
	int v,w,x,y,z,col,colmatch;
	int sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	/* program-specific variables */
	long nlistgrp1=0,nlistfreq=0,nlisttime=0,ntempdata1=0,ntempdata2=0,maxntimes=0;
	float *grp1=NULL,*freq=NULL,*time=NULL,*listgrp1=NULL,*listfreq=NULL,*listtime=NULL;
	double *data=NULL,*tempdata1=NULL,*tempdata2=NULL;
	size_t zero=0,iii;
	/* arguments */
	int setformat=1,setbintot=25,setcolg1=1,setcolfreq=2,setcoltime=3,setcoldata=4;
	float setlow=0.0,sethigh=0.0,setbinwidth=0.0;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<5) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Generate a freq. x time density matrix from 3-columned input\n");
		fprintf(stderr,"Designed for files with unequal time categories per frequency band\n");
		fprintf(stderr,"Resamples so each row (frequency) has the same number of columns\n");
		fprintf(stderr,"Will average multiple entries for the same time/freq\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [freq] [time] [density]\n",thisprog);
		fprintf(stderr,"	[input]: file or \"stdin\" with freq,time & coherence columns\n");
		fprintf(stderr,"	[freq] column defining frequency [%d]\n",setcolfreq);
		fprintf(stderr,"	[time] column defining time [%d]\n",setcoltime);
		fprintf(stderr,"	[density] column containing density values [%d]\n",setcoldata);
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s CO2_coherence.txt 1 2   4\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin 2 2 4   7\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	average-density matrix - row = frequency, column=time\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	setcolfreq=atoi(argv[2]);
	setcoltime=atoi(argv[3]);
	setcoldata=atoi(argv[4]);

	/* STORE DATA METHOD 3 - newline delimited input  from user-defined columns */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	n=0;
	while(fgets(line,MAXLINELEN,fpin)!=NULL) {
		if(line[0]=='#') continue;
		pline=line; colmatch=3; // number of columns to match
		for(col=1;(pcol=strtok(pline," ,\t\n\r"))!=NULL;col++) {
			pline=NULL;
			if(col==setcolfreq && sscanf(pcol,"%f",&b)==1) colmatch--;
			if(col==setcoltime && sscanf(pcol,"%f",&c)==1) colmatch--;
			if(col==setcoldata && sscanf(pcol,"%lf",&aa)==1) colmatch--;
		}
		if(colmatch!=0 || !isfinite(a) || !isfinite(b)) continue;
		if((freq=(float *)realloc(freq,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((time=(float *)realloc(time,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((data=(double *)realloc(data,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		freq[n]=b;
		time[n]=c;
		data[n]=aa;
		n++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	//for(i=0;i<10;i++) fprintf(stderr,"%.12f	%.12f	%.12f\n",freq[i],time[i],data[i]);

	/* ALLOCATE MEMORY */
	if((listfreq=(float *)realloc(listfreq,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((listtime=(float *)realloc(listtime,(n+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((tempdata1=(double *)realloc(tempdata1,(n+1)*sizeofdouble))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};


	/* CREATE A SORTED LIST OF THE ELEMENTS IN freq */
	for(i=0;i<n;i++) listfreq[i]=freq[i];
	qsort(listfreq,n,sizeof(float),xf_compare1_f);
	/* copy only unique items to new version of listfreq */
	a=freq[0]; for(i=nlistfreq=1;i<n;i++) {if(listfreq[i]!=a) listfreq[nlistfreq++]=listfreq[i];a=listfreq[i]; }

	/* CREATE A SORTED LIST OF THE ELEMENTS IN time */
	for(i=0;i<n;i++) listtime[i]=time[i];
	qsort(listtime,n,sizeof(float),xf_compare1_f);
	/* copy only unique items to new version of listtime */
	a=time[0]; for(i=nlisttime=1;i<n;i++) {if(listtime[i]!=a) listtime[nlisttime++]=listtime[i];a=listtime[i]; }

	/* CREATE A TEMPORARY ARRAY FOR EACH OUTOUT ROW (FREQUENCY) */
	if((tempdata2=(double *)realloc(tempdata2,nlisttime*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* GENERATE THE MATRIX */
	// for each frequency, in reverse (starting at highest) - so the matrix will not be inverted for plotting
	for(j=(nlistfreq-1);j>-1;j--) {
		// for each time...
		ntempdata2=0;
		for(k=0;k<nlisttime;k++) {
			ntempdata1=0;
			/* copy the data for this category to a temp array */
			for(m=0;m<n;m++) if(freq[m]==listfreq[j] && time[m]==listtime[k]) tempdata1[ntempdata1++]=data[m];
			/* calculate the average data value for this combination of freq and time */
			if(ntempdata1>0) {
				xf_stats2_d(tempdata1,ntempdata1,2,result_d);
				tempdata2[ntempdata2++]=result_d[0];
			}
		}
		/* if this is the highest frequency, assume that this is the frequency with the maximum number of freq vs. time categories */
		if(j==(nlistfreq-1)) maxntimes=ntempdata2;
		/* resample the data for this frequency so it spans the entire potential list of times */
		if(ntempdata2<=maxntimes) {
			tempdata2= xf_expand1_d(tempdata2 ,ntempdata2,maxntimes,message);
			if(tempdata2==NULL) {fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
		}
		else {
			iii=ntempdata2;
			aa= xf_bin1a_d(tempdata2,&iii,&zero,maxntimes,message);
			if(aa<=0.0) {fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
		}
		/* output */
		i=0; printf("%.3f",tempdata2[i]); for(i=1;i<maxntimes;i++) printf("	%.3f",tempdata2[i]); printf("\n");
	}

	/* FREE MEMORY */
	if(freq!=NULL) free(freq);
	if(listfreq!=NULL) free(listfreq);
	if(tempdata2!=NULL) free(tempdata2);
	if(time!=NULL) free(time);
	if(listtime!=NULL) free(listtime);
	if(data!=NULL) free(data);
	if(tempdata1!=NULL) free(tempdata1);

	exit(0);
	}
