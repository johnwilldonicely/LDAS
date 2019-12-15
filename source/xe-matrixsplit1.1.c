#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>

#define thisprog "xe-matrixsplit1"
#define TITLE_STRING thisprog" v 1: 13.October.2014 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000

/*
<TAGS>signal_processing matrix</TAGS>

TO DO:

- add checks to windows

- add binary read option
	- must know "width" of matrix, and read width-sized chunks of data
	- thereafter, processing is identical to the ASCII version

*/

/* external functions start */
char *xf_lineread1(char *line, long *maxlinelen, FILE *fpin);
int xf_readbin1_f(FILE *fpin,off_t *parameters,float *data1,char *message);
int xf_rollbuffer1_f(float *data, size_t nbuff, size_t offset, int direction, char *message);
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	char infile[256],outfile[256],*line=NULL,message[MAXLINELEN];
	long int i,j,k,n,maxlinelen=0;
	int v,w,x,y,z;
	float a,b,c,d;
	double aa,bb,cc,dd,ee,result_d[64];
	FILE *fpin,*fpout;
	size_t ii,jj,kk,nn,mm;

	/* program-specific variables */
	int output=0;
	float *data1=NULL;
	off_t datasize,startbyte,ntoread,nread,bytestoread,parameters[8];
	size_t block,nblocks,*bstart=NULL,*bstop=NULL;

	/* arguments */
	char blkfile[256];
	int setdatatype=-1;
	float setsampfreq=1.0;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Split a large matrix (y-axis= time, top=zero) into blocks\n");
		fprintf(stderr,"Uses a start/stop file to determine which rows \n");
		fprintf(stderr,"Non-numeric values will be ignored\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [blockfile] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name or \"stdin\"\n");
		fprintf(stderr,"	[blockfile]: file containing <start> <stop> pairs defining blocks\n");
		fprintf(stderr,"		- <start> and <stop> are row-numbers, starting at zero\n");
		fprintf(stderr,"		- blocks must not overlap\n");
		fprintf(stderr,"		- however <stop> for one block may be <start> for the next\n");
		fprintf(stderr,"		- the <stop> sample is not included in the curent block\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-dt: type of data [%d]\n",setdatatype);
		fprintf(stderr,"		-1  = ASCII\n");
		fprintf(stderr,"		0 to 9 = uchar,char,ushort,short,uint,int,ulong,long,float,double\n");
		fprintf(stderr,"		NOTE: if data is not ASCII, matrix width (-w) must be defined\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/* READ THE INPUT FILENAME AND OPTIONAL ARGUMENTS */
	sprintf(infile,"%s",argv[1]);
	sprintf(blkfile,"%s",argv[2]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-dt")==0) setdatatype=atoi(argv[++ii]);
			else if(strcmp(argv[ii],"-sf")==0) setsampfreq=atof(argv[++ii]);
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}

	if(setdatatype<-1||setdatatype>9) {fprintf(stderr,"\n--- Error[%s]: data type (-dt %d) must be -1 or 0-9\n\n",thisprog,setdatatype);exit(1);};
	if(setsampfreq<=0) {fprintf(stderr,"\n--- Error[%s]: -sf (%ld) must be >0\n\n",thisprog,setsampfreq);exit(1);};


	/******************************************************************************/
	/* READ THE START SIGNALS */
	/******************************************************************************/
	fprintf(stderr,"Reading the start-stop signals...\n");
	if((fpin=fopen(blkfile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: time-file \"%s\" not found\n\n",thisprog,blkfile);exit(1);}
	nblocks=0;
	kk=sizeof(size_t);
	k=-1; // value that each start time must exceed
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		if(sscanf(line,"%ld %ld",&i,&j)!=2) continue;
		if(i<0||j<0) {fprintf(stderr,"\n--- Error[%s]: block file %s contains negative values\n\n",thisprog,blkfile);exit(1);}
		if(i>=j) {fprintf(stderr,"\n--- Error[%s]: block file %s contains a start-sample (%ld) which is >= the corresponding stop-sample (%ld)\n\n",thisprog,blkfile,i,j);exit(1);}
		if(i<k) {fprintf(stderr,"\n--- Error[%s]: block file %s contains a start-sample (%ld) which is < the preceeding stop-sample (%ld)\n\n",thisprog,blkfile,i,k);exit(1);}

		if((bstart=(size_t *)realloc(bstart,(nblocks+1)*kk))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		bstart[nblocks]=(size_t)i;
		if((bstop=(size_t *)realloc(bstop,(nblocks+1)*kk))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);}
		bstop[nblocks]=(size_t)j;

		k=j; // next start-sample must be at least the size of the current stop-sample
		nblocks++;
	}
	fclose(fpin);
	if(nblocks==0) {fprintf(stderr,"--- Warning[%s]: no valid windows in file %s\n",thisprog,blkfile);exit(0);}

//TEST: fprintf(stderr,"%ld	%ld	%ld\n",ii,bstart[ii],bstop[ii]);

	block=nn=0;
	/******************************************************************************/
	/* READ THE DATA  */
	/******************************************************************************/
	fprintf(stderr,"Reading the matrix...\n");

	/* ASCII READ */
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"r"))==0) {fprintf(stderr,"\n--- Error[%s]: file \"%s\" not found\n\n",thisprog,infile);exit(1);}
	while((line=xf_lineread1(line,&maxlinelen,fpin))!=NULL) {
		if(maxlinelen==-1)  {fprintf(stderr,"\n--- Error[%s]: readline function encountered insufficient memory\n\n",thisprog);exit(1);}
		/* firt check if this line marks the end of the current block - it may also mark the beginning of the next block*/
		if(nn>=bstop[block]) { if(++block>=nblocks) break; output=0;}
		/*now check if a new block has begun */
		if(nn==bstart[block]) { printf("# BLOCK %ld\n",block); output=1;}
		/* if we are in a block, output the line*/
		if(output==1) {	printf("%s",line); }
		nn++;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* BINARY READ */
	//??? to do

	/* FREE MEMORY */
	if(bstart==NULL) free(bstart);
	if(bstop==NULL) free(bstop);
	if(line!=NULL) free(line);
	if(data1!=NULL) free(data1);

	exit(0);
}
