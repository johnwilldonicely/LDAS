/*
<TAGS>file </TAGS>
DESCRIPTION:
	Fast-read a binary stream into memory from a file  (NOT SUITABLE FOR STDIN)

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpin : pointer to input stream/file
	void **data : pointer to void which will be assigned memory for bytes of data
	size_t startbyte : zero-offset position at which to start reading (eg. to skip a 500-byte header, set to 500)
	size_t bytestoread : number of bytes to read (if set to zero, will read all)
	char *message : an array to hold diagnostic messages on return

RETURN VALUE:
	number of bytes read, or 0 on failure ( error recorded in message[] )

SAMPLE CALL:
--------------------------------------------------------------------------------
	#include <stdio.h>
	#include <stdlib.h>

	char infile[256];
	char message[256];
	void *data=NULL;
	size_t ii,nn;
	fpin=fopen(infile,"r");
	nn = xf_readbin2_v(fpin,&data,0,0,message);
	fclose(fpin);
	for(ii=0;ii<nn;ii++) printf("%ld	%g\n",ii,data[ii]);
	free(data)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

off_t xf_readbin2_v(FILE *fpin, void **data, off_t startbyte, off_t bytestoread, char *message) {

	void *tempdata=NULL;
	char *thisfunc="xf_readbin2_v\0";
	size_t ii;
	off_t filesize,nbytes;


	/* GET TOTAL FILE SIZE, BYTES AVAILABLE AFTER STARTBYTE (NBYTES), AND NUMBER OF BYTES TO ACTUALLY READ */
	fseeko(fpin,0L,SEEK_END); // go to end of file
	filesize=ftello(fpin);    // remember final position (bytes)
	rewind(fpin);             // go back to start

	if(filesize>startbyte) nbytes= filesize-startbyte;
	else {
		sprintf(message,"%s ( start byte (%ld) must be less than file size (%ld) )",thisfunc,startbyte,filesize);
		return(0);
	}
	if(bytestoread==0) bytestoread=nbytes;
	else if(bytestoread>nbytes) {
		sprintf(message,"%s ( bytes to read (%ld) must not exceed (file size) - (start byte) (%ld) )",thisfunc,startbyte,(filesize-startbyte));
		return(0);
	}

	/* test
	fprintf(stderr,"filesize=%ld\n",filesize);
	fprintf(stderr,"nbytes=%ld\n",nbytes);
	fprintf(stderr,"startbyte=%ld\n",startbyte);
	fprintf(stderr,"bytestoread=%ld\n",bytestoread);
	fprintf(stderr,"ii=%ld\n",ii);
	*/


	/* ALLOCATE MEMORY */
	if((tempdata=(void *)realloc(tempdata,bytestoread))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		return(0);
	}

	/* SKIP THE SPECIFIED NUMBER OF BYTES */
	if(fseeko(fpin,startbyte,SEEK_SET)!=0) {
			sprintf(message,"%s [Error]: problem reading binary file (errno=%d)",thisfunc,ferror(fpin));
			return(0);
	}

	/* READ THE DATA IN ONE CHUNK */
	ii=fread(tempdata,(size_t)bytestoread,1,fpin);
	if(ii!=1) {
		sprintf(message,"%s [Error]: problem reading binary file (errno=%d)",thisfunc,ferror(fpin));
		return(0);
	}
	else sprintf(message,"%s: successfully read %ld bytes",thisfunc,bytestoread);

	/* CLOSE FILE, POINT *DATA AT THE FILLED MEMORY AND RETURN nn */
	(*data)=tempdata;
	return(bytestoread);

}
