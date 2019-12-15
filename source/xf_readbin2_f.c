/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read simple binary file into memory, converting to float
	fseeko is used to determine memory requirements, so DO NOT USE WITH STDIN
	This version handles file-opening and conversion to float internally
	If you want flexibility in what the data is converted to, use xf_readbin1_v instead

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpin : pointer to input stream/file
	off_t *parameters : pointer to an array of parameters defining behaviour (see below)
	char *message : an array to hold diagnostic messages on return

		parameters[0] = data type (0-9) uchar,char,ushort,short,uint,int,ulong,long,float.double
		parameters[1] = number of bytes at the top of the file (header) to ignore
		parameters[2] = number of numbers to skip (bytes skipped calculated based on size of data type)
		parameters[3] = number of numbers to be read
			- if set to zero, function attempts to read all bytes after header and skipped numbers
			- if set to zero, will be reset to the number of numbers read before the function finishes

RETURN VALUE:
	pointer to float array holding numbers, or NULL on error

SAMPLE CALL:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float *xf_readbin2_f(char *infile, off_t *parameters, char *message) {

	char *thisfunc="xf_readbin2_f\0";
	void *data0=NULL;
	float *data1=NULL;
	off_t ii,jj,kk,filesize,datasize,bytesavailable,startbyte,bytestoread=0;
	FILE *fpin;
	off_t datatype=parameters[0];
	off_t headerbytes=parameters[1];
	off_t start=parameters[2];
	off_t ntoread=parameters[3];
	parameters[3]=0; /* if successful, this will be udated to hold the number of bytes actually read */

	/* DETERMINE DATA SIZE */
	if(datatype==0||datatype==1) datasize=(off_t)sizeof(char);
	else if(datatype==2||datatype==3) datasize=(off_t)sizeof(short);
	else if(datatype==4||datatype==5) datasize=(off_t)sizeof(int);
	else if(datatype==6||datatype==7) datasize=(off_t)sizeof(long);
	else if(datatype==8) datasize=(off_t)sizeof(float);
	else if(datatype==9) datasize=(off_t)sizeof(double);
	else {
		sprintf(message,"%s [Error]: invalid data-type parameter %ld, must be 0-9",thisfunc,datatype);
		return(NULL);
	}

	if(strcmp(infile,"stdin")==0) {
		sprintf(message,"%s [Error]: this function should not be used with standard input (stdin)",thisfunc);
		return(NULL);
	}

	/* OPEN THE BINARY FILE */
	if((fpin=fopen(infile,"rb"))==0) {
		sprintf(message,"%s [Error]: file \"%s\" not found",thisfunc,infile);
		return(NULL);
	}


	/* GET TOTAL FILE SIZE, BYTES AVAILABLE AFTER STARTBYTE (bytesavailable), AND NUMBER OF BYTES TO ACTUALLY READ */
	fseeko(fpin,0L,SEEK_END); // go to end of file
	filesize=ftello(fpin);    // remember final position (bytes)
	rewind(fpin);             // go back to start



	/* DETERMINE HOW MANY DATA-BYTES ARE AVAILABLE IN THE FILE, AFTER SKIPPING THE HEADER AND THE SPECIFIED AMOUNT OF DATA */
	startbyte= headerbytes + (start*datasize);
	if(filesize>startbyte) {
		bytesavailable= filesize-startbyte;
		/* check bytesavailable is a multiple of datasize */
		if(bytesavailable%datasize != 0) {
			sprintf(message,"%s [Error]: data-type does not match file size (%ld), or specified header-size (%ld) is incorrect ",thisfunc,filesize,headerbytes);
			fclose(fpin);
			return(NULL);
		}
	}
	else {
		sprintf(message,"%s [Error]: header-size + start setting exceeds file size ",thisfunc);
		fclose(fpin);
		return(NULL);
	}



	/* DETERMINE HOW MANY DATA-BYTES TO ACTUALLY READ */
	if(ntoread==0) {
		bytestoread=bytesavailable;
		ntoread=bytestoread/datasize;
	}
	else {
		bytestoread= ntoread*datasize;
		if(bytestoread>bytesavailable) {
			sprintf(message,"%s [Error]: bytes to read (%ld) must not exceed (file size) - (start byte) (%ld) ",thisfunc,startbyte,(filesize-startbyte));
			fclose(fpin);
			return(NULL);
		}
	}

/* TEST
fprintf(stderr,"filesize=%ld\n",filesize);
fprintf(stderr,"bytesavailable=%ld\n",bytesavailable);
fprintf(stderr,"startbyte=%ld\n",startbyte);
fprintf(stderr,"bytestoread=%ld\n",bytestoread);
*/

	/* ALLOCATE MEMORY FOR TEMPORARY ARRAY */
	if((data0=(void *)realloc(data0,bytestoread))==NULL) {
		sprintf(message,"%s [Error]: insufficient memory",thisfunc);
		fclose(fpin);
		return(NULL);
	}


	/* SKIP THE SPECIFIED NUMBER OF BYTES */
	if(fseeko(fpin,startbyte,SEEK_SET)!=0) {
		sprintf(message,"%s [Error]: problem reading binary file (errno=%d)",thisfunc,ferror(fpin));
		fclose(fpin);
		free(data0);
		return(NULL);
	}

	/************************************************************
	READ THE DATA IN ONE CHUNK AND CLOSE THE FILE
	************************************************************/
	ii=fread(data0,(size_t)bytestoread,1,fpin);
	if(ii==1) {
		sprintf(message,"%s: successfully read %ld bytes",thisfunc,bytestoread);
		parameters[3]=ntoread; // reset this parameter to a valid value to reflect success
		fclose(fpin);
	}
	else {
		sprintf(message,"%s [Error]: problem reading binary file (errno=%d)",thisfunc,ferror(fpin));
		fclose(fpin);
		free(data0);
		return(NULL);
	}


	/************************************************************
	CONVERT TO FLOAT IF IT ISN'T ALREADY A FLOAT
	************************************************************/
	/* if data read is a float anyway, just return the pointer to data0, but do not free the memory! */
	if(datatype==8) {
		return((float *)data0);
	}
	/* otherwise, 1) allocate new memory, 2) define an appropriately cast pointer to data0, and copy the array */
	/* note that in this case we will return a pointer to data1, and data0 should be freed */
	else {
		if((data1=(float *)realloc(data1,ntoread*sizeof(float)))==NULL) {
			sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);
			free(data0);
			return(NULL);
		}

		else if(datatype==0) {
			unsigned char *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==1) {
			char *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==2) {
			unsigned short *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==3) {
			short *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==4) {
			unsigned int *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==5) {
			int *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==6) {
			unsigned long *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==7) {
			long *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
 		else if(datatype==8) {
			float *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}
		else if(datatype==9) {
			double *pdata = data0;
			for(ii=0;ii<ntoread;ii++) data1[ii]=(float)pdata[ii];
		}

		free(data0);
		return(data1);
	}
}
