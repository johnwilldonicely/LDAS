/*
<TAGS>file </TAGS>

DESCRIPTION:
	Create BINX format binary output from an array in memory
	BINX format:
		1. 500-byte header in 3 parts - the header is generated only once
			a. Filetype
			b. Parameters
			c. Text description
		2. The data, all of one type

	Calling function defines the data-size, data-type, and number of elements (depth) in up to 3 dimensions

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpin : pointer to output stream/file
	void *data : pointer to the array of numbers
	size_t *params : the file parameters
		params[0] : headersize, filled by this function on first call. NOTE: calling function should initialize to 0 on first call to indicate to THIS function that the header still needs to be written
		params[1] : data-size - i.e. the number of bytes in each datum (1,2,4 or 8)
		params[2] : data-type (0-9 = uchar,char,ushort,short,uint,int,ulong,long,float,double)
		params[3] : depth of dimension-1 - if data is 1d, this is also the total data-points
		params[4] : depth of dimension-2
		params[5] : depth of dimension-3
	size_t ntowrite : number of data to write. NOTE: may be diffrerent from total data in file as indicated in header, if function is called multiple times to add data
	char *message : an array to hold diagnostic messages on return

RETURN VALUE:
	data-points written, or 0 on error

SAMPLE CALL:
--------------------------------------------------------------------------------
	Suppose we have an array of short integers acquired by sampling 16 recording
	channels 1000 times. Acquisition systems will typically interlace the data
	from each channel, and the total samples will number 16,000.

	In this instance the data shouldbe treated as 2-dimensional, with a depth of
	16 in dimension-0, and a depth of 1000 in dimension-1

	char outfile[256];
	char message[256];
	size_t params[6],z;

	sprintf(outfile,"stdout");
	fpout=fopen(infile,"wb");

	dsize=sizeof(short);
	params[1]=2; 	# 2-bytes per datum
	params[2]=3; 	# datatype short
	params[3]=16;	# 1st dimension (channel) is 16 elements deep
	params[4]=1000;	# 2nd dimension (sample-number) is 1000 elements deep
	params[5]=0;	# 3rd dimension undefined

	nn = xf_writebinx1(fpout,data,params,message);

	fclose(fpout);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BINX_HEADERSIZE 500

size_t xf_writebinx1(FILE *fpout, void *data, size_t *params, size_t ntowrite, char *message) {

	char *thisfunc="xf_writebinx1\0";
	size_t ii,jj,kk,nn,mm,nparams,datasize,datatype,hsize1,hsize2,hsize3,depth1,depth2,depth3;
	char filetype[BINX_HEADERSIZE];	/* array to hold file-type */
	char describe[BINX_HEADERSIZE];	/* array to hold file-description */

	//for(ii=0;ii<6;ii++) fprintf(stderr,"params[%d]=%d\n",ii,params[ii]);

	/********************************************************************************
	1. CREATE THE HEADER IF PARAMS[0] (HEADER-SIZE) == 0. THIS IS THE FIRST CALL TO
	THIS FUNCTION SO SET PARAMS[0], CHECK VALIDITY, AND WRITE THE HEADER
	********************************************************************************/
	if(params[0]==0) {

		params[0] = BINX_HEADERSIZE;

		/* set headersize, determine requirements for filetype, prameters and description */
		nparams = 6; /* elements in the params array */
		hsize1 = 32 ; /* amount of header allocated for ASCII filetype descriptor */
		hsize2 = nparams * sizeof(size_t); /* amount of header allocated for parameters */
		hsize3 = BINX_HEADERSIZE - ( hsize1  + hsize2); /* remainder of header allocated for ASCII description of file structure */
 		ii = hsize1+hsize2+hsize3 ;
 		if(ii>BINX_HEADERSIZE) {
 			sprintf(message,"%s [Error]: BINX_HEADERSIZE (%d bytes) is too small to accommodate header contents (%ld bytes) - recode required!!",thisfunc,BINX_HEADERSIZE,ii);
 			return(0);
 		}

		/* make aliases for convenience */
		datasize = params[1];
		datatype = params[2];
		depth1 = params[3];
		depth2 = params[4];
		depth3 = params[5];

		/* check datasize (datasize) is 1,2,4 or 8 */
		if(datasize!=1 && datasize!=2 && datasize!=4 && datasize!=8) {
			sprintf(message,"%s [Error]: invalid datasize (datasize = %ld) - must be 1,2,4 or 8",thisfunc,datasize);
			return(0);
		}
		/* check datatype (datatype) is between 0 and 9 */
		if(datatype<0 || datatype>9) {
			sprintf(message,"%s [Error]: invalid datatype (datatype = %ld) - must be 0-9",thisfunc,datatype);
			return(0);
		}
		/* check defined dimensions are assigned depths (params [3,4,5]) & determine total datapoints */
		nn = depth1 * depth2 * depth3;
		if(nn<1) {
			sprintf(message,"%s [Error]: parameters specify no data to be written (n = %ld x %ld x %ld = %ld)",thisfunc,depth1,depth2,depth3,nn);
			return(0);
		}

		/* create the filetype portion of the header */
		sprintf(filetype,"BINX_V1________________________");
		if(strlen(filetype)>hsize1) {
			sprintf(message,"%s [Error]: length of filetype (%ld) exceeds limit (%ld)",thisfunc,strlen(filetype),hsize1);
			return(0);
		}

		/* create the descriptive ascii portion of the header - note - if any changes are made to BINX_HEADERSIZE, hsize1 or nparams, update this! */
 		sprintf(describe,"\nBYTES:   TYPE   DESCRIPTION\n");
		ii=1;
		kk=sizeof(size_t);
		jj=ii+(hsize1-1); sprintf(describe,"%s%03ld-%03ld: (char) file type\n",describe,ii,jj); ii+=hsize1;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) header length\n",describe,ii,jj); ii+=kk;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) bytes in each datum (1|2|4|8)\n",describe,ii,jj); ii+=kk;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) data type (0-9 = uchar,char,ushort,short,uint,int,usize_t,size_t,float,double)\n",describe,ii,jj); ii+=kk;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) depth, dim1\n",describe,ii,jj); ii+=kk;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) depth, dim2\n",describe,ii,jj); ii+=kk;
		jj=ii+(kk-1); sprintf(describe,"%s%03ld-%03ld: (size_t) depth, dim3\n",describe,ii,jj); ii+=kk;
		sprintf(describe,"%s%03ld-%03d: (char) this file description\n",describe,ii,BINX_HEADERSIZE);
		/* pad the end of the description field and rterminate with a newline + null */
		kk=strlen(describe);
		if(kk<hsize3) {
			for(ii=kk;ii<(hsize3-2);ii++) describe[ii]='-';
			describe[(hsize3-2)]='\n';
			describe[(hsize3-1)]='\0';
		}
		else {
			sprintf(message,"%s [Error]: length of description (%ld) exceeds limit (%ld)",thisfunc,kk,hsize3);
			return(0);
		}

		/* WRITE THE HEADER - note that header size (params[0]) will have been updated */
		/* write the file type  */
		//for(ii=0;ii<6;ii++) fprintf(stderr,"parameter[%d]=%d\n",ii,params[ii]);
 		mm= fwrite(filetype,sizeof(char),hsize1,fpout);
		if(mm!=hsize1) {sprintf(message,"%s [Error]: problem writing header (filetype)",thisfunc);fclose(fpout);return(0);}
		/* write the data-parameters  */
 		mm= fwrite(params,sizeof(long),nparams,fpout);
		if(mm!=nparams) {sprintf(message,"%s [Error]: problem writing header (parameters)",thisfunc);fclose(fpout);return(0);}
		/* write the ASCII description */
 		mm= fwrite(describe,sizeof(char),hsize3,fpout);
		if(mm!=hsize3) {sprintf(message,"%s [Error]: problem writing header (description)",thisfunc);fclose(fpout);return(0);}


	}


	/********************************************************************************
	2. WRITE THE DATA AS A SINGLE BLOCK - recalculate n and quick-check dimension variables
	********************************************************************************/
	nn = params[3] * params[4] * params[5];

	if(ntowrite<=nn) {

		mm = fwrite(data,(params[1]*ntowrite),1,fpout); /* params[1] = datasize, in bytes */

		if(mm==1) mm = ntowrite; /* if block was written successfully, mm = totak numbers stored */
		else {
			mm=0;
			sprintf(message,"%s [Error]: problem writing binary data",thisfunc);
		}
	}
	else {
		mm = 0;
		sprintf(message,"%s [Error]: request to write more data (%ld) than the total specified by the parameters (%ld)",thisfunc,ntowrite,nn);
	}

	return(mm);

}
