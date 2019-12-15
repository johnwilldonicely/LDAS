/*
<TAGS>file </TAGS>
DESCRIPTION:

	Read simple binary stream into memory from a file
	This version accepts a pointer to the input stream - suitable for repeat calls and stdin

	On each call:
		- calling function passes a pointer to a pre-allocated void buffer, which holds the input
		- a pointer of the appropriate data-type is then pointed at the buffer
		- parameters[2] reports the number of data read

	NOTE:
		To read an entire file with one function call, (~5 times faster), use xf_readbin2_v
		That function allows the system to determine the optimal block read size
		However, because it also requires pre-determining the size of the input, it is not suitable for reading from piped streams

USES:
	Reading a binary input stream of unknown size in chunks
	Suitable for reading binary input from stdin


DEPENDENCY TREE:
	No dependencies


ARGUMENTS:
	FILE *fpin : pointer to input stream/file - assumes stream has already been successfully opened and error-checked
	*void buffer : pointer to pre-allocated array for temporary storage of bytes
	off_t parameters : parameters defining input and output
		parameters[0] = size of each datum (bytes)
		parameters[1] = number of values to be read
		parameters[2] = output, number of values actually read (overwritten by this function)
	char *message : pointer to character string which is written to IF an error is encountered


RETURN VALUE:
	-1: error
	0: success

	parameters[2] will hold the actual number of data points read
	this may be less than parameters[1] (ntoread) if this is the last block of data read from the file


SAMPLE CALL:
	void *buffer=NULL; // storage for data from each call
	unsigned short *data=NULL; // pointer to buffer assuming input is ushort
	off_t datasize=sizeof(unsigned int); // size of each datum in bytes
	off_t blocksize=1000; // define amount of data to be read on each call
	if((buffer=(void *)realloc(buffer,(blocksize*datasize)))==NULL) {fprintf(stderr,"\n--- Error[%s]: insufficient memory\n\n",thisprog);exit(1);};

	params[0]=datasize
	params[1]=blocksize;
	while(!feof(fpin)) {
		x= xf_readbin1_v(fpin,buffer,params,message);
		if(x<0)	{fprintf(stderr,"\n*** %s/%s\n\n",thisprog,message); exit(1);}
		nread=params[2];
		if(nread==0) break;
		data=(unsigned short *)buffer;
		for(ii=0;ii<nread;ii++) printf("%hu\n",data[ii]);
	}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_readbin1_v(FILE *fpin, void *buffer, off_t *parameters, char *message) {

	char *thisfunc="xf_readbin1_v\0";
	off_t datatype,datasize,ntoread,nread;
	off_t ii,jj,kk,bytestoread,bytesread;

	datasize=parameters[0];
	ntoread=parameters[1];
	bytestoread=datasize*ntoread;
	/************************************************************/
	/* READ THE DATA INTO THE BUFFER */
	/************************************************************/
	bytesread= fread(buffer,1,bytestoread,fpin);

	if((bytesread%datasize)==0)	{
		nread=bytesread/datasize;
		if(nread==0) return(0);	/* if no data was read, this is not necessarily an error - could just be end of file */
	}
	else {
		sprintf(message,"%s [ERROR]: corrupt input (bad byte-count)",thisfunc);
		return(-1);
	}
	parameters[2]=nread;
	return(0);
}
