/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read a binary Start-Stop-Pair file (.ssp) or equivalent stdin
	Data is stored as long integers (64-bit)

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile:   name of the file to read
	long **start:   unallocated pointer for start array passed by calling function as &start
	long **stop:    unallocated pointer for stop array passed by calling function as &start
	char *message:  character array to hold messages, error-related or otherwise.

RETURN VALUE:
	The number of start-stop pairs read
	-1 on failure
	0 if input was empty

SAMPLE CALL:

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#define thisprog "test-function"

	char message[1000],*infile;
	long *start=NULL,*stop=NULL
	long ii,nn;

	infile="mydata.txt\0";
	nn = xf_readscore1_s(infile,&start,&stop,0,message);
	if(nn==-1) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
	for(ii=0;ii<nn;ii++) printf("%ld\t%ld\n",start[ii],stop[ii]);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

long xf_readssp1(char *infile, long **start, long **stop, char *message) {

	int sizeoflong=sizeof(long);
	char *thisfunc="xf_readssp1\0";
	long ii,jj,kk,mm,nn,datasize,blocksize,bytestoread,bytesread,nread;
	long *buffer1=NULL,*temp1a=NULL,*temp1z=NULL;
	double aa,bb,cc;
	FILE *fpin;

	/************************************************************/
	/* DETERMINE OPTIMAL (~64KiB) BLOCK SIZE */
	/************************************************************/
	datasize= sizeof(long);
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
	bytestoread= datasize*blocksize;

	/************************************************************/
	/* ALLOCATE BUFFER MEMORY */
	/************************************************************/
	if((buffer1=(long *)realloc(buffer1,bytestoread))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}

	/************************************************************/
	/* STORE THE DATA */
	/************************************************************/
	nn=0;
	if(strcmp(infile,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile,"rb"))==0) {sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,infile);return(-1);}
	while(!feof(fpin)) {
		/* read a block of data */
		bytesread= fread(buffer1,1,bytestoread,fpin);
		/* make sure an appropriate number of bytes were read for the data-size (long int = 16bytes) */
		if((bytesread%datasize)!=0)	{sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);return(-1);}
		/* if ok, calculate the number of numbers read */
		else nread=bytesread/datasize;
		/* if nothing was read this time, that's the end of the file! */
		if(nread==0) break;
		/* otherwise make sure pairs of values were read */
		if((nread%2)!=0) {sprintf(message,"%s [ERROR]: file %s does not contain pairs of numbers",thisfunc,infile);return(-1);}
		/* allocate memory for start-stop pairs and store (nread must be divided by two) */
		kk= (nn+(nread/2))*sizeoflong;
		if((temp1a=(long *)realloc(temp1a,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		if((temp1z=(long *)realloc(temp1z,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);};
		/* dynamically assign include/exclude pairs to array */
		jj=nn;
		for(ii=0;ii<nread;ii+=2) {
			temp1a[jj]=buffer1[ii];
			temp1z[jj]=buffer1[ii+1];
			jj++;
		}
		nn+= nread/2;
	}
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* set pointers to the memory allocated by this function - temp1a & temp1z */
	(*start) = temp1a;
	(*stop)  = temp1z;

	/* return the number of start-stop pairs */
	free(buffer1);
	return(nn);

}
