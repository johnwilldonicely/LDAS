#define thisprog "xe-readbinary1"
#define TITLE_STRING thisprog" v 8: 13.May.2016 [JRH]"

#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/********************************************************************************
<TAGS>file</TAGS>

v 8: 13.May.2016 [JRH]
	- bugfix: previously if -s was defined buut not -n, entire input was output

v 8: 16.September.2015 [JRH]
	- bugfix in method of calling xf_readbin3_v

v 7: 5.July.2015 [JRH]
	- switch to new more flexible binary read function readbin3_v

v 6: 7.October.2014 [JRH]
	- use correct print formats for unsigned number output

v 5: 19.January.2014 [JRH]
	- allow program to start reading at a particular item, independent of header

********************************************************************************/

/* external functions start */
void *xf_readbin3_v(char *infile, off_t *params, off_t *start, off_t *toread, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],message[MAXLINELEN];
	double aa,bb,cc,dd, result_d[64];
	size_t ii,nn;
	off_t iii,jjj;
	FILE *fpin,*fpout;

	/* program-specific variables */
	void *data=NULL;
	off_t temp1[1],temp2[1],params[4];
	int datasize;
	off_t setblocks,startbyte,bytestoread;
	/* arguments */
	int setdatatype=0,setverb=0;
	off_t setheaderbytes=0,setstart=0,setntoread=0;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract values from a simple binary file\n");
		fprintf(stderr,"Assumes data (numbers) are all of the same type\n");
		fprintf(stderr,"Allows for presence of a single header\n");
		fprintf(stderr,"Allows extraction of a chunk starting at a particular element\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS, defaults in []:\n");
		fprintf(stderr,"	-dt: type of data [%d]\n",setdatatype);
		fprintf(stderr,"		0: unsigned char\n");
		fprintf(stderr,"		1: signed char\n");
		fprintf(stderr,"		2: unsigned short\n");
		fprintf(stderr,"		3: signed short\n");
		fprintf(stderr,"		4: unsigned int\n");
		fprintf(stderr,"		5: signed int\n");
		fprintf(stderr,"		6: unsigned long\n");
		fprintf(stderr,"		7: signed long\n");
		fprintf(stderr,"		8: float\n");
		fprintf(stderr,"		9: double\n");
		fprintf(stderr,"	-h: size of header (bytes) excluded from output [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-s: start reading at this element (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of elements to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"		NOTE: -s and -n will be internally converted to bytes\n");
		fprintf(stderr,"		NOTE: if -n is zero, reads from start to the end\n");
		fprintf(stderr,"	-v set verbose mode (0=data only, 1=stderr-report, 2=logfile) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -h 500 -s 0 -n 1000 \n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin \n",thisprog);
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	ASCII numbers representing the data\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0) setheaderbytes=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0) setstart=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0) setntoread=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setverb<0||setverb>2) {fprintf(stderr,"\n--- Error[%s]: verbocity (-v %d) must be 0-2 \n\n",thisprog,setverb); exit(1);}

	if(setdatatype==0||setdatatype==1)      datasize=sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=sizeof(long);
	else if(setdatatype==8)                 datasize=sizeof(float);
	else if(setdatatype==9)                 datasize=sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-dt %d) must be 0-9 \n\n",thisprog,setdatatype); exit(1);}


	/************************************************************
	READ THE DATA AS ONE BLOCK
	************************************************************/
	params[0]=datasize;
	params[1]=setheaderbytes;
	params[2]=1; // set number of blocks to read to 1
	params[3]=0; // initialize number of values read
	temp1[0]=setstart; // start index - assumes a single block
	temp2[0]=setntoread; // n-to-read - assumes a single block

	data= xf_readbin3_v(infile,params,temp1,temp2,message);
	if(data==NULL) { fprintf(stderr,"\n\t--- Error[%s/%s]\n\n",thisprog,message); exit(1); }
	if(params[3]<1) {fprintf(stderr,"\n\t--- Error [%s]: file %s is empty\n",thisprog,infile);exit(1);}
	else nn= params[3];

	/************************************************************
	OUTPUT - cast pdata to the appropriate type and link to a re-cast of data (originally void)
	************************************************************/
	if(setdatatype==0) {
		unsigned char *pdata=(unsigned char *)data;
		for(ii=0;ii<nn;ii++) printf("%u\n",pdata[ii]);
	}
	else if(setdatatype==1) {
		char *pdata=(char *)data;
		for(ii=0;ii<nn;ii++) printf("%d\n",pdata[ii]);
	}
	else if(setdatatype==2) {
		unsigned short *pdata=(unsigned short *)data;
		for(ii=0;ii<nn;ii++) printf("%hu\n",pdata[ii]);
	}
	else if(setdatatype==3) {
		short *pdata=(short *)data;
		for(ii=0;ii<nn;ii++) printf("%hd\n",pdata[ii]);
	}
	else if(setdatatype==4) {
		unsigned int *pdata=(unsigned int *)data;
		for(ii=0;ii<nn;ii++) printf("%u\n",pdata[ii]);
	}
	else if(setdatatype==5) {
		int *pdata=(int *)data;
		for(ii=0;ii<nn;ii++) printf("%d\n",pdata[ii]);
	}
	else if(setdatatype==6) {
		unsigned long *pdata=(unsigned long *)data;
		for(ii=0;ii<nn;ii++) printf("%lu\n",pdata[ii]);
	}
	else if(setdatatype==7) {
		long *pdata=(long *)data;
		for(ii=0;ii<nn;ii++) printf("%ld\n",pdata[ii]);
	}
	else if(setdatatype==8) {
		float *pdata=(float *)data;
		for(ii=0;ii<nn;ii++) printf("%f\n",pdata[ii]);
	}
	else if(setdatatype==9) {
		double *pdata=(double *)data;
		for(ii=0;ii<nn;ii++) printf("%lf\n",pdata[ii]);
	}

	if(setverb==1) {
		fprintf(stderr,"total_bytes: %ld\n",nn*datasize);
		fprintf(stderr,"total_numbers: %ld\n",nn);
	}
	if(setverb==2) {
		sprintf(outfile,"temp_%s.log",thisprog);
		if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: log file \"%s\" could not be created\n\n",thisprog,outfile);exit(1);}
		fprintf(stderr,"total_bytes: %ld\n",nn*datasize);
		fprintf(stderr,"total_numbers: %ld\n",nn);
		fclose(fpout);
	}


	free(data);
	exit(0);

}
