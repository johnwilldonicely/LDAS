#define thisprog "xe-readbinary3"
#define TITLE_STRING thisprog" v 1: 16.September.2015 [JRH]"

#define MAXLINELEN 1000
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
<TAGS>file</TAGS>

v 1: 16.September.2015 [JRH]
	- bugfix in verbose-2 (logfile) output

v 0: 28.October.2014 [JRH]
	- derived from xe-readbinary1.6.c  with the following changes
		- switch to block-read style: should allow piped input
		- switch to using xf_readbin1_f:
			- function handles conversion
			- bytes-to-read calculated by calling function - saves computation
	- note however that this is 4x slower than reading all the data into memory at once
	- ROOM FOR SPEED IMPROVEMENTS?

*************************************************************************/

/* external functions start */
int xf_readbin1_s(FILE *fpin, void *buffer, short *data, off_t *parameters, char *message);
int xf_readbin1_i(FILE *fpin, void *buffer, int *data, off_t *parameters, char *message);
int xf_readbin1_l(FILE *fpin, void *buffer, long *data, off_t *parameters, char *message);
int xf_readbin1_f(FILE *fpin, void *buffer, float *data, off_t *parameters, char *message);
int xf_readbin1_d(FILE *fpin, void *buffer, double *data, off_t *parameters, char *message);
int xf_writebin1_v(FILE *fpout, void *data0, size_t nn, size_t datasize, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],message[MAXLINELEN];
	int x,y,z;
	double aa,bb,cc,dd, result_d[64];
	size_t ii,jj,kk,nn;
	off_t iii,jjj;
	FILE *fpin,*fpout;

	/* program-specific variables */
	void *buffer1=NULL;
	short *data3=NULL;
	int *data5=NULL;
	long *data7=NULL;
	float *data8=NULL;
	double *data9=NULL;
	off_t parameters[8],datasize,startbyte,bytestoread,nread,nreadtot;
	/* arguments */
	int setdatatype=0,setouttype=-1,setverb=0;
	off_t setheaderbytes=0,setstart=0,setntoread=0,setblocksize=1000;


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
		fprintf(stderr,"Data is processed in chunks: reduced memory load but reduced speed\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name\n");
		fprintf(stderr,"\n");
		fprintf(stderr,"VALID OPTIONS, defaults in []:\n");
		fprintf(stderr,"	-dt: data type [%d]\n",setdatatype);
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
		fprintf(stderr,"	-out: set output type, -1 (ASCII) or 3,5,7,8,9 as above [%d]\n",setouttype);
		fprintf(stderr,"	-b: data block size (numbers to read at a time) [%ld]\n",setblocksize);
		fprintf(stderr,"	-h: size of header (bytes) excluded from output [%ld]\n",setheaderbytes);
		fprintf(stderr,"	-s: start reading at this element (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of elements to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"		NOTE: -s and -n will be internally converted to bytes\n");
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

	/************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-out")==0) setouttype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0) setblocksize=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-h")==0) setheaderbytes=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-s")==0) setstart=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-n")==0) setntoread=(off_t)atol(argv[++ii]);
			else if(strcmp(argv[ii],"-v")==0) setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setverb<0||setverb>2) {fprintf(stderr,"\n--- Error[%s]: verbocity (-v %d) must be 0-2 \n\n",thisprog,setverb); exit(1);}
	if(setouttype!=-1&&setouttype!=3&&setouttype!=5&&setouttype!=7&&setouttype!=8&&setouttype!=9) {fprintf(stderr,"\n--- Error[%s]: output type (-out %d) must be -1,3,5,7,8 or 9\n\n",thisprog,setverb); exit(1);}


	/************************************************************/
	/* DEFINE THE KEY PARAMETERS */
	/************************************************************/
	if(setdatatype==0||setdatatype==1) datasize=(off_t)sizeof(char);
	else if(setdatatype==2||setdatatype==3) datasize=(off_t)sizeof(short);
	else if(setdatatype==4||setdatatype==5) datasize=(off_t)sizeof(int);
	else if(setdatatype==6||setdatatype==7) datasize=(off_t)sizeof(long);
	else if(setdatatype==8) datasize=(off_t)sizeof(float);
	else if(setdatatype==9) datasize=(off_t)sizeof(double);
	else {fprintf(stderr,"\n--- Error[%s]: data type (-t %d) must be 0-9 \n\n",thisprog,setdatatype); exit(1);}

	bytestoread=(size_t)(0.5 + (setblocksize*datasize));
	parameters[0]= setdatatype;
	parameters[1]= datasize;
	parameters[2]= setblocksize;
	parameters[3]= 0;


	/************************************************************/
	/* ALLOCATE BUFFER MEMORY */
	/************************************************************/
	if((buffer1=(void *)realloc(buffer1,bytestoread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	//TEST:fprintf(stderr,"MAIN: bytestoread=%ld\n",bytestoread);

	/************************************************************/
	/************************************************************/
	/* READ THE DATA AND OUTPUT */
	/************************************************************/
	/************************************************************/
	nn=nreadtot=0;

	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}

	/* skip the required number of bytes - header plus data to skip */
	iii= setheaderbytes+(setstart*datasize);
	if(iii>0) {
		if(fseeko(fpin,iii,SEEK_CUR)!=0) {
				fprintf(stderr,"\n--- Error[%s]:  problem reading binary file (errno=%d)\n\n",thisprog,ferror(fpin));
				return(0);
	}}

	/* read data and output ASCII  */
	if(setouttype==-1) {
		if((data9=(double *)realloc(data9,(setblocksize*sizeof(double))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_d(fpin,buffer1,data9,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				for(ii=0;ii<kk;ii++) printf("%g\n",data9[ii]); // output this much of what was read this time
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else for(ii=0;ii<nread;ii++) printf("%g\n",data9[ii]);
	}}

	/* read data and output short   */
	if(setouttype==3) {
		if((data3=(short *)realloc(data3,(setblocksize*sizeof(short))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_s(fpin,buffer1,data3,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				x= xf_writebin1_v(stdout,data3,kk,sizeof(short),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else {
				x= xf_writebin1_v(stdout,data3,nread,sizeof(short),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}}}

	/* read data and output int  */
	if(setouttype==5) {
		if((data5=(int *)realloc(data5,(setblocksize*sizeof(int))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_i(fpin,buffer1,data5,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				x= xf_writebin1_v(stdout,data5,kk,sizeof(int),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else {
				x= xf_writebin1_v(stdout,data5,nread,sizeof(int),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}}}

	/* read data and output long   */
	if(setouttype==7) {
		if((data7=(long *)realloc(data7,(setblocksize*sizeof(long))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_l(fpin,buffer1,data7,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				x= xf_writebin1_v(stdout,data7,kk,sizeof(long),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else {
				x= xf_writebin1_v(stdout,data7,nread,sizeof(long),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}}}

	/* read data and output float   */
	if(setouttype==8) {
		if((data8=(float *)realloc(data8,(setblocksize*sizeof(float))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_f(fpin,buffer1,data8,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				x= xf_writebin1_v(stdout,data8,kk,sizeof(float),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else {
				x= xf_writebin1_v(stdout,data8,nread,sizeof(float),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}}}


	/* read data and output float   */
	if(setouttype==9) {
		if((data9=(double *)realloc(data9,(setblocksize*sizeof(double))))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		while(!feof(fpin)) {
			/* read a block of data */
			x= xf_readbin1_d(fpin,buffer1,data9,parameters,message);
			/* check for error (fail to read, bad number of byytes read) */
			if(x<0)	{fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1);}
			/* update the number of data read this time, and the total */
			nread=parameters[3]; nreadtot+=nread;
			/* if no data was read this time, that's the end of the file! */
			if(nread==0) break;
			/* if total read exceeds setntoread, then input has more than enough data, so only output what is needed to make up the right number, and break */
			if(setntoread>0 && nreadtot>=setntoread){
				jj=nreadtot-nread; // this is what was read in the previous iterration
				kk=setntoread-jj; // this is how much extra is needed to make up setntoread
				x= xf_writebin1_v(stdout,data9,kk,sizeof(double),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
				nreadtot=jj+kk; // modify the reported amount actually read
				break;
			}
			/* if we still havent read enough data (or if there is no limit), output everything and continue */
			else {
				x= xf_writebin1_v(stdout,data9,nread,sizeof(double),message);
				if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	}}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);


	/************************************************************/
	/* WRAP UP */
	/************************************************************/
	if(setverb==1) {
		fprintf(stderr,"total_read: %ld\n",nreadtot);
	}
	if(setverb==2) {
		sprintf(outfile,"temp_%s.log",thisprog);
		if((fpout=fopen(outfile,"w"))==0) {fprintf(stderr,"\n--- Error[%s]: log file \"%s\" could not be created\n\n",thisprog,outfile);exit(1);}
		fprintf(fpout,"total_read: %ld\n",nreadtot);
		fclose(fpout);
	}


	free(buffer1);
	if(data3!=NULL) free(data3);
	if(data5!=NULL) free(data5);
	if(data7!=NULL) free(data7);
	if(data8!=NULL) free(data8);
	if(data8!=NULL) free(data9);

	exit(0);

}
