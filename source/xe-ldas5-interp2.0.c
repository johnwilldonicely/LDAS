#define thisprog "xe-ldas5-interp2"
#define TITLE_STRING thisprog" v 0: 29.November.2018 [JRH]"

#define MAXLINELEN 1000
#define CHANNELMAX 256

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h> /* to get maximum possible short value */
#include <sys/stat.h>  /* this and next header allow testing file exists using stat() function */
#include <errno.h>

/* <TAGS> file signal_processing filter</TAGS> */

/************************************************************************
v 0: 17.December.2018 [JRH]
	- update instructions
v 0: 29.November.2018 [JRH]
	- add ability to use zero or -1 as the invalid value
v 0: 1.May.2016 [JRH]
	- add ability to set minimum good-value-sequence-length criteria
	- this allows removal of short bits of good data between packet-loss which seem to generate artefacts
v 0: 25.October.2015 [JRH]
*************************************************************************/

/* external functions start */
int xf_readbin1_v(FILE *fpin, void *buffer1, off_t *params, char *message);
int xf_filter_mingood2_s(short *data0, size_t nn, size_t nchan, size_t mingood, short setbad, char *message);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],message[256],*pline,*pcol;
	long int ii,jj,kk,nn=0;
	int v,w,x,y,z,col,colmatch;
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;
	struct stat sts;

	/* program-specific variables */
	void *buffer1=NULL;
	short *data=NULL,*buffer2=NULL,*prevgood=NULL,*tempdata=NULL,bval;
	int datasize,itemsread,itemstoread,blocksread;
	double *step=NULL;
	long headerbytes=0,maxread;
	long veryfirstgood,firstgood,lastgood,lastbad,badcount;
	off_t params[4]={0,0,0,0},block,nread,nreadtot;

	/* arguments */
	char datatype[256];
	int setout=0,setbad=1,setchtot=16,setverb=0;
	off_t setheaderbytes=0,setstart=0,setntoread=0,setblocksize=0,setmingood=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Linear interpolation of multi-channel .dat files (signed short)\n");
		fprintf(stderr," - reads entire file into memory\n");
		fprintf(stderr," - processes channels in parallel for speed\n");
		fprintf(stderr," - output is a binary stream - redirect to a file\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"		- samples are 16-bit short signed integers\n");
		fprintf(stderr,"		- a sample refers to a multi-channel set of data\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-nch: total number of channels [%d]\n",setchtot);
		fprintf(stderr,"	-bad: identify an invalid value (0,-1, or 1=max) [%d]\n",setbad);
		fprintf(stderr,"		- these values will be interpolated across\n");
		fprintf(stderr,"		- should be set, as NAN is undefined for integers\n");
		fprintf(stderr,"	-min: minimum-number of sequential good values [%ld]\n",setmingood);
		fprintf(stderr,"		- in each read-block, shorter sequences are set to bad\n");
		fprintf(stderr,"	-b: read-block size (multi-channel samples) (0=auto ~64KiB) [%ld]\n",setblocksize);
		fprintf(stderr,"	-verb: set verbocity of output (0=low, 1=high) [%d]\n",setverb);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s old.dat -verb 1 > new.dat\n",thisprog);
		fprintf(stderr,"	cat old.dat | %s stdin -nch 32 > new.dat\n",thisprog);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/************************************************************/
	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	/************************************************************/
	sprintf(infile,"%s",argv[1]);
	if(strcmp(infile,"stdin")!=0 && stat(infile,&sts)==-1 && errno == ENOENT) {
		fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);
		exit(1);
	}
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-nch")==0)  setchtot=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-bad")==0)    setbad=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-min")==0)    setmingood=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-b")==0)    setblocksize=atol(argv[++ii]);
			else if(strcmp(argv[ii],"-verb")==0)  setverb=atoi(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setchtot<1||setchtot>CHANNELMAX) {fprintf(stderr,"SHRT_MIN\n--- Error[%s]: invalid -nch (%d) : must be >0 and <%d\n\n",thisprog,setchtot,CHANNELMAX);exit(1);}
	if(setbad<-1||setbad>1) {fprintf(stderr,"\n--- Error[%s]: invalid -bad (%d) : must be -1, 0, or 1 \n\n",thisprog,setbad);exit(1);}
	if(setverb<0 || setverb>1) {fprintf(stderr,"\n--- Error[%s]: invalid -verb (%d) : must be 0-1\n\n",thisprog,setverb);exit(1);}
	if(setmingood<0) {fprintf(stderr,"\n--- Error[%s]: invalid -min (%d) : must be >=0\n\n",thisprog,setmingood);exit(1);}

	/* convert flags to modifier values for data */
 	if(setbad==0)  bval=0;
 	if(setbad==-1) bval=-1;
	if(setbad==1)  bval=SHRT_MAX;

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE (MULTI_CHANNEL SAMPLES) */
	/************************************************************/
	datasize=sizeof(short);
	if(setblocksize==0){
		ii= datasize*setchtot; // number of bytes per multi-channel data chunk
		aa= pow(2.0,16.0)/(double)ii; // number of chunks per 64KiB
		setblocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
	}

	/************************************************************/
	/* ALLOCATE MEMORY */
	/************************************************************/
	maxread= setblocksize*setchtot*datasize;
	if((buffer1=(void *)realloc(buffer1,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((buffer2=(short *)realloc(buffer2,maxread))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((prevgood=(short *)realloc(prevgood,setchtot*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((tempdata=(short *)realloc(tempdata,setchtot*datasize))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	if((step=(double *)realloc(step,setchtot*sizeof(double)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
	for(jj=0;jj<setchtot;jj++) prevgood[jj]=0;


	/************************************************************/
	/* OPEN THE INPUT */
	/************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: could not open file: %s\n\n",thisprog,infile);exit(1);}

	/************************************************************/
	/* READ THE DATA */
	/************************************************************/
	if(setverb==1) {
		fprintf(stderr,"\tblock_size: %ld samples\n",setblocksize);
		fprintf(stderr,"\treading block: ");
	}
	block=0;

	params[0]=datasize*setchtot; /* ensures an error if bytes read do not match datasize and channel count */
	params[1]=setblocksize;
	nreadtot=0;
	badcount=0;
	veryfirstgood=-1;

	while(!feof(fpin)) {

		if(setverb==1) fprintf(stderr,"%9ld\b\b\b\b\b\b\b\b\b",block);
		/* read a block of data */
		x= xf_readbin1_v(fpin,buffer1,params,message);
		/* check for error (fail to read, bad number of byytes read) */
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		/* get the number of data read */
		nread=params[2];
		nreadtot+=nread;
		/* if no data was read this time, that's the end of the file! */
		if(nread==0) break;
		/* set a pointer to the buffer1 */
		data=(short *)buffer1;
		/* condition the input to invalidate sections of good data which are too short - ignore errors */
		if(setmingood>0) xf_filter_mingood2_s(data,nread,setchtot,setmingood,bval,message);
		/* find the first good datum */
		firstgood=lastgood=lastbad=-1;
		for(ii=0;ii<nread;ii++) {
			if(data[ii*setchtot]==bval) lastbad=ii;
			else { if(firstgood<0) firstgood=ii; lastgood=ii; }
		}

/*
fprintf(stderr,"nread=%ld\n",nread);
fprintf(stderr,"firstgood=%ld\n",firstgood);
fprintf(stderr,"lastgood=%ld\n",lastgood);
fprintf(stderr,"lastbad=%ld\n",lastbad);
*/
		/* if there are no good values, nothing to be done but to increase the bad data counter */
		if(firstgood<0) { badcount+=nread; }

		/* if at least one good value was found... */
		else {

			/* if this is the very first good record in the file, re-initialise prevgood[] array */
			/* this ensures that the first interpolation resembles "sample & hold" */
			if(veryfirstgood<0) {
				veryfirstgood=firstgood;
				for(jj=0;jj<setchtot;jj++) prevgood[jj]=data[firstgood*setchtot+jj];
			}

			/* if all values are good... */
			/* interpolate & write if there are bad values at the end of the preceding block */
			/* write current block in its entireity  */
			if(lastbad<0) {	/* back-interpolation */
				if(badcount>0) {
					 for(jj=0;jj<setchtot;jj++) step[jj]=(double)(data[jj]-prevgood[jj]) / (double)(badcount+1);
					 for(kk=1;kk<=badcount;kk++) {
						for(jj=0;jj<setchtot;jj++) tempdata[jj]=(short)(prevgood[jj]+kk*step[jj]);
						fwrite(tempdata,(datasize*setchtot),1,stdout);
					 }
				}
				fwrite(buffer1,(datasize*setchtot),nread,stdout);
			}

			/* if there is a mixture of good & bad values... */
			/* crawl through the block, backin-interpolating and writing good data as it is encountered */
			else {
				for(ii=0;ii<nread;ii++) {

					if(data[ii*setchtot]==bval) badcount++;

					else {
						if(badcount>0) { /* back-interpolation */
							 for(jj=0;jj<setchtot;jj++) step[jj]=(double)(data[ii*setchtot+jj]-prevgood[jj]) / (double)(badcount+1);
							 for(kk=1;kk<=badcount;kk++) {
							 	for(jj=0;jj<setchtot;jj++) tempdata[jj]=(short)(prevgood[jj]+kk*step[jj]);
								fwrite(tempdata,(datasize*setchtot),1,stdout);
							 }
						}
						/* now reset prevgood[] as the current multi-channel record, and output */
						for(jj=0;jj<setchtot;jj++) prevgood[jj]=data[ii*setchtot+jj];
						fwrite(prevgood,(datasize*setchtot),1,stdout);
						badcount=0;
					}
				}
			}


			/* update badcount and update prevgood array with last good values in this block  */
			badcount=nread-lastgood-1;
			for(ii=0;ii<setchtot;ii++) prevgood[ii]=data[lastgood*setchtot+ii];
		}
		block++;
	}

	/* if there are any "leftover" bad data, output as prevgood (sample & hold) */
	if(badcount>0) {
		for(kk=0;kk<badcount;kk++) {
			fwrite(prevgood,(datasize*setchtot),1,stdout);
	}}


	if(strcmp(infile,"stdin")!=0) fclose(fpin);
	if(setverb==1) fprintf(stderr,"\n\tread %ld multi-channel records\n",nreadtot);

	if(step!=NULL) free(step);
	if(prevgood!=NULL) free(prevgood);
	if(buffer1!=NULL) free(buffer1);
	if(buffer2!=NULL) free(buffer2);
	exit(0);

}
