/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read a binary cluster-timestamp file (.clubt) and its complimentary cluster-id file (.club)

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile1:  file containing long-int (64-bit) timestamps
	char *infile2:  file containing short-int (16-bit) cluster-IDs
	long **clubt:   unallocated pointer for tinmestamp array, passed by calling function as &clubt
	short **club:   unallocated pointer for cluster-ID array, passed by calling function as &club
	char *message:  character array to hold messages, error-related or otherwise.

RETURN VALUE:
	The number of cluster records read
	-1 on failure

SAMPLE CALL:
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#define thisprog "test the function"
	int main (int argc, char *argv[]) {
		char message[1000],infile1[256],infile2[256];
		long *clubt=NULL,ii,nn;
		short *club=NULL;
		sprintf(infile1,argv[1]); sprintf(infile2,argv[2]);
		nn = xf_readclub1(infile1,infile2,&clubt,&club,message);
		if(nn==-1) { fprintf(stderr,"\n\t--- Error: %s\n\n",message); exit(1); }
		for(ii=0;ii<nn;ii++) printf("%ld\t%d\n",clubt[ii],club[ii]);
		exit(0);
	}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

long xf_readclub1(char *infile1, char *infile2, long **clubt, short **club, char *message) {

	char *thisfunc="xf_readclub1\0";
	int sizeofshort=sizeof(short),sizeoflong=sizeof(long);
	long ii,jj,kk,mm,nn,datasize,blocksize,bytestoread,bytesread,nread;
	short *temp2a=NULL;
	long *temp1a=NULL;
	double aa,bb,cc;
	FILE *fpin;

	/************************************************************/
	/* STORE THE CLUSTER TIMES (SAMPLE-NUMBERS) */
	/************************************************************/
	/* determine optimal (~64kib) block size */
	datasize=sizeof(long);
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
	bytestoread=datasize*blocksize;
	nn=0;
	if(strcmp(infile1,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile1,"rb"))==0) {sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,infile1);return(-1);}
	while(!feof(fpin)) {

		/* allocate memory for cluster-times */
		kk= (nn+blocksize)*sizeoflong;
		if((temp1a=(long *)realloc(temp1a,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		/* read a block of data */
		bytesread= fread(temp1a+nn,1,bytestoread,fpin);
		/* make sure an appropriate number of bytes were read for the data-size (long int = 16 bytes) */
		if((bytesread%datasize)!=0)	{sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);return(-1);}
		/* if ok, calculate the number of numbers read */
		else nread=bytesread/datasize;
		/* if nothing was read this time, that's the end of the file! */
		if(nread==0) break;
		/* otherwise, update nn */
		nn+= nread;
	}
	if(strcmp(infile1,"stdin")!=0) fclose(fpin);


	/************************************************************/
	/* STORE THE CLUSTER IDs */
	/************************************************************/
	/* determine optimal (~64kib) block size */
	datasize=sizeof(short);
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
	bytestoread=datasize*blocksize;

	mm=0;
	if(strcmp(infile2,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile2,"rb"))==0) {sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,infile2);return(-1);}
	while(!feof(fpin)) {
		/* allocate memory for cluster-ids */
		kk= (mm+blocksize)*sizeofshort;
		if((temp2a=(short *)realloc(temp2a,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		/* read a block of data */
		bytesread= fread(temp2a+mm,1,bytestoread,fpin);
		/* make sure an appropriate number of bytes were read for the data-size (short int = 4 bytes) */
		if((bytesread%datasize)!=0)	{sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);return(-1);}
		/* if ok, calculate the number of numbers read */
		else nread=bytesread/datasize;
		/* if nothing was read this time, that's the end of the file! */
		if(nread==0) break;
		/* otherwise, update mm */
		mm+= nread;
	}
	if(strcmp(infile2,"stdin")!=0) fclose(fpin);


	/************************************************************/
	/* MAKE SURE THE TWO FILES HAVE THE SAME NUMBER OF RECORDS */
	/************************************************************/
	if(mm!=nn) {sprintf(message,"%s [ERROR]: records in %s (%ld) unequal to records in %s (%ld)",thisfunc,infile1,nn,infile2,mm);return(-1);}


	/************************************************************/
	/* SET POINTERS TO THE MEMORY AND RETURN NO. OF RECORDS */
	/************************************************************/
	(*club) = temp2a;
	(*clubt)= temp1a;
	return(nn);

}
