/*
<TAGS>file </TAGS>

DESCRIPTION:
	Write a simple binary stream from memory to file, regardless of the data format
	This version accepts a pointer to the output stream - suitable for multiple calls to write

USES:
	Simple binary file writing

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpout     : pointer to output stream
	void *data0     : pointer to array holding data, cast as (void *) by the calling function
	size_t nn       : number of elements in data0
	size_t datasize : byte-size of data0, as defined by the calling function, usually using sizeof())
	char *message   : an array to hold diagnostic messages on return

RETURN VALUE:
	0 on success, -1 on error

SAMPLE CALL:
		i= xf_writebin1_v(stdout,(void *)data1,nn,sizeof(float),message);
		if(i!=0) { fprintf(stderr,"\n\t--- %s\n\n",message); exit(1); }
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_writebin1_v(FILE *fpout, void *data0, size_t nn, size_t datasize, char *message) {

	char *thisfunc="xf_writebin1_v\0";
	size_t ii,bytestowrite= nn*datasize;


	/* WRITE THE DATA IN ONE CHUNK AND CLOSE THE FILE */
	ii= fwrite(data0,(size_t)bytestowrite,1,fpout);

	if(ii==1) {
		sprintf(message,"%s: successfully wrote %ld bytes",thisfunc,bytestowrite);
		return(0);
	}
	else {
		sprintf(message,"%s [Error]: problem writing binary output (errno=%d)",thisfunc,ferror(fpout));
		return(-1);
	}
}
