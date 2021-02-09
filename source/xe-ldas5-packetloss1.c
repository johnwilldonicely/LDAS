#define thisprog "xe-ldas5-packetloss1"
#define TITLE_STRING thisprog" 29.November.2018 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> /* to get maximum possible short value */

/* <TAGS> detect </TAGS> */

/*
29.November.2018 [JRH]
	- add ability to use zero or -1 as the invalid value
17.September.2016 [JRH]
	- bugfix memory potential problem - binning now occurs before double array is defined
	- make data1 range 0-1000, not 0-100
		- allows binning to get decimal precision even though we are working with integers

26.August.2016 [JRH]
	- add ability to set binning factor
	- bugfix: new bin function returns 0 or 1, not nbins

20.August.2016 [JRH]
	- first version
*/

/* external functions start */
int xf_readbin1_v(FILE *fpin, void *buffer1, off_t *params, char *message);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_bin1b_s(short *data, long *setn, long *setz, double setbinsize, char *message);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
int xf_percentile1_d(double *data, long n, double *result);
int xf_compare1_d(const void *a, const void *b);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
	long int ii,jj,kk,ll,mm,nn,nbad,nchars=0,maxlinelen=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d,result_f[64];
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	size_t iii,jjj,kkk,lll,nnn,mmm;
	size_t sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);

	/* program-specific variables */
	void *buffer1=NULL;
	short *buffpoint=NULL,*data1=NULL,bval,bval2,flipval;
	int datasize,itemsread,itemstoread,blocksread;
	off_t params[4]={0,0,0,0},block,nread,nlist;
	long *start1=NULL,*stop1=NULL,*index=NULL;
	long maxread,blocksize,nbins;
	double *data2=NULL;
	double binsize,min,max,mean;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	int setverb=0,setscreen=0,setbad=1;
	long setchtot=16;
	double setsf=19531.25,setbin=1.0;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate time-course of packet-loss in a .dat file\n");
		fprintf(stderr,"Assumes one valid numeric value per input line\n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%g]\n",setsf);
		fprintf(stderr,"	-nch: total number of channels [%d]\n",setchtot);
		fprintf(stderr,"	-bad: invalid value (0,-1, or 1=SHRT_MAX) [%d]\n",setbad);
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining inclusion bounds []\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining inclusion bounds []\n");
		fprintf(stderr,"	-bin: binning factor (seconds) to apply [%g]\n",setbin);
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -t 1\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -t 3\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: lower limit of each bin\n");
		fprintf(stderr,"	2nd column: value (eg. counts) in that bin\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)    setsf=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-nch")==0)   setchtot=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bad")==0)   setbad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)  setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0)  setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-bin")==0)   setbin=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setbad<-1||setbad>1) {fprintf(stderr,"\n--- Error[%s]: invalid -bad (%d) : must be -1, 0, or 1 \n\n",thisprog,setbad);exit(1);}

	binsize=setbin*setsf;

	/************************************************************/
	/* INTERPRET SETBAD - IF +1, SET BAD VALUE TO SHORT_MAX  */
	/************************************************************/
	if(setbad==1)  setbad=SHRT_MAX;

	/************************************************************/
	/* SET DATASIZE AS THE BYTES READ FOR EACH MULTI-CHANNEL SAMPLE */
	/************************************************************/
	datasize=setchtot*sizeof(short);

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE (MULTI_CHANNEL SAMPLES) */
	/* overridden by -b or -dec */
	/************************************************************/
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two

	/************************************************************/
	/* ALLOCATE BUFFER MEMORY */
	/************************************************************/
	maxread= blocksize*setchtot*datasize;
	if((buffer1=(void *)realloc(buffer1,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/************************************************************/
	/* OPEN INPUT AND SKIP THE REQUIRED NUMBER OF BYTES */
	/************************************************************/
	if(setverb==1) {
		fprintf(stderr,"\treading %s\n",infile);
	}
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file: %s\n\n",thisprog,infile);exit(1);}
	/************************************************************
	READ THE INCLUDE OR EXCLUDE LIST
	/************************************************************/
	if(setscreenlist!=NULL) {
		if(setverb==1) fprintf(stderr,"\tparsing screen-list\n");
		setscreen=1;
		index= xf_lineparse2(setscreenlist,",",&nlist);
		if((nlist%2)!=0) {fprintf(stderr,"\n--- Error[%s]: screen-list does not contain pairs of numbers: %s\n\n",thisprog,setscreenlist);exit(1);}
		nlist/=2;
		if((start1=(long *)realloc(start1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		if((stop1=(long *)realloc(stop1,nlist*sizeoflong))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(ii=0;ii<nlist;ii++) {
			start1[ii]=atol(setscreenlist+index[2*ii]);
			stop1[ii]= atol(setscreenlist+index[2*ii+1]);
	}}
	else if(setscreenfile!=NULL) {
		if(setverb==1) fprintf(stderr,"\treading screen-file %s\n",setscreenfile);
		setscreen=1;
		nlist = xf_readssp1(setscreenfile,&start1,&stop1,message);
		if(nlist==-1) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}
	//for(jj=0;jj<nlist;jj++) printf("%ld	%ld	%ld\n",jj,start1[jj],stop1[jj]);free(start1);free(stop1);exit(0);

	/************************************************************/
	/* FOR ONE CHANNEL ONLY, STORE THE WHOLE INPUT */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tblocksize:  %ld samples x %ld channels\n",blocksize,setchtot);
	if(setverb==1) fprintf(stderr,"\treading block: ");
	params[0]=datasize; /* ensures an error if bytes read do not match channel count */
	params[1]=blocksize;
	nn=block=z=0;
	while(!feof(fpin)) {
		if(setverb==1) fprintf(stderr,"%9ld\b\b\b\b\b\b\b\b\b",block);
		/* read a block of buffer1 */
		x= xf_readbin1_v(fpin,buffer1,params,message);
		/* check for error (fail to read, bad number of bytes read) */
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		/* set a short pointer to the buffer1, because that's what the input data is */
		buffpoint=(short *)buffer1;
		/* get the number of multi-channel buffpoint read */
		nread=params[2];
		/* if no buffpoint was read this time, that's the end of the file! */
		if(nread==0) break;
		/* this is the total number of values read */
		jj=nread*setchtot;
		/* allocate additional (nread) memory */
		data1=(short *)realloc(data1,(nn+nread)*sizeofshort);
		if(data1==NULL) { fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog); exit(1); }
		/* store the specified channel - channel number is the initial offset for the loop */
		for(ii=0;ii<jj;ii+=setchtot) data1[nn++]=buffpoint[ii];
		/* once the last bit is read, z should have been set to 1 : if so, break */
		if(z!=0) break;
		block++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==1) fprintf(stderr,"\n\tread %ld samples\n",nn);
	// TEST:for(ii=0;ii<nn;ii++) printf("%d\n",data1[ii]); exit(0);

	/************************************************************/
	/* SCREEN THE DATA USING THE START=STOP PAIRS  */
	/************************************************************/
	if(setscreen==1){
		if(setverb==1) fprintf(stderr,"\tscreening data using %ld start-stop pairs\n",nlist);
		ii=jj=kk=0;
		while(jj<nlist) {
			if(ii<start1[jj]) ii++;
			else if(ii>=stop1[jj]) jj++;
			else data1[kk++]=data1[ii++];
		}
		nn=kk;
	}


	/************************************************************/
	/* CONVERT DATA TO 0-1000 CODES */
	/* 0-1000 gives us 0.1% resolution for the final packet loss values */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tconverting data to 0 or 1000 codes for calculations\n");
	nbad=0;
	for(ii=0;ii<nn;ii++) {
		if(data1[ii]==setbad) { data1[ii]=1000; nbad++;}
		else data1[ii]=0;
	}
	printf("N= %ld\n",nn);
	printf("NBAD= %ld\n",nbad);
	printf("GRANDMEAN= %.9f\n",((double)(nbad*100)/(double)nn));

	/************************************************************/
	/* BIN THE DATA - THIS WILL MODIFY nn ! */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tbinning the data (bin size %.4f)\n",binsize);
	kk=0;
	z= xf_bin1b_s(data1,&nn,&kk,binsize,message);
	if(z==-1) {fprintf(stderr,"*** %s\n",message); exit(1);}
	nbins=nn;

	/************************************************************/
	/* COPY THE BINNED DATA TO DOUBLE FOR CALCULATING STATS */
	/* divide by 10 because the original values ranged from 0-1000, not 0-100 */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tconverting binned data to double for stats\n",binsize);
	data2=(double *)realloc(data2,nn*sizeof(double));
	if(data2==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n",thisprog);exit(1);}
	for(ii=0;ii<nn;ii++) data2[ii]=(double)data1[ii]/10.0;

	/************************************************************/
	/* CALCULATE MEAN PACKET LOSS */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tcalculating mean\n");
	xf_stats2_d(data2,nbins,1,result_d);
	min=result_d[4];
	max=result_d[5];
	mean=result_d[0];

	if(setverb==1) fprintf(stderr,"\tcalculating median\n");
	z=xf_percentile1_d(data2,nbins,result_d);
	if(z!=0) {fprintf(stderr,"\t\aError[%s]: insufficient memory for calculation of percentiles%s\n",thisprog);exit(1);}

	printf("NBINS= %ld\n",nbins);
	printf("MIN=  %.9f\n",min);
	printf("MAX=  %.9f\n",max);
	printf("MEAN=  %.9f\n",mean);
	printf("PERCENTILE_1= %.9f\n",result_d[0]);
	printf("PERCENTILE_2.5= %.9f\n",result_d[1]);
	printf("PERCENTILE_5= %.9f\n",result_d[2]);
	printf("PERCENTILE_10= %.9f\n",result_d[3]);
	printf("PERCENTILE_25= %.9f\n",result_d[4]);
	printf("PERCENTILE_50= %.9f\n",result_d[5]);
	printf("PERCENTILE_75= %.9f\n",result_d[6]);
	printf("PERCENTILE_90= %.9f\n",result_d[7]);
	printf("PERCENTILE_95= %.9f\n",result_d[8]);
	printf("PERCENTILE_97.5= %.9f\n",result_d[9]);
	printf("PERCENTILE_99= %.9f\n",result_d[10]);

	for(ii=0;ii<nn;ii++) printf("%.4f\n",data2[ii]);


	/************************************************************/
	/* CLEANUP AND EXIT  */
	/************************************************************/
	if(data1!=NULL) free(data1);
	if(data2!=NULL) free(data2);
	if(start1!=NULL) free(start1);
	if(stop1!=NULL) free(stop1);
	if(templine!=NULL) free(templine);
	exit(0);
}
