#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#define thisprog "xe-timestamp1"
#define TITLE_STRING thisprog" v 1: 8.March.2016 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing</TAGS>
v 4: 1.September.2019 [JRH]
	- add header output option
v 4: 8.March.2016 [JRH]
	- add ability to timestamp using sample-interval
v 4: 7.September.2015[JRH]
	- bugfix: change "start" from long to double for better precision
	- allow user to set roundig mode for integer output
v 3: 5.September.2015[JRH]
	- bugfix to zero-decimal-precision output - previously used %g formatter instead of %ld with a cast to long
v 2: 24.February.2014[JRH]
	- add ability to read simple binary files
v 1: 14.August.2012 [JRH]
	- bugfix - changed use of fscanf to read data with fgets/sscanf, to avoid problems related to "-" and "."
*/

/* external functions start */
float *xf_readbin2_f(char *infile, off_t *parameters, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char line[MAXLINELEN],infile[MAXLINELEN],message[MAXLINELEN];
	long ii,jj,nn;
	double aa;
	FILE *fpin;
	/* program-specific variables */
	off_t datasize,startbyte,bytestoread,parameters[8];
	double sampleint,start;
	float *data1=NULL;
	/* arguments */
	char *sethead=NULL;
	int setp=-1,setdatatype=-1,setround=0;
	double setsf=NAN,setsi=NAN,seto=0.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Apply time-stamps to each line of a data series\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: newline-delimited data file or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data  (ascii or binary types) [%d]\n",setdatatype);
		fprintf(stderr,"		-1= ASCII\n");
		fprintf(stderr,"		 0-9= uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"	-sf: sampling-frequency (Hz) to emulate [1]\n");
		fprintf(stderr,"	-si: sampling-interval (seconds) to emulate [unset]\n");
		fprintf(stderr,"		NOTE: cannot set both -sf and -si\n");
		fprintf(stderr,"	-o: start-sample offset (seconds) [%g]\n",seto);
		fprintf(stderr,"		e.g. you may want timestamps to begin at -5 seconds\n");
		fprintf(stderr,"	-p: decimal precision (-1=auto (%%f), 0=none(integer), >0=precision) [%d]\n",setp);
		fprintf(stderr,"	-r: round down (0) or to the nearest integer (1) [%d]\n",setround);
		fprintf(stderr,"		NOTE: only applies to integer output (-p 0) \n");
		fprintf(stderr,"	-head: optional timestamp-header text for files with headers [unset]\n");
		fprintf(stderr,"		NOTE: only applies to ASCII input (-dt -1)\n");
		fprintf(stderr,"		NOTE: assumes first line of input is the header\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -sf 1000 \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -sf 24000\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1st column: time (seconds)\n");
		fprintf(stderr,"	2nd column: data\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}


	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0) setsf=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-si")==0) setsi=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-o")==0)  seto=atof(argv[++ii]);
			else if(strcmp(argv[ii],"-p")==0)  setp=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-r")==0) setround=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-head")==0) sethead=argv[++ii];
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setp!=-1&&setp<0) {fprintf(stderr,"\n--- Error[%s]: -p (%d) option must be positive or -1\n\n",thisprog,setp);exit(1);}
	if(setround!=0&&setround!=1) {fprintf(stderr,"\n--- Error[%s]: -r (%d) option must be 0 or 1\n\n",thisprog,setround);exit(1);}
	if(isfinite(setsf) && isfinite(setsi)) {fprintf(stderr,"\n--- Error[%s]: canno set both -sf (%g) and -si (%g) \n\n",thisprog,setsf,setsi);exit(1);}

	if(isfinite(setsi)) {
		sampleint= setsi;
		setsf=1.0/setsi;
	}
	else {
		if(!isfinite(setsf)) setsf=1.0;
		sampleint=1.0/setsf;
	}
	start=(seto*setsf);

	/* READ DATA - IF ASCII */
	if(setdatatype==-1) {
		if(strcmp(infile,"stdin")==0) fpin=stdin;
		else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

		/* read first line and treat as header - output user-defined timestamp header */
		if(sethead!=NULL) {
			fgets(line,MAXLINELEN,fpin);
			printf("%s\t%s",sethead,line);
		}

		if(setp==-1) while(fgets(line,MAXLINELEN,fpin)!=NULL) {printf("%f\t%s",(start*sampleint),line);start++;}
		if(setp==0) {
			if(setround==0) while(fgets(line,MAXLINELEN,fpin)!=NULL) {printf("%ld\t%s",(long)(start*sampleint),line);start++;}
			if(setround==1) while(fgets(line,MAXLINELEN,fpin)!=NULL) {printf("%ld\t%s",(long)(0.5+(start*sampleint)),line);start++;}
		}
		if(setp>0) while(fgets(line,MAXLINELEN,fpin)!=NULL) {printf("%.*f\t%s",setp,(start*sampleint),line);start++;}

		if(strcmp(infile,"stdin")!=0) fclose(fpin);
	}
	/* OTHERWISE< IF BINARY, READ INTO MEMORY */
	else {
		parameters[0]= setdatatype;
		parameters[1]= 0; /* header bytes */
		parameters[2]= 0; /* bytes to skip */
		parameters[3]= 0; /* bytes to read (zero = read all) */

		data1= xf_readbin2_f(infile,parameters,message);
		if(data1!=NULL) nn=parameters[3];
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

		//TEST:	for(ii=0;ii<nn;ii++) printf("%ld	%g\n",ii,data1[ii]);free(data1);exit(0);

		if(setp==-1) for(ii=0;ii<nn;ii++) {
			printf("%f\t%g\n",(start*sampleint),data1[ii]);start++;
		}
		if(setp==0)  {
			if(setround==0) for(ii=0;ii<nn;ii++) {printf("%ld\t%g\n",(long)(start*sampleint),data1[ii]);start++;}
			if(setround==1) for(ii=0;ii<nn;ii++) {printf("%ld\t%g\n",(long)(0.5+(start*sampleint)),data1[ii]);start++;}
		}
		if(setp>0) {
			for(ii=0;ii<nn;ii++) {printf("%.*f\t%g\n",setp,(start*sampleint),data1[ii]);start++;}
		}

		free(data1);
	}

	exit(0);
	}
