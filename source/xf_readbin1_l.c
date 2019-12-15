/*
<TAGS>file </TAGS>
DESCRIPTION:

	Read simple binary stream into memory from a file, converting to long
	This version accepts a pointer to the input stream - suitable for repeat calls

	On each call:
		- calling function designates the destination array
		- a temporary void buffer is allocated to hold the read results
		- a pointer of the appropriate data-type is pointed at the buffer
		- the void buffer is copied, via the pointer, to the destination array, casting each number as a long

	NOTE:
		To read an entire file with one function call, (~5 times faster), use xf_readbin2_v
		That function allows the system to determine the obtimal block read size
		However, because it also requires pre-determining the size of the input, it is not suitable for reading from piped streams

USES:
	Reading a binary input stream of unknown size in chunks
	Suitable for reading binary input from stdin


DEPENDENCY TREE:
	No dependencies


ARGUMENTS:
	FILE *fpin : pointer to input stream/file - assumes stream has already been successfully opened and error-checked
	*void buffer : pointer to pre-allocated array for temporary storage of bytes
	*long data1  : pointer to pre-allocated array for final, converted version of the data
	off_t parameters : parameters defining input and output
		parameters[0] = data type (0-9) uchar,char,ulong,long,uint,int,ulong,long,long,double
		parameters[1] = size of each datum (bytes)
		parameters[2] = number of values to be read
		parameters[3] = number of values actually read (overwritten by this function)
	char *message : pointer to character string which is written to IF an error is encountered


RETURN VALUE:
	-1: error
	0: no data read
	1: data read

	parameters[3] will hold the actual number of data points read
	this may be less than parameters[1] (ntoread) if this is the last block of data read from the file


SAMPLE CALL:
	setdatatype=0; // unsigned char
	datasize=sizeof(unsigned char);
	ntoread=1000; // define amount of data to be read on each call
	bytestoread=(size_t)(0.5 + (ntoread*datasize));

	buffer=(void *)malloc(bytestoread); // allocate memory
	data=(long *)malloc(ntoread*sizeof(long)); // allocate memory

	parameters[0]= setdatatype;
	parameters[1]= datasize;
	parameters[2]= ntoread;
	parameters[3]= 0;

	while(!feof(fpin)) {
		x= xf_readbin1_l(fpin,buffer,data,parameters,message);
		if(x<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }
		else if(parameters[3]==0)break;
		else for(ii=0;ii<parameters[3];ii++) printf("%g\n",data[ii]);
	}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_readbin1_l(FILE *fpin,void *buffer, long *data1, off_t *parameters, char *message) {

	char *thisfunc="xf_readbin1_f\0";
	int x;
	double aa;
	off_t datatype,datasize,ntoread,nread;
	off_t ii,jj,kk,bytestoread,bytesread;

	datatype=parameters[0];
	datasize=parameters[1];
	ntoread=parameters[2];
	bytestoread=datasize*ntoread;

//TEST:fprintf(stderr,"datasize=%ld\n",datasize);
//TEST:fprintf(stderr,"ntoread=%ld\n",ntoread);
//TEST:fprintf(stderr,"bytestoread=%ld\n",bytestoread);

	/************************************************************/
	/* READ THE DATA INTO THE BUFFER */
	/************************************************************/
	bytesread= fread(buffer,1,bytestoread,fpin);

//TEST:fprintf(stderr,"bytesread=%ld\n",bytesread);

	if((bytesread%datasize)==0)	{
		nread=bytesread/datasize;
		parameters[3]=nread;
		if(nread==0) return(0);	/* if no data was read, this is not necessarily an erro - could just be end of file */
	}
	else {
		sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);
		return(-1);
	}

//TEST:fprintf(stderr,"nread=%ld\n",nread);
//TEST:fprintf(stderr,"message=%s\n",message);
//TEST:for(ii=0;ii<3;ii++) fprintf(stderr,"parameters[%ld]=%ld\n",ii,parameters[ii]);


	/************************************************************
	CONVERT NUMBERS ACTUALLY READ TO LONG
	************************************************************/
	if(datatype==0) {
		unsigned char *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==1) {
		char *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==2) {
		unsigned short *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==3) {
		short *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==4) {
		unsigned int *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==5) {
		int *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==6) {
		unsigned long *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==7) {
		long *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==8) {
		long *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}
	else if(datatype==9) {
	double *pdata = buffer;
		for(ii=0;ii<nread;ii++) data1[ii]=(long)pdata[ii];
	}

//TEST:for(ii=0;ii<nread;ii++) printf("data[%ld]: %g\n",ii,data1[ii]);

	return(1);
}
