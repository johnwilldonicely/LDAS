/*
<TAGS>file </TAGS>
DESCRIPTION: [ 23 June 2015: JRH ]

	Read a flat binary file into memory in one or more blocks (fast!) (NOT SUITABLE FOR STDIN)
	Handles file-opening internally
		- for chunk-wise reading of data, use xf_readbin1_*
		- for internal conversion from one format to another, use xf_readbin2_*

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:

	char *infile  : name of file to read
	off_t *params : binary read parameters array
		params[0] = datasize, size of data type (bytes) - for multichannel data consider using nchans*sizeof(type)
		params[1] = headerbytes, number of bytes at the top of the file to ignore
		params[2] = nblocks,  number of blocks of data to read (set to 0 to read the entire file)
		params[3] = output, total values read, set by this function
	off_t *start  : array of [nblocks], start-record-numbers marking the beginning of each read-block
	off_t *toread : array of [nblocks], block-sizes (number of records in each block, if 0, read to file end)
	char *message : an array to hold diagnostic messages on return

RETURN VALUE:
	no errors:
		pointer to array holding numbers, needs to be cast by calling function
		params[3] overwritten by total nnumber of records read
	error:
		NULL

NOTES
	- Any error for any block will invalidate the entire result - UNDEFINED PARTIAL-FILE-READS UNSUPPORTED
	- fseeko is used to determine memory requirements- DO NOT USE WITH STDIN
	- *start and *toread are converted internally to bytes, according to [datasize]
	- *start and *toread must be initialized by the calling function to hold at least one value
	- If no errors are reported, the total number of values read will be the sum of *toread
	- If nblocks (params[2]) = 0,
		nblocks becomes 1
		start[0] becomes 0
		toread[0] becomes the (filesize-headerbytes)/datasize
	- Blocks are allowed to overlap, and any block can be set to read any part of the file
		- that is, every block-read begins with a seek to the start of the file

SAMPLE CALL: READ 3 BLOCKS OF BINARY FLOAT DATA
	char message[256], infile="data.bin";
	int datasize=sizeof(float);
	long ii, nn=0, nblocks=3, headersize=0, start[3]={0,100,200}, nrecords[3]={12,12,12};
	long params[4]={datasize,headersize,nblocks,nn}
	void *datvoid=NULL;
	float *datfloat=NULL;

	datvoid= xf_readbin3_v(infile,params,start,nrecords,message);
	if(datvoid==NULL) { fprintf(stderr,"\n\t--- Error[%s/%s]\n\n",thisprog,message); exit(1); }
	if(nn<1) {fprintf(stderr,"\n\t--- Error [%s]: .club file %s is empty\n",thisprog,clufile);exit(1);}
	nn=params[3];
	pfloat=(float *)datvoid;
	for(ii=0;ii<nn;ii++) printf("%f\n",pfloat[ii]);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xf_readbin3_v(char *infile, off_t *params, off_t *start, off_t *toread, char *message) {

	char *thisfunc="xf_readbin3_v\0";
	void *data1=NULL;
	off_t ii,jj,kk,nn,block,filesize,bytestart,bytestoread,bytesavailable,memreq,readcheck;
	off_t datasize,headerbytes,nblocks;
	FILE *fpin;

	/* store the parameters*/
	datasize=params[0];
	headerbytes=params[1];
	nblocks=params[2];
	nn=params[3]=0; // nn = datacount and pointer to read position - always reset to zero before data is read

	/* ERROR CHECKS */
	if(strcmp(infile,"stdin")==0) {
		sprintf(message,"%s: this function should not be used with standard input (stdin)",thisfunc);
		return(NULL);
	}

	/* OPEN THE BINARY FILE */
	if((fpin=fopen(infile,"rb"))==0) {
		sprintf(message,"%s: file \"%s\" not found",thisfunc,infile);
		return(NULL);
	}

	/* GET TOTAL FILE SIZE, BYTES AVAILABLE AFTER bytestart (bytesavailable), AND NUMBER OF BYTES TO ACTUALLY READ */
	fseeko(fpin,0L,SEEK_END); // go to end of file
	filesize=ftello(fpin);    // remember final position (bytes)
	bytesavailable= filesize-headerbytes; // total number of bytes available to be read

	/* CHECK THAT FILE-HEADER IS OF APPROPRIATE SIZE FOR DATA TYPE */
	if(bytesavailable%datasize != 0) {
		sprintf(message,"%s: corrupt binary file(%s), or wrong datasize(%ld) / headerbytes(%ld) specified",thisfunc,infile,datasize,headerbytes);
		fclose(fpin);
		return(NULL);
	}

	/* IF nblocks==0, ENTIRE FILE IS TO BE READ, BUILD A SINGLE "FAKE" BLOCK */
	if(nblocks==0) {
		nblocks=1;
		start[0]= 0;
		toread[0]= bytesavailable/datasize;
	}


	/* DETERMINE TOTAL MEMORY REQUIRED AND ADJUST toread[block]=0 AS REQUIRED */
	memreq=0;
	for(block=0;block<nblocks;block++) {
		/* deal with "read to end of file" blocks */
		if(toread[block]<=0) toread[block]= (bytesavailable-(start[block]*datasize)) / datasize;
		memreq+= (datasize*toread[block]);
	}

	/* ALLOCATE MEMORY */
	if((data1=(void *)realloc(data1,memreq))==NULL) {
		sprintf(message,"%s: insufficient memory",thisfunc);
		return(NULL);
	}

	/* READ EACH BLOCK INTO MEMORY */
	for(block=0;block<nblocks;block++) {

		/* calculate bytestart and bytestoread, accounting for the number of bytes to skip */
		/* if block has an undefined toread (0) redefine as "the rest of the file" */
		bytestart= headerbytes + (datasize*start[block]);
		if(toread[block]==0) toread[block]= (bytesavailable-bytestart)/datasize;
		bytestoread= (datasize*toread[block]);


		/* make sure the block falls within the file byte-range */
		if(bytestart>filesize || (bytestart+bytestoread)>filesize) {
			sprintf(message,"%s: block %ld (read %ld items starting at item %ld) exceeds filesize (%ld) ",thisfunc,block,toread[block],start[block],filesize);
			free(data1);
			data1=NULL;
			break;
		}
		/* skip the specified number of bytes - always from the beginning of the file */
		if(fseeko(fpin,bytestart,SEEK_SET)!=0) {
			sprintf(message,"%s: problem seeking to byte-%ld in block %ld of %s (errno=%d)",thisfunc,bytestart,block,infile,ferror(fpin));
			free(data1);
			data1=NULL;
			break;
		}
		/* read the block into data1, starting at position nn, in one chunk  */
		readcheck= fread((data1+(datasize*nn)),bytestoread,1,fpin);
		if(readcheck!=1){
			sprintf(message,"%s: problem reading %ld bytes in block %ld in %s (errno=%d)",thisfunc,bytestoread,block,infile,ferror(fpin));
			free(data1);
			data1=NULL;
			break;
		}
		/* update data1 pointer an total data count */
		nn+=toread[block];
	}

	/* CLOSE THE INPUT FILE */
	fclose(fpin);

	params[3]=nn; // set params[3] to return the number of data points actually read

	return(data1);
}
