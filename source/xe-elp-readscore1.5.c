#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-elp-readscore1"
#define TITLE_STRING thisprog" v 5: 1.July.2013 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#define CHANNELMAX 256

/*
<TAGS>signal_processing file</TAGS>

v 5: 1.July.2013 [JRH]
	- switch to using size_t file data counters, introduce variables ii,jj,kk,mm,nn
	- change -head option to -h, to avoid confusion with Linux head command
	- fix some error messages referring to legacy "-bin" option

 v 4: 28.June.2013 [JRH]
 	- output tweaks
	- minror fixes to xf_writebinx function
	- bugfix to ASCII output - now correctly adjusts pointer to data according to the -start argument

v 3: 12.June.2013 [JRH]
	- program now passes file-pointer to binary read-write functions - allows potential piecemeal writing of files
*/


/* external functions start */
size_t xf_readscore1(char *infile, unsigned char **data, int headers, char *message);
size_t xf_writebinx1(FILE *fpout, void *data, size_t *params, size_t ntowrite, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile[MAXLINELEN],outfile[MAXLINELEN],message[MAXLINELEN];
	int v,w,x,y,z;
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd,result_d[32];
	size_t ii,jj,kk,mm,nn;
	FILE *fpin,*fpout;
	/* program-specific variables */
	unsigned char *data=NULL,*pdata;
	unsigned long blocksize=4000,params[6],ntowrite;
	/* arguments */
	int setasc=1,setheaders=0;
	unsigned long setstart=1,setstop=0;
	double setfreq=100.0; // sample frequency of input (samples/s)

	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<2) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read a SCORE raw file and output as ASCII or binary stream\n");
		fprintf(stderr,"Non-numeric values will be recoded \"NaN\"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input] [options]\n",thisprog);
		fprintf(stderr,"	[input]: file name\n");
		fprintf(stderr,"		NOTE: input is pre-read to determine size\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-h: output ASCII headers only (0=NO, 1=YES) [%d]\n",setheaders);
		fprintf(stderr,"	-asc: ASCII output, 1=YES 0=NO, (USE BINX binary format) [%d]\n",setasc);
		fprintf(stderr,"	-start: first block of 10-seconds to output[%ld]\n",setstart);
		fprintf(stderr,"	-stop: last block of 10-seconds to output (0 = all) [%ld]\n",setstop);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	sprintf(infile,"%s",argv[1]);
	for(ii=2;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-h")==0) 	{ setheaders=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-asc")==0) 		{ setasc=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-start")==0) 	{ setstart=atol(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-stop")==0) 	{ setstop=atol(argv[ii+1]); ii++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setasc!=0 && setasc!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -asc (%d) : must be 0 or 1\n\n",thisprog,setasc);exit(1);}
	if(setheaders!=0 && setheaders!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -h (%d) : must be 0 or 1\n\n",thisprog,setheaders);exit(1);}
	if(setstart<1) {fprintf(stderr,"\n--- Error[%s]: invalid -start (%ld) : must be >0\n\n",thisprog,setstart);exit(1);}
	if(setstop>0 && setstop<setstart) {fprintf(stderr,"\n--- Error[%s]: invalid -stop (%ld) : must be >= setstart or 0\n\n",thisprog,setstop);exit(1);}

 	/********************************************************************************
	READ THE DATA
	********************************************************************************/
 	fprintf(stderr,"%s\n",thisprog);
	fprintf(stderr,"	* Reading...\n");

 	nn = xf_readscore1(infile,&data,setheaders,message);

	if(nn>=1) {
	 	fprintf(stderr,"	* %s \n",message);
	}
	else {
		fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
		exit(1);
	}
	/* if only headers are output, nothing else to do */
	if(setheaders==1) { free(data); exit(0); }

	/********************************************************************************
	ADJUST OUTPUT
	********************************************************************************/
	blocksize=4000;
	if(setstop<1) setstop = nn/blocksize;
	pdata = data + ((setstart-1)*blocksize);
	nn=((setstop-setstart)+1)*blocksize;

	/********************************************************************************
	DEFINE PARAMETERS FOR BINARY OUTPUT, IF SETASC==0
	********************************************************************************/
	params[0] = 0; /* headersize - initialize to 0 for first call, to be set by the write function */
	params[1] = sizeof(unsigned char); /* datasize, bytes per datum  */
	params[2] = 0;  	/* datatype, 0 = unsigned char */
	params[3] = nn; /* depth of dimension-1 */
	params[4] = 1; 	/* depth of dimension-2 */
	params[5] = 1;  /* depth of dimension-3 */


	/********************************************************************************
	OUTPUT THE DATA
	********************************************************************************/
 	fprintf(stderr,"	* Writing...\n");
	if(strcmp(outfile,"stdout")==0) {
		fflush(stdout);
		fpout=stdout;
	}
	else {
		if(setasc==1) fpout=fopen(outfile,"w");
		if(setasc==0) fpout=fopen(outfile,"wb");
		if(fpout==NULL) {
			fprintf(stderr,"\n--- Error[%s]: file \"%s\" could not be opened for writing\n\n",thisprog,outfile);
			exit(1);
	}}
	/* ASCII output */
	if(setasc==1) {
		for(ii=0;ii<nn;ii++) fprintf(fpout,"%d\n",pdata[ii]);
	}
 	/* binary output */
	if(setasc==0) {
		ntowrite=nn;
		nn = xf_writebinx1(fpout,pdata,params,ntowrite,message);
		if(nn==0) fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
	}

 	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

 	fprintf(stderr,"	* Complete: %ld data-points written\n",nn);

	free(data);

	}
