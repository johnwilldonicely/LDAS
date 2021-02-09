/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read a binary SCORE record (RAW file format) containing a header and continuously sampled 8-bit data (EEG, LFP)
	Assumes data is stored as unsigned characters
	This version reads a single SCORE raw record, storing but not processing the header and the data

USES:

DEPENDENCY TREE: No dependencies

ARGUMENTS:
	FILE *fpin     : pointer to input stream
	char *header   : pre-allocated array to hold the block header
	size_t nheader : number of bytes (characters) in the header (typically 35)
	unsigned char *data    : pre-allocated array to hold the block data
	size_t ndata   : number of data bytes to read (sample-frequency x numbers-in-record x 1)
	char *message  : character array to hold messages, error-related or otherwise.

RETURN VALUE:
	 0: success
	-1: failure

SAMPLE CALL:
	char message[1000], header[35], infile[256];
	int samplefreq=400, duration=10, ndata=samplefreq*duration;
	while(!feof(fpin)) {
		x= xf_readscore_raw1(fpin,header,nheader,data,ndata,data,message);
		if(x<0) {fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1);)
	}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int xf_readscore_raw1(FILE *fpin, char *header, size_t nheader, unsigned char *data, size_t ndata, char *message) {

	char *thisfunc="xf_readscore_raw1\0";
	size_t nread;

	/********************************************************************************/
	/* READ HEADER */
	/********************************************************************************/
	nread= fread(header,1,nheader,fpin);
	/* failure to read any bytes does not indicate a corrupt file - merely the end of file has been reached */
	if(nread==0) return(0);
	/* however if a portion of a header has been read, something is clearly wrong */
	else if(nread<nheader){
		sprintf(message,"%s [ERROR]: bad header: less than the expected %ld bytes read",thisfunc,nheader);
		return(-1);
	}

	/********************************************************************************/
	/* READ DATA BLOCK  */
	/********************************************************************************/
	nread = fread(data,1,ndata,fpin);
	/* if an entire data block has not been read, given that a header HAS been already read on this call, there must be a problem*/
	if(nread!=ndata){
		sprintf(message,"%s [ERROR]: bad block: only %ld of expected %ld bytes read",thisfunc,nread,ndata);
		return(-1);
	}

	return(0);

}
