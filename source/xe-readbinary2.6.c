#define thisprog "xe-readbinary2"
#define TITLE_STRING thisprog" v 6: 12.October.2014 [JRH]"

#define MAXLINELEN 1000
#define CHANNELMAX 256

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
<TAGS>file</TAGS>

v 6: 12.October.2014 [JRH]
	- enable option to read fixed number of data points starting at a particular number (-s and -n options)

v 5: 6.October.2014 [JRH]
	- change argument sr (sampling rate) to sf (sampling frequency) for consistency with other programs

v 4: 29.September.2014 [JRH]
	- improve channel-selection by using xf_lineparse2 function
	- avoid nested calls to strtok()
	- improve checks on channel validity

v 3: 3.June.2013 [JRH]
	- set default to all channels (64)
	- add binary output option

v 1.2: 29.September.2011 [JRH]
	- added option to output "all" channels

*************************************************************************/


/* external functions start */
long *xf_lineparse2(char *line,char *delimiters, long *nwords);
int xf_stats2_d(double *data, long n, int varcalc, double *result_d);
/* external functions end */

int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],line[MAXLINELEN],templine[MAXLINELEN],word[256],*matchstring=NULL,*pline,*pcol;
	long int i,j,k,n=0;
	int v,w,x,y,z,col,colmatch;
	int sizeofchar=sizeof(char),sizeofshort=sizeof(short),sizeoflong=sizeof(long),sizeofint=sizeof(int),sizeoffloat=sizeof(float),sizeofdouble=sizeof(double);
	float a,b,c,d;
	double aa,bb,cc,dd, result_d[64];
	FILE *fpin,*fpout;

	/* program-specific variables */
	short *buffer=NULL,*buffer2=NULL;
	int datasize,itemsread,itemstoread,blocksread;
	double *bindata=NULL,*bintime=NULL,time=0.0,timeprev=0.0,interval;
	int *chout=NULL; // array of set channel numbers
	long *chindex=NULL,nchout=0; // array of indices to each element of setchout, and total elements in chout

	/* arguments */
	char datatype[256];
	char *setchout=NULL; // pointer to argv if it defines a channel list character array
	int blocksize=1000,setasc=1,setchtot=1;
	float setfreq=0.0;
	off_t setstart=0,setntoread=0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Extract values from a binary file - ASCII output to screen\n");
		fprintf(stderr,"This version assumes data are short-integers (2 bytes)\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-ch: comma-separated channels to extract (zero to (nch-1), or \"all\")\n");
		fprintf(stderr,"	-nch: total number of channels[%d]\n",setchtot);
		fprintf(stderr,"	-s: start reading at this element (zero-offset) [%ld]\n",setstart);
		fprintf(stderr,"	-n: number of elements to read (0=all) [%ld]\n",setntoread);
		fprintf(stderr,"	-sf: sample frequency (Hz): if +ive, times are output [%g]\n",setfreq);
		fprintf(stderr,"		NOTE: -s and -n will be internally converted to bytes\n");
		fprintf(stderr,"	-b: size of multi-channel data-blocks to read at a time [%d]\n",blocksize);
		fprintf(stderr,"	-asc: ASCII output, 1=ASCII, 0=binary (short int) [%d]\n",setasc);
		fprintf(stderr,"		NOTE: time output is not possible with binary\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	%s data.txt -nch 64 -ch 0,1,2\n",thisprog);
		fprintf(stderr,"	cat temp.txt | %s stdin -nch 64 -ch 62,63\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	time (if -sf>0) and data for selected channel\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	for(i=2;i<argc;i++) {
		if( *(argv[i]+0) == '-') {
			if((i+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
			else if(strcmp(argv[i],"-nch")==0)  setchtot=atoi(argv[++i]);
			else if(strcmp(argv[i],"-ch")==0)   setchout=argv[++i];
			else if(strcmp(argv[i],"-s")==0)   setstart=(off_t)atol(argv[++i]);
			else if(strcmp(argv[i],"-n")==0)   setntoread=(off_t)atol(argv[++i]);
			else if(strcmp(argv[i],"-sf")==0)   setfreq=atof(argv[++i]);
			else if(strcmp(argv[i],"-b")==0)    blocksize=atoi(argv[++i]);
			else if(strcmp(argv[i],"-asc")==0)  setasc=atoi(argv[++i]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[i]); exit(1);}
	}}
	if(setchtot<1||setchtot>CHANNELMAX) {fprintf(stderr,"\n--- Error[%s]: invalid -nch (%d) : must be >0 and <%d\n\n",thisprog,setchtot,CHANNELMAX);exit(1);}
	if(setasc!=0 && setasc!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -asc (%d) : must be 0 or 1\n\n",thisprog,setasc,CHANNELMAX);exit(1);}

	if(setchout!=NULL) {
		chindex= xf_lineparse2(setchout,",",&nchout);
		if(nchout>setchtot) {fprintf(stderr,"\n--- Error[%s]: number of channels selected (%d) cannot exceed channel total (%d)\n\n",thisprog,nchout,setchtot,CHANNELMAX);exit(1);}
		if((chout=(int *)realloc(chout,(nchout)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(i=0;i<nchout;i++) {
			chout[i]=atol(setchout+chindex[i]);
			if(chout[i]>setchtot) {fprintf(stderr,"\n--- Error[%s]: channel number (%d) cannot exceed channel total (%d)\n\n",thisprog,chout[i],setchtot);exit(1);}
			if(chout[i]<0) {fprintf(stderr,"\n--- Error[%s]: channel %d is less than zero\n\n",thisprog);exit(1);}
	}}
	else {
		nchout=setchtot;
		if((chout=(int *)realloc(chout,(nchout)*sizeofint))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};
		for(i=0;i<setchtot;i++) chout[i]=i;
	}

	if(setntoread>0) blocksize=setntoread;


	datasize=sizeof(short);
	interval= (double)(1.0/setfreq);
	itemstoread=setchtot*blocksize;
	buffer=(short *)realloc(buffer,(itemstoread*datasize));
	buffer2=(short *)realloc(buffer2,(nchout+1*datasize));
	if(buffer==NULL) {fprintf(stderr,"\n\t--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}

	/************************************************************/
	/* READ AND OUTPUT THE DATA */
	/************************************************************/
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	fseek(fpin,setstart,SEEK_SET);
	z=nchout-1;

	/* binary output */
	if(setasc==0) {
		while((itemsread=fread(buffer,datasize,itemstoread,fpin))==itemstoread){
			for(i=0;i<blocksize;i++) {
				for(j=0;j<nchout;j++) buffer2[j] = buffer[i*setchtot+chout[j]];
				fwrite(buffer2,datasize,nchout,stdout);
			}
		if(setntoread>0) break;
		}
		/* Now output remaining records ( < itemstoread ) which were read in the final (unassigned) iteration in the above loop */
		if(itemsread<itemstoread) {
			blocksread=(int)(1.0*itemsread/setchtot);
			for(i=0;i<blocksread;i++) {
				for(j=0;j<nchout;j++) buffer2[j] = buffer[i*setchtot+chout[j]];
				fwrite(buffer2,datasize,nchout,stdout);
	}}}

	/* ascii output, no timestamps */
	else if(setfreq<=0) {
		while((itemsread=fread(buffer,datasize,itemstoread,fpin))==itemstoread){
			for(i=0;i<blocksize;i++) {
				for(j=0;j<z;j++) printf("%d\t",buffer[i*setchtot+chout[j]]);
				printf("%d\n",buffer[i*setchtot+chout[z]]);
			}
			if(setntoread>0) break;
		}
		/* Now output remaining records (<blocksize) which were read in the final (unassigned) iteration in the above loop */
		if(itemsread<itemstoread) {
			blocksread=(int)(1.0*itemsread/setchtot);
			for(i=0;i<blocksread;i++) {
				for(j=0;j<z;j++) printf("%d\t",buffer[i*setchtot+chout[j]]);
				printf("%d\n",buffer[i*setchtot+chout[z]]);
	}}}

	/* ascii output, with timestamps */
	else if(setfreq>0) {
		while((itemsread=fread(buffer,datasize,itemstoread,fpin))==itemstoread){
			for(i=0;i<blocksize;i++) {
				printf("%.6f\t",time);
				for(j=0;j<z;j++) printf("%d\t",buffer[i*setchtot+chout[j]]);
				printf("%d\n",buffer[i*setchtot+chout[z]]);
				time+=interval;
			}
			if(setntoread>0) break;
		}
		/* Now output remaining records (<blocksize) which were read in the final (unassigned) iteration in the above loop */
		if(itemsread<itemstoread) {
			blocksread=(int)(1.0*itemsread/setchtot);
			for(i=0;i<blocksread;i++) {
				printf("%.6f\t",time);
				for(j=0;j<z;j++) printf("%d\t",buffer[i*setchtot+chout[j]]);
				printf("%d\n",buffer[i*setchtot+chout[z]]);
				time+=interval;
	}}}

	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	if(buffer!=NULL) free(buffer);
	if(buffer2!=NULL) free(buffer2);
	if(chindex!=NULL) free(chindex);
	if(chout!=NULL) free(chout);
	exit(0);

}
