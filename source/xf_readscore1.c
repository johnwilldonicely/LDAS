/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read a SCORE binary file containing continuously sampled data (EEG, LFP)
	Data is stored as unsigned characters

	35-byte header + 4000  bytes of data (unsigned char)

		Type 	Content			Bytes	Comment
		char 	name[9]			9
		char 	date[9]			9
		char 	time[9]			9
		char 	channel 		1
		char 	epoch_length	1
		char	sample_rate 	1
		char	artefact_def	1
		char 	score			1
		char 	EMG 			1
		short	temp			2


USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile			: name of the file to read
	unsigned char **data	: pointer for an unsigned character array initialized to *data=NULL, and passed as &data
	int headers				: output headers to stdout (0=NO, 1=YES)
	char *message			: character array to hold messages, error-related or otherwise.

RETURN VALUE:
	The number of data-point read for each of the channels - multiply by nchans to get the total data read
	Zero on failure

SAMPLE CALL:

	char message[1000], infile[256];
	int nchans=4;
	size_t n;
	unsigned char *data1=NULL;

	sprintf(infile,"mydata.txt");

	n = xf_readscore1_s(infile,&data1,0,message);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t xf_readscore1(char *infile, unsigned char **data, int headers, char *message) {

	char *thisfunc="xf_readscore1\0";
	char *header=NULL, *ctemp=NULL;
	unsigned char *buffer1=NULL;
	short *buffer2=NULL;
	int x,y,z,blocksize,nchans,nread,ntoread;
	int year,month,day,hour,minute,second;
	size_t headersize,datasize,ii,jj,kk,mm,nn;
	off_t aaa,filesize;
	FILE *fpin;

	/********************************************************************************
	DEFINE PARAMETERS FOR SCORE FILES
	********************************************************************************/
	datasize=sizeof(unsigned char);
	headersize=35;
	blocksize=4000;
	nchans=1;
	ntoread=nchans*blocksize;

	/********************************************************************************
	OPEN THE INPUT FILE
	********************************************************************************/
	if((fpin=fopen(infile,"rb"))==0) {
		sprintf(message,"%s [Error]: file \"%s\" not found",thisfunc,infile);
		return(0);
	}

	/*******************************************************************************
	DETERMINE TOTAL FILE SIZE, INCLUDING HEADERS
	********************************************************************************/
	aaa=ftello(fpin);			// remember current position
	fseeko(fpin,0L,SEEK_END); 	// go to end of file
	filesize=ftello(fpin);		// remember final position (bytes)
	fseeko(fpin,0L,aaa); 		// go back to original position


	/********************************************************************************
	ALLOCATE SUFFICIENT MEMORY
	Note: for header and buffer1 the datum size can be assumed to be 1 byte
	********************************************************************************/
	if((header=(char *)realloc(header,headersize))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		return(0);
	}
	if((buffer1=(unsigned char *)realloc(buffer1,filesize))==NULL) {
		sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
		free(header);
		return(0);
	}

	/********************************************************************************
	START SCANNING THE FILE
	Data is stored in buffer1 in the order it is read - ie. possibly interlaced for
	multi-channel input. "m" holds the total number of samples from all channels,
	while "n" holds the number of parallel samples. m=n if nchans=1.
	********************************************************************************/
	/* initialize m (block-counter) and n (total data read) */
	mm=nn=(size_t)0;
	/* if "headers only" output is selected, output column headers to stderr */
	if(headers==1) printf("date    \ttime    \tname\tchan\tlength\trate\tart\tscore\tEMG\n");
	/* read the file */
	while(!feof(fpin)) {
		/* read header */
		fread(header,datasize,headersize,fpin);
		/* if "headers only" output is selected, output fields - convert date to yy:mm:dd format */
		if(headers==1) {
			month=atoi(header+9);
			day=atoi(header+12);
			year=atoi(header+15);
			hour=atoi(header+18);
			minute=atoi(header+21);
			second=atoi(header+24);
			printf("%02d:%02d:%02d\t%02d:%02d:%02d\t%s\t%d\t%d\t%d\t\"%c\"\t%d\t%d\n",
				year,month,day, hour,minute,second, header,header[27],header[28],header[29],header[30],header[31],header[32]);
		}
		/* read block into buffer1 */
		nread = fread(buffer1+nn,datasize,ntoread,fpin);
		/* increment m by 1 and n by number of data read */
		mm ++;
		nn += nread;
	}

	/* close file if it is not standard input */
	if(strcmp(infile,"stdin")!=0) fclose(fpin);

	/* n records number of parallel samples from all channels */
	nn = nn/(size_t)nchans;
	if(nn>(size_t)0){
		sprintf(message,"%s : %ld blocks read, total %ld samples from %d channels",thisfunc,mm,nn,nchans);
	}
	else {
		sprintf(message,"%s [ERROR]: no data read",thisfunc);
		free(header);
		return(0);

	}

	/* point data to memory reserved for buffer1 */
	(*data) = buffer1;

	free(header);
	return(nn);

}
