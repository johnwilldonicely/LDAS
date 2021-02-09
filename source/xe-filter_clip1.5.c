#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-filter_clip1"
#define TITLE_STRING thisprog" v5: 16.September.2015 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing filter</TAGS>

v5: 16.September.2015 [JRH]
	- bugfix in call to xf_writebin2_v: should pass data size, not type
v 4: 21.February.2014 [JRH]
	- replace setascin and setascout with datatype variable which represents ascii, binary or binx
	- allow no-filter option if -min==nan || -max==nan

v 3: 10.February.2014 [JRH]
	- minor bugfix in instructions
	- include check so that datatype is NOT set if input is ascii
v 2: 27.January.2013 [JRH]
	- add read-write binary functionality

*/

/* external functions start */
int xf_filter_clip1_f(float *X, size_t nn, size_t nseq, float min, float max, float newmin, float newmax, char *message);
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[MAXLINELEN];
	long i,j,k,l,m,n;
	size_t ii,jj,kk,nn;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd;
	FILE *fpin,*fpout;
	/* program-specific variables */
	off_t startbyte,bytestoread,parameters[8];
	size_t n1,n2,start,stop,npad=0;
	float *data1=NULL,newmin,newmax;
	double sum,mean;
	/* arguments */
	off_t setheaderbytes=0;
	size_t setnseq=1;
	int setverb=0,setdatatypeout=-1,setdatatype=-1,setnewmin=0,setnewmax=0;
	float setmin=-10.0,setmax=10.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Apply a clipping filter to the input\n");
		fprintf(stderr,"Clips data at user-defined min/max values\n");
		fprintf(stderr,"Clips data at user-defined min/max values\n");
		fprintf(stderr,"Non-numeric values, NAN or INF will also be \"clipped\"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\" comprised of a single column\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data  [%d]\n",setdatatype);
		fprintf(stderr,"		-1= ASCII\n");
		fprintf(stderr,"		 0-9= uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-dtout:output format (see -dt for options) [%d]\n",setdatatypeout);
		fprintf(stderr,"	-h: for binary input, size of header (bytes) excluded from output [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-min: minimum valid value [%g]\n",setmin);
		fprintf(stderr,"	-max: maximum valid value [%g]\n",setmax);
		fprintf(stderr,"		NOTE: if -min or -max are set to NAN, no filtering is performed\n");
		fprintf(stderr,"	-newmin: replacement minimum value [-min, by default]\n");
		fprintf(stderr,"	-newmax: replacement maximum value [-max by default]\n");
		fprintf(stderr,"	-nseq: required number of sequential values meeting criteria [%d]\n",setnseq);
		fprintf(stderr,"	-v: set verbose output (0=NO,1=YES) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s original.txt -max 0  > clipped.txt\n",thisprog);
		fprintf(stderr,"	%s original.bin -ascin -1 -dt 8 -max 0 -ascout -1 > clipped.bin\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-dt")==0) setdatatype=atoi(argv[++i]);
			else if(strcmp(argv[i],"-dtout")==0) setdatatypeout=atoi(argv[++i]);
			else if(strcmp(argv[i],"-h")==0) setheaderbytes=(off_t)atol(argv[++i]);
			else if(strcmp(argv[i],"-min")==0) setmin=atof(argv[++i]);
			else if(strcmp(argv[i],"-max")==0) setmax=atof(argv[++i]);
			else if(strcmp(argv[i],"-newmin")==0) { newmin=atof(argv[++i]); setnewmin=1; }
			else if(strcmp(argv[i],"-newmax")==0) { newmax=atof(argv[++i]); setnewmax=1; }
			else if(strcmp(argv[i],"-nseq")==0)   setnseq=atoi(argv[++i]);
			else if(strcmp(argv[i],"-v")==0) 	  setverb=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}

	if(setmin>=setmax) {fprintf(stderr,"\n--- Error[%s]: -max (%g) must be greater than -min (%g)\n\n",thisprog,setmax,setmin);exit(1);};
	if(setnseq<0) {fprintf(stderr,"\n--- Error[%s]: -nseq (%g) must be greater than 0 \n\n",thisprog,setnseq);exit(1);};
	if(setverb!=0 && setverb!=1) {fprintf(stderr,"\n--- Error[%s]: -v option (%d) must be 0 or 1 \n\n",thisprog,setverb);exit(1);};
	if(setdatatype<-1||setdatatype>9) {fprintf(stderr,"\n--- Error[%s]: -dt (%d) must be -1 or 0-9\n\n",thisprog,setdatatype);exit(1);};
	if(setdatatypeout<-1||setdatatypeout>9) {fprintf(stderr,"\n--- Error[%s]: -dtout (%d) must be -1 or 0-9\n\n",thisprog,setdatatypeout);exit(1);};

	if(setnewmin==0) newmin=setmin;
	if(setnewmax==0) newmax=setmax;

	/********************************************************************************/
	/* STORE RAW DATA - SINGLE COLUMN */
	/********************************************************************************/
	if(setdatatype==-1) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
		n1=0;z=0;
		while(fgets(line,MAXLINELEN,fpin)!=NULL) {
			if(sscanf(line,"%f",&a)!=1) a=NAN;
			if((data1=(float *)realloc(data1,(n1+1)*sizeoffloat))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
			if(!isfinite(a)) z=1;
			data1[n1++]=a;
		}
		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}

	else {
		parameters[0]= setdatatype;
		parameters[1]= setheaderbytes;
		parameters[2]= 0; /* bytes to skip */
		parameters[3]= 0; /* bytes to read (zero = read all) */

		data1= xf_readbin2_f(infile,parameters,message);

		if(data1!=NULL) n1=parameters[3];
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}
	//TEST:	for(ii=0;ii<1000;ii++) printf("%ld	%g\n",ii,data1[ii]);free(data1);exit(0);

	/********************************************************************************
	WARN, OUTPUT & EXIT IF THERE ARE TOO FEW DATA POINTS TO ACTUALLY FILTER
	********************************************************************************/
	if(n1==0){
		fprintf(stderr,"\n--- Error[%s]: input \"%s\" is empty\n\n",thisprog,infile);
		exit(1);
	}
	if(n1<setnseq) {
		for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]);
		if(data1!=NULL) free(data1);
		fprintf(stderr,"--- Warning[%s]: -nseq (%g) is greater than length of data (%g) from %s\n",thisprog,setnseq,n1,infile);
		exit(0);
	}

	/********************************************************************************
	APPLY THE FILTER
	********************************************************************************/
	if(isfinite(setmin) && isfinite(setmax)) {

		i= xf_filter_clip1_f(data1,n1,setnseq,setmin,setmax,newmin,newmax,message);

		if(i<0) {
			fprintf(stderr,"\n\t --- Error [%s]: %s\n\n\n",thisprog,message);
			free(data1);
			exit(1);
		}
		if(setverb==1) {
			fprintf(stderr,"\n");
			fprintf(stderr,"%s\n",message);
			fprintf(stderr,"infile: %s\n",infile);
			fprintf(stderr,"samples: %ld\n",n1);
			fprintf(stderr,"nseq: %ld\n",setnseq);
			fprintf(stderr,"min: %g\n",setmin);
			fprintf(stderr,"newmin: %g\n",newmin);
			fprintf(stderr,"max: %g\n",setmax);
			fprintf(stderr,"newmax: %g\n",newmax);
			fprintf(stderr,"\n");
		}
	}

	/********************************************************************************
	OUTPUT THE FILTERED DATA, OMITTING ANY PADDING
	********************************************************************************/
	if(setdatatypeout==-1) {
		for(ii=0;ii<n1;ii++) printf("%g\n",data1[ii]);
	}
	else {
		ii= xf_writebin2_v("stdout",(void *)data1,n1,sizeof(float),message);
		if(ii!=0){ fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

	}

	free(data1);
	exit(0);

}
