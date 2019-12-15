#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define thisprog "xe-readscore2"
#define TITLE_STRING thisprog" v 5: 20.September.2013 [JRH]"
#define MAXLINELEN 1000
#define MAXLABELS 1000
#define CHANNELMAX 256

/*
<TAGS>file</TAGS>

v 5: 20.September.2013 [JRH]
	- branch from original xe-elp-readscore1
	- this version reads TWO SCORE files and interlaces the data


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
size_t xf_readscore1(char *infile1, unsigned char **data, int headers, char *message);
size_t xf_writebinx1(FILE *fpout, void *data, size_t *params, size_t ntowrite, char *message);
/* external functions end */

int main (int argc, char *argv[]) {
	/* general variables */
	char infile1[MAXLINELEN],infile2[MAXLINELEN],outfile[MAXLINELEN],message[MAXLINELEN];
	int v,w,x,y,z,status=1;
	float a,b,c,d,result_f[32];
	double aa,bb,cc,dd,result_d[32];
	size_t ii,jj,kk,mm,nn;
	FILE *fpin,*fpout;
	/* program-specific variables */
	unsigned char *data1=NULL, *data2=NULL, *data3=NULL, *pdata;
	unsigned long blocksize=4000,params[6],ntowrite;
	/* arguments */
	int setasc=1,setheaders=0;
	unsigned long setstart=1,setstop=0;
	double setfreq=100.0; // sample frequency of input (samples/s)

	sprintf(outfile,"stdout");

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<3) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Read two SCORE raw files, interlace output as ASCII or binary stream\n");
		fprintf(stderr,"Non-numeric values will be recoded \"NaN\"\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [input1] [input2] [options]\n",thisprog);
		fprintf(stderr,"	[input1]: first file name (1 channel of data)\n");
		fprintf(stderr,"	[input2]: second file (1 channel of data)\n");
		fprintf(stderr,"		NOTE: input is pre-read to determine size\n");
		fprintf(stderr,"VALID OPTIONS:\n");
		fprintf(stderr,"	-asc: ASCII output, 1=YES 0=NO, (USE BINX binary format) [%d]\n",setasc);
		fprintf(stderr,"	-start: first block of 10-seconds to output[%d]\n",setstart);
		fprintf(stderr,"	-stop: last block of 10-seconds to output (0 = all) [%d]\n",setstop);
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}

	/********************************************************************************
	READ THE FILENAME AND OPTIONAL ARGUMENTS
	********************************************************************************/
	sprintf(infile1,"%s",argv[1]);
	sprintf(infile2,"%s",argv[2]);
	for(ii=3;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-asc")==0) 		{ setasc=atoi(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-start")==0) 	{ setstart=atol(argv[ii+1]); ii++;}
			else if(strcmp(argv[ii],"-stop")==0) 	{ setstop=atol(argv[ii+1]); ii++;}
			else {fprintf(stderr,"\n--- Error[%s]: invalid command line argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setasc!=0 && setasc!=1) {fprintf(stderr,"\n--- Error[%s]: invalid -asc (%d) : must be 0 or 1\n\n",thisprog,setasc);exit(1);}
	if(setstart<1) {fprintf(stderr,"\n--- Error[%s]: invalid -start (%d) : must be >0\n\n",thisprog,setstart);exit(1);}
	if(setstop>0 && setstop<setstart) {fprintf(stderr,"\n--- Error[%s]: invalid -stop (%d) : must be >= setstart or 0\n\n",thisprog,setstop);exit(1);}


 	/********************************************************************************
	READ THE DATA FROM FILE 1
	********************************************************************************/
 	fprintf(stderr,"%s\n",thisprog);
	fprintf(stderr,"	* Reading...\n");
 	mm = xf_readscore1(infile1,&data1,setheaders,message);
	if(mm>0) {
	 	fprintf(stderr,"	* %s \n",message);
	}
	else {
		fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
		goto END3;
	}
 	/********************************************************************************
	READ THE DATA FROM FILE 2
	********************************************************************************/
 	fprintf(stderr,"%s\n",thisprog);
	fprintf(stderr,"	* Reading...\n");
 	nn = xf_readscore1(infile2,&data2,setheaders,message);
	if(nn>=1) {
	 	fprintf(stderr,"	* %s \n",message);
	}
	else {
		fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
		free(data2);
		exit(1);
	}

	/* make sure files contain the same number of records */
	if(nn!=mm)  {
		fprintf(stderr,"\n--- Error[%s]: %s and %s contain unequal amounts of data - cannot be combined\n\n",thisprog,infile1,infile2);
		goto END2;
	}

	/********************************************************************************
	INTERLACE DATA INTO NEW ARRAY
	********************************************************************************/
	nn=nn*2;
	if((data3=(unsigned char *)realloc(data3,nn* sizeof(unsigned char)))==NULL) {
		fprintf(stderr,"\n--- Error[%s]: insufficient memmory\n\n",thisprog);
		free(data1);
		free(data2);
		exit(1);
	}
	for(ii=jj=0;ii<mm;ii++) {
		data3[jj++]=data1[ii];
		data3[jj++]=data2[ii];
	}
	//TEST:for(ii=0;ii<10;ii++) fprintf(stderr,"%d %d		%d\n",data1[ii],data2[ii],data3[ii]); goto END1;


	/********************************************************************************
	ADJUST OUTPUT
	********************************************************************************/
	blocksize=2*4000; // SCORE data blocks are 4000 samples (10s) - this is two interlaced datasets
	if(setstop<1) setstop = nn/blocksize;
	pdata = data3 + ((setstart-1)*blocksize);
	nn=((setstop-setstart)+1)*blocksize;
	//TEST: for(ii=0;ii<10;ii++) fprintf(stderr,"%d\n",pdata[ii]); goto END1;


	/********************************************************************************
	DEFINE PARAMETERS FOR BINARY OUTPUT, IF SETASC==0
	********************************************************************************/
	params[0] = 0; /* headersize - initialize to 0 for first call, to be set by the write function */
	params[1] = sizeof(unsigned char); /* datasize, bytes per datum  */
	params[2] = 0; /* datatype, 0 = unsigned char */
	params[3] = 2; /* depth of dimension-1 */
	params[4] = nn/2; 	/* depth of dimension-2 */
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
			goto END1;
	}}

	/* ASCII output */
	if(setasc==1) {
		for(ii=0;ii<nn;ii+=2) fprintf(fpout,"%d\t%d\n",pdata[ii],pdata[ii+1]);
	}
 	/* binary output */
	if(setasc==0) {
		ntowrite=nn;
		nn = xf_writebinx1(fpout,pdata,params,ntowrite,message);
		if(nn==0) fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message);
	}

 	if(strcmp(outfile,"stdout")!=0) fclose(fpout);

 	fprintf(stderr,"	* Complete: %d data-points written\n",nn);
	status=0;

END1:
	free(data3);
END2:
	free(data1);
END3:
	free(data2);

	exit(status);

	}
