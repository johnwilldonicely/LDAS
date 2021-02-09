#define thisprog "xe-writebinary1"
#define TITLE_STRING thisprog" v5: 16.September.2015 [JRH]"

#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/************************************************************************
<TAGS>file </TAGS>

v5: 16.September.2015 [JRH]
	- bugfix in call to xf_writebin2_v: should pass data size, not type
	- add verbose-2 (logfile) output

v4: 7.September.2015 [JRH]
	- add n (sample count) to stderr output

v3: 28.June.2015 [JRH]
	- bugfix in sscanf format for unsigned and signed long integers

v2: 6.January.2015 [JRH]
	- bugfix in sscanf format for unsigned and signed charaacters
*************************************************************************/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],message[MAXLINELEN],*line=NULL;
	int x,y,z;
	double aa,bb,cc,dd, result_d[64];
	size_t ii,nn,maxlinelen=0;
	off_t iii,jjj;
	FILE *fpin,*fpout;

	/* program-specific variables */
	long nwords=0,*start=NULL;
	size_t datasize;
	off_t startbyte,bytestoread;

	/* arguments */
	int setdatatype=9,setverb=0;
	off_t setheaderbytes=0,setstart=0,setntoread=0;


	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Write an ASCII input to a simple binary file\n");
		fprintf(stderr,"Input and output will be treated as a single numerical data type\n");
		fprintf(stderr,"Multi-column input will be converted to a continous output stream\n");
		fprintf(stderr,"Choose appropriate -dt option to determine binary data type (see below)\n");
		fprintf(stderr,"Non-numeric data will be saved as 0 if -dt <8, or NAN if -dt =8 or 9\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: ASCII file name or \"stdin\"\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS, defaults in []:\n");
		fprintf(stderr,"	-dt: type of data to write [%d]\n",setdatatype);
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
		fprintf(stderr,"	-v set verbose mode (0=data only, 1=stderr-report, 2=logfile) [%d]\n",setverb);
		fprintf(stderr,"\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	Binary stream\n");
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

	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be 0-9 \n\n",thisprog,setdatatype); exit(1);}

	/************************************************************
	READ THE ASCII DATA - STORE AS APPROPRIATE TYPE
	************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	nn=0;

	if(setdatatype==0) {
		unsigned char *data=NULL;
		unsigned char xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(unsigned char *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%c",&xx)==1) data[nn++]=xx;
				else data[nn++]=(unsigned char)NAN;
		}}
		x=xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==1) {
		char *data=NULL;
		char xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(char *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%c",&xx)==1) data[nn++]=xx;
				else data[nn++]=(char)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==2) {
		unsigned short *data=NULL;
		unsigned short xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(unsigned short *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%hu",&xx)==1) data[nn++]=xx;
				else data[nn++]=(unsigned short)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==3) {
		short *data=NULL;
		short xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(short *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%hd",&xx)==1) data[nn++]=xx;
				else data[nn++]=(short)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==4) {
		unsigned int *data=NULL;
		unsigned int xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(unsigned int *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%u",&xx)==1) data[nn++]=xx;
				else data[nn++]=(unsigned int)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==5) {
		int *data=NULL;
		int xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(int *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%d",&xx)==1) data[nn++]=xx;
				else data[nn++]=(int)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==6) {
		unsigned long *data=NULL;
		unsigned long xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(unsigned long *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%lu",&xx)==1) data[nn++]=xx;
				else data[nn++]=(unsigned long)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==7) {
		long *data=NULL;
		long xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(long *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%ld",&xx)==1) data[nn++]=xx;
				else data[nn++]=(long)NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==8) {
		float *data=NULL;
		float xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(float *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%f",&xx)==1) data[nn++]=xx;
				else data[nn++]=NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(setdatatype==9) {
		double *data=NULL;
		double xx;
		while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
			if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
			start= xf_lineparse2(line,"\t ,",&nwords);
			for(ii=0;ii<nwords;ii++) {
				if((data=(double *)realloc(data,(nn+1)*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
				if(sscanf((line+start[ii]),"%lf",&xx)==1) data[nn++]=xx;
				else data[nn++]=NAN;
		}}
		x= xf_writebin2_v("stdout",(void *)data,nn,datasize,message);
		if(x>=0) free(data);
		else { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
	}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* report the selected data type and size in bytes */
	sprintf(message,"uchar char ushort short uint int ulong long float double\n");
	start= xf_lineparse2(message,"\t ,",&nwords);
	if(setverb==1) {
		fprintf(stderr,"datatype= %d (%s)\n",setdatatype,(message+start[setdatatype]));
		fprintf(stderr,"datasize= %lu bytes\n",datasize);
		fprintf(stderr,"n= %lu\n",nn);
	}
	if(setverb==2) {
		sprintf(outfile,"temp_%s.log",thisprog);
		if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: log file \"%s\" could not be created\n\n",thisprog,outfile);exit(1);}
		fprintf(fpout,"datatype= %d (%s)\n",setdatatype,(message+start[setdatatype]));
		fprintf(fpout,"datasize= %lu bytes\n",datasize);
		fprintf(fpout,"n= %lu\n",nn);
		fclose(fpout);
	}

	if(start!=NULL) free(start);
	if(line!=NULL) free(line);
	exit(0);

}
