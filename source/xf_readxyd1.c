/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read a binary position-timestamp file (.xydt) and its complementary x-y-direction triplet file (.xyd)

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile1: file containing long-int (64-bit) timestamps
	char *infile2: file containing floating-point (32-bit) position data in triplets (x,y,direction)
	long **post:   unallocated pointer for the timestamp array, passed by calling function as &post
	long **posx:   unallocated pointer for the x-position array, passed by calling function as &posx
	long **posy:   unallocated pointer for the y-position array, passed by calling function as &posy
	long **posd:   unallocated pointer for the direction array, passed by calling function as &posd
	char *message: character array to hold messages, error-related or otherwise.

RETURN VALUE:
	The number of posx-posy-posd triplets read
	-1 on failure

SAMPLE CALL:
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#define thisprog "test the function"
	int main (int argc, char *argv[]) {
		char message[1000],infile1[256],infile2[256];
		long *post=NULL,ii,nn;
		float *posx=NULL,*posy=NULL,*posd=NULL;
		sprintf(infile1,argv[1]); sprintf(infile2,argv[2]);
		nn = xf_readxyd1(infile1,infile2,&posx,&posy,&posd,message);
		if(nn==-1) { fprintf(stderr,"\n\t--- Error: %s\n\n",message); exit(1); }
		for(ii=0;ii<nn;ii++) printf("%ld\t%f\t%f\t%f\n",posxt[ii],posx[ii],posy[ii],posd[ii]);
		exit(0);
	}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

long xf_readxyd1(char *infile1, char *infile2, long **post, float **posx, float **posy, float **posd, char *message) {

	char *thisfunc="xf_readxyd1\0";
	int nitems=3; // number of items per sample in the data-file
	int sizeoflong=sizeof(long),sizeoffloat=sizeof(float);
	long ii,jj,kk,mm,nn,datasize,blocksize,bytestoread,bytesread,nread;
	long *temp1=NULL;
	float *temp2a=NULL,*temp2b=NULL,*temp2c=NULL,*buffer2=NULL;
	double aa,bb,cc;
	FILE *fpin;

	/************************************************************/
	/* STORE THE TIMESTAMPS (SAMPLE-NUMBERS) */
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
		if((temp1=(long *)realloc(temp1,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		/* read a block of data */
		bytesread= fread(temp1+nn,1,bytestoread,fpin);
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
	/* STORE THE XYD DATA */
	/************************************************************/
	/* determine optimal (~64kib) block size */
	datasize= nitems * sizeof(float); // account for items-per-record
	aa= pow(2.0,16.0)/(double)datasize; // number of chunks per 64KiB
	blocksize= (off_t)pow(2,(log2(aa))); // find the next lowest power of two
	bytestoread=datasize*blocksize;
	/* allocate buffer memory */
	if((buffer2=(float *)realloc(buffer2,bytestoread))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
	mm=0;
	if(strcmp(infile2,"stdin")==0) fpin=stdin;
	else if((fpin=fopen(infile2,"rb"))==0) {sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,infile2);return(-1);}
	while(!feof(fpin)) {
		/* allocate memory for data  */
		kk= (mm+blocksize)*sizeoffloat;
		if((temp2a=(float *)realloc(temp2a,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		if((temp2b=(float *)realloc(temp2b,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}
		if((temp2c=(float *)realloc(temp2c,kk))==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(-1);}

		/* read a block of data */
		bytesread= fread(buffer2,1,bytestoread,fpin);
		/* make sure an appropriate number of bytes were read for the combined datasize */
		if((bytesread%datasize)!=0) {sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);return(-1);}
		/* if ok, calculate the number of triplets read */
		else nread=bytesread/datasize;
		/* if nothing was read this time, that's the end of the file! */
		if(nread==0) break;
		/* dynamically assign include/exclude pairs to array */
		jj=mm;
		kk=nread*nitems;
		for(ii=0;ii<kk;ii+=nitems) {
			temp2a[jj]=buffer2[ii];
			temp2b[jj]=buffer2[ii+1];
			temp2c[jj]=buffer2[ii+2];
			jj++;
		}
		mm+= nread;
	}
	if(strcmp(infile2,"stdin")!=0) fclose(fpin);

	/* compare number of records in input files */
	if(mm!=nn) {sprintf(message,"%s [ERROR]: unequal number of records in %s (%ld) and %s (%ld)",thisfunc,infile1,nn,infile2,mm);return(-1);}

	/* set pointers to the memory allocated by this function - temp2a, temp2b & temp2c */
	(*post) = temp1;
	(*posx) = temp2a;
	(*posy) = temp2b;
	(*posd) = temp2c;

	/* return the number of posx-posy-posd triplets */
	free(buffer2);
	return(nn);

}
