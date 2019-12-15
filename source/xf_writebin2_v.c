/*
<TAGS>file </TAGS>

DESCRIPTION:
	Write a simple binary stream from memory to file
	This version handles file-opening and closing, and can handle any data type

USES:
	Simple binary file writing

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	outfile         : output file name or "stdout"
	void *data0     : pointer to array holding data, cast as (void *) by the calling function
	size_t nn       : number of elements in data0
	size_t datasize : byte-size of data0, as defined by the calling function, usually using sizeof())
	char *message   : an array to hold diagnostic messages on return

RETURN VALUE:
	0 on success, -1 on error

SAMPLE CALL:
		i= xf_writebin2_v("stdout",(void *)data1,nn,sizeof(float),message);
		if(i!=0) { fprintf(stderr,"\b\n\t*** %s\n\n",message); exit(1); }
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_writebin2_v(char *outfile, void *data0, size_t nn, size_t datasize, char *message) {

	char *thisfunc="xf_writebin2_v\0";
	size_t ii,bytestowrite;
	FILE *fpout;

	/* DETERMINE TOTAL NUMBER OF BYTES TO WRITE */
	bytestowrite = nn*datasize;

	/* OPEN THE BINARY FILE */
	if(strcmp(outfile,"stdout")==0) fpout=stdout;
	else if((fpout=fopen(outfile,"wb"))==0) {
		sprintf(message,"%s [Error]: (file \"%s\" could not be opened for writing)",thisfunc,outfile);
		return(-1);
	}

	/* WRITE THE DATA IN ONE CHUNK AND CLOSE THE FILE */
	ii= fwrite(data0,(size_t)bytestowrite,1,fpout);
	if(strcmp(outfile,"stdout")!=0) fclose(fpout);


	if(ii==1) {
		sprintf(message,"%s: successfully wrote %ld bytes to %s",thisfunc,bytestowrite,outfile);
		return(0);
	}
	else {
		sprintf(message,"%s [Error]: problem writing binary file %s (errno=%d)",thisfunc,outfile,ferror(fpout));
		return(-1);
	}
}
