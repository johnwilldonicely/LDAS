#define thisprog "xe-ldas5-packetloss2"
#define TITLE_STRING thisprog" 23.December.2018 [JRH]"
#define MAXLINELEN 1000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> /* to get maximum possible short value */

/*
<TAGS>signal_processing,time</TAGS>
23.December.2018 [JRH]
	- allow summary, spp, or bad-sample-number output
	- allow pre-invalidation of short periods of good data
29.November.2018 [JRH]
	- add ability to use zero or -1 as the invalid value
23.January.2018 [JRH]
	- simplification, really, of packetloss1
*/

/* external functions start */
int xf_readbin1_v(FILE *fpin, void *buffer1, off_t *params, char *message);
int xf_filter_mingood2_s(short *data0, size_t nn, size_t nchan, size_t mingood, short setbad, char *message);
long xf_readssp1(char *infile, long **start, long **stop, char *message);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_writebin1_v(FILE *fpout, void *data0, size_t nn, size_t datasize, char *message);

/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char *infile,*line=NULL,*templine=NULL,word[256],*pline,*pcol,message[MAXLINELEN];
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
	long maxread,blocksize;
	double *data2=NULL;
	double binsize,min,max,mean;

	/* arguments */
	char *setscreenfile=NULL,*setscreenlist=NULL;
	int setverb=0,setscreen=0,setbad=1,setout=1;
	long setchtot=16,setmingood=0;
	double setsf=19531.25,setbin=1.0;

	if((line=(char *)realloc(line,6))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((templine=(char *)realloc(templine,MAXLINELEN))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Calculate total packet-loss in a .dat file\n");
		fprintf(stderr,"	- uses channel-0, assumes packet loss will span all channels\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-sf: sample frequency (Hz) [%.3f]\n",setsf);
		fprintf(stderr,"	-nch: total number of channels [%d]\n",setchtot);
		fprintf(stderr,"	-bad: invalid value (0,-1, or 1=SHRT_MAX) [%d]\n",setbad);
		fprintf(stderr,"	-min: minimum-number of sequential good values [%ld]\n",setmingood);
		fprintf(stderr,"		- in each read-block, shorter sequences are set to bad\n");
		fprintf(stderr,"	-scrf: screen-file (binary ssp) defining inclusion bounds []\n");
		fprintf(stderr,"		- NOTE: SSP/sample-numbers output will be inaccurate\n");
		fprintf(stderr,"	-scrl: screen-list (CSV) defining inclusion bounds []\n");
		fprintf(stderr,"		- NOTE: SSP/sample-numbers output will be inaccurate\n");
		fprintf(stderr,"	-out: output format [%d]\n",setout);
		fprintf(stderr,"		1: summary\n",setout);
		fprintf(stderr,"		2: start-stop pairs for lost-data\n");
		fprintf(stderr,"		3: start-stop pairs for good-data\n");
		fprintf(stderr,"		4: sample-numbers of missing data (64-bit long int)\n");
		fprintf(stderr,"	-verb: verbose output (0=NO 1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s test.dat > missing.txt\n",thisprog);
		fprintf(stderr,"	%s test.dat -verb 1 | wc -l\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/* READ THE FILENAME AND OPTIONAL ARGUMENTS - including comma-separated list item */
	infile=argv[1];

	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-sf")==0)    setsf=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-nch")==0)   setchtot=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bad")==0)   setbad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0)   setout=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)   setmingood=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-scrf")==0)  setscreenfile=argv[++ii];
			else if(strcmp(argv[ii],"-scrl")==0)  setscreenlist=argv[++ii];
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setscreenfile!=NULL && setscreenlist!=NULL) {fprintf(stderr,"\n--- Error[%s]: cannot define both a screen-list and a screen-file\n\n",thisprog);exit(1);}
	if(setverb!=0 && setverb!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -verb [%d] must be 0 or 1\n\n",thisprog,setverb);exit(1);}
	if(setbad<-1||setbad>1) {fprintf(stderr,"\n--- Error[%s]: invalid -bad (%d) : must be -1, 0, or 1 \n\n",thisprog,setbad);exit(1);}
	if(setout<1||setout>4) {fprintf(stderr,"\n--- Error[%s]: invalid -out (%d) : must be 1-4\n\n",thisprog,setout);exit(1);}

	binsize=setbin*setsf;

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
	/* INTERPRET SETBAD - IF +1, SET BAD VALUE TO SHORT_MAX  */
	/************************************************************/
	if(setbad==1)  setbad=SHRT_MAX;

	/************************************************************/
	/* SET DATASIZE AS THE BYTES-READ FOR EACH MULTI-CHANNEL SAMPLE */
	/************************************************************/
	datasize= setchtot*sizeof(short);

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE (MULTI_CHANNEL SAMPLES) */
	/************************************************************/
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two

	/************************************************************/
	/* ALLOCATE BUFFER MEMORY */
	/************************************************************/
	maxread= blocksize*datasize;
	if((buffer1=(void *)realloc(buffer1,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	/************************************************************/
	/* OPEN INPUT */
	/************************************************************/
	if(setverb==1) {
		fprintf(stderr,"\treading %s\n",infile);
	}
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file: %s\n\n",thisprog,infile);exit(1);}

	/************************************************************/
	/* FOR ONE CHANNEL ONLY, STORE THE WHOLE INPUT */
	/************************************************************/
	if(setverb==1) fprintf(stderr,"\tblocksize:  %ld samples x %ld channels\n",blocksize,setchtot);
	if(setverb==1) fprintf(stderr,"\treading block: ");
	params[0]=datasize; /* multi-channel data-size, encourages error if bytes read don't match channel count */
	params[1]=blocksize;
	nn=block=z=0;
	while(!feof(fpin)) {
		if(setverb==1) fprintf(stderr,"%9ld\b\b\b\b\b\b\b\b\b",block);
		/* read a block of buffer1 */
		x= xf_readbin1_v(fpin,buffer1,params,message);
		/* check for error (fail to read, bad number of bytes read) */
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		/* set a short pointer to the buffer1, because that's what the input data is */
		buffpoint= (short *)buffer1;
		/* get the number of multi-channel buffpoint read */
		nread= params[2];
		/* if no buffpoint was read this time, that's the end of the file! */
		if(nread==0) break;
		/* condition the input to invalidate sections of good data which are too short - ignore errors */
		if(setmingood>0) xf_filter_mingood2_s(buffpoint,nread,setchtot,setmingood,setbad,message);
		/* this is the total number of values read */
		jj= nread*setchtot;
		/* allocate additional (nread) memory */
		data1= (short *)realloc(data1,(nn+nread)*sizeofshort);
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
	/* OUTPUT */
	/************************************************************/
	if(setout==1) {
		for(ii=mm=0;ii<nn;ii++) { if(data1[ii]==setbad) mm++; }
		aa= 100.0 *((double)mm/(double)nn);
		printf("total_samples= %ld\n",nn);
		printf("lost_samples= %ld\n",mm);
		printf("percent_lost= %.4f\n",aa);
	}

	if(setout==2) {
		z= 0; // flag, =1 if preceeding data is bad
		ii=0; if(data1[ii]==setbad) {
			x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
			if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			z=1;
		}
		for(ii=1;ii<nn;ii++) {
			if(data1[ii]==setbad) {
				if(z==0) {
					x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
				}
				z= 1;
			}
			else {
				if(z==1) {
					x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
				}
				z= 0;
			}
		}
		if(z==1) { // if data ends in bad block, output the final stop
			x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
			if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		}
	}

	if(setout==3) { // output SSPs for good blocks
		z= 1; // flag, =1 if preceeding data is bad
		ii=0; if(data1[ii]!=setbad) {
			x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
			if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
			z=0;
		}
		for(ii=1;ii<nn;ii++) {
			if(data1[ii]!=setbad) {
				if(z==1) {
					x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
				}
				z= 0;
			}
			else {
				if(z==0) {
					x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
					if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
				}
				z= 1;
			}
		}
		if(z==0) { // if data ends in good block, output the final stop
			x= xf_writebin1_v(stdout,&ii,1,sizeof(long),message);
			if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
		}
	}


	if(setout==4) {
		for(ii=0;ii<nn;ii++) {
			if(data1[ii]==setbad) {
				x= xf_writebin1_v(stdout,&ii,1,sizeoflong,message);
				if(x<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}}}



	/************************************************************/
	/* CLEANUP AND EXIT  */
	/************************************************************/
	if(data1!=NULL) free(data1);
	if(start1!=NULL) free(start1);
	if(stop1!=NULL) free(stop1);
	exit(0);
}
