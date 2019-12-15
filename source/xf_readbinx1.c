/*
<TAGS>file </TAGS>
DESCRIPTION:
	Read BINX format binary input from a file or stdin
	BINX format:
		1. 500-byte header in 3 parts - the header is generated only once
			a. Filetype
			b. Parameters
			c. Text description
		2. The data, all of one type

	Because any data-type may be read, calling function should define pointers to all types for potential use

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	FILE *fpin : pointer to input stream/file
	void **data : pointer to void which will be assigned memory for bytes of data
	long *params : an array of the file parameters obtained from othe file header
		params[0] : header-size
			NOTE: calling function should initialize to 0 on first call to
			indicate to THIS function that the header still needs to be read
			NOTE: initializing to 1 will result only a summary of the header being output
		params[1] : data-size - i.e. the number of bytes in each datum (1,2,4 or 8)
		params[2] : data-type (0-9 = uchar,char,ushort,short,uint,int,ulong,long,float,double)
		params[3] : depth of dimension-1 - if data is 1-dimensional, this is also the total data-points
		params[4] : depth of dimension-2
		params[5] : depth of dimension-3

	char *message : an array to hold diagnostic messages on return

RETURN VALUE:
	(unsigned long) number of data points read, or 0 on error

SAMPLE CALL:
--------------------------------------------------------------------------------

	char infile[256];
	char message[256];
	void  *data0=NULL;
	float *data1=NULL;
	unsigned long i,j,k,n,m,params[6];
	FILE *fpin

	sprintf(infile,"myfile.dat");

	// read the data
	fpin=fopen(infile,"r");
	params[0] = -1;
	n = xf_readbinx1(fpin,&data0,params,message);
	fclose(fpin);

	// set up some aliases for easy reading - lets say we know the data is 2-dimensional
	unsigned long datatype = params[2];
	unsigned long ndim1 = params[3];
	unsigned long ndim2 = params[4];

	// set appropriate pointer , convert data0 to float
	short *p3;
	int *p5;
	long *p7;
	if(datatype==3) { p3 = data0; for(i=0;i<n;i++) data1[i]=(float)p3[i]; } // short
	if(datatype==5) { p5 = data0; for(i=0;i<n;i++) data1[i]=(float)p5[i]; } // int
	if(datatype==7) { p7 = data0; for(i=0;i<n;i++) data1[i]=(float)p7[i]; } // long

	// outut the data and the column (i) and row (j) labels
	for(i=j=k=0;i<n;i++) {
		printf("%d\t%d\t%g\n",j,k,data1[i]);
		if(++j == ndim1) { j=0; if(++k ==ndim2) k=0; }
	}

	// free the memory
	free(data0)
	free(data1)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BINX_HEADERSIZE 500

size_t xf_readbinx1(FILE *fpin,void **data, size_t *params, char *message) {

	void *tempdata=NULL;
	char *thisfunc="xf_readbinx1\0";
	char filetype[BINX_HEADERSIZE];	/* array to hold file-type */
	char describe[BINX_HEADERSIZE];	/* array to hold file-description */
	int headerout=0;		/* output header only? set to 1 (yes) if params[0] = -2 */
	size_t ii,jj,kk,nn,mm,nparams,headersize,datasize,datatype,depth1,depth2,depth3,hsize1,hsize2,hsize3;

	nn=depth1=depth2=depth3= 0;

	/********************************************************************************
	READ THE HEADER in 3 parts (FILETYPE,PARAMS,DESCRIPTION) if it hasn't already been read!
	********************************************************************************/
	if(params[0]<2) {

		if(params[0]==1) { headerout=1; }

		/* set headersize, determine requirements for filetype, prameters and description */
		nparams = 6; /* elements in the params array */
		hsize1 = 32 ; /* amount of header allocated for ASCII filetype descriptor */
		hsize2 = nparams * sizeof(size_t); /* amount of header allocated for parameters */
		hsize3 = BINX_HEADERSIZE - ( hsize1  + hsize2); /* remainder of header allocated for ASCII description of file structure */
 		ii = hsize1+hsize2+hsize3 ;
 		if(ii>BINX_HEADERSIZE) {
 			sprintf(message,"%s [Error]: BINX_HEADERSIZE (%d bytes) is too small to accommodate header contents (%d bytes) - recode required!!",thisfunc,BINX_HEADERSIZE,ii);
 			return(0);
 		}

		/* read the file type  */
		mm= fread(filetype,sizeof(char),hsize1,fpin);
		if(mm!=hsize1) {sprintf(message,"%s [Error]: problem reading header (filetype)",thisfunc); nn=0; goto END1;}
		/* read the data-parameters - will set headersize/params[0] */
		mm= fread(params,sizeof(unsigned long),nparams,fpin);
		if(mm!=nparams) {sprintf(message,"%s [Error]: problem reading header (parameters)",thisfunc); nn=0; goto END1;}
		/* read the ASCII description */
		mm= fread(describe,sizeof(char),hsize3,fpin);
		if(mm!=hsize3) {sprintf(message,"%s [Error]: problem reading header (description)",thisfunc); nn=0; goto END1;}

		/* make aliases */
		datasize = params[1];
		datatype = params[2];
		depth1 = params[3];
		depth2 = params[4];
		depth3 = params[5];

		/* check filetype */
		if(strstr(filetype,"BINX")==NULL) {
			sprintf(message,"%s [Error]: input is not BINX format ",thisfunc);
			nn=0; goto END1;
		}
		/* check datasize is 1,2,4 or 8 */
		if(datasize!=1 && datasize!=2 && datasize!=4 && datasize!=8) {
			sprintf(message,"%s [Error]: invalid datasize (%d) - must be 1,2,4 or 8",thisfunc,datasize);
			nn=0; goto END1;
		}
		/* check datatype is between 0 and 9 */
		if(params[2]<0 || params[2]>9) {
			sprintf(message,"%s [Error]: invalid params[2] (params[2] = %d) - must be 0-9",thisfunc,params[2]);
			nn=0; goto END1;
		}
		/* check defined dimensions are assigned depths (params [3,4,5]) & determine total datapoints */
		nn = depth1 * depth2 * depth3;
		if(nn<1) {
			sprintf(message,"%s [Error]: input is empty (header only)",thisfunc);
			nn=0; goto END1;
		}

		if(headerout==1) {
			printf("FileType: %s\n",filetype);
			printf("nHeader: %ld\n",params[0]);
			printf("DataSize: %ld\n",params[1]);
			printf("DataType: %ld\n",params[2]);
			printf("Depth1: %ld\n",params[3]);
			printf("Depth2: %ld\n",params[4]);
			printf("Depth3: %ld\n",params[5]);
			printf("Description: %s\n",describe);
			//nn=0; while(fread(&k,(params[1],1,fpin)==1) nn++; printf("total %d\n",nn); nn=0;
			goto END1;
		}

	}

	/********************************************************************************
	ALLOCATE MEMORY & READ THE DATA AS A SINGLE BLOCK
	********************************************************************************/
	if((tempdata=(void *)realloc(tempdata,params[1]*nn))==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); nn=0; goto END1; }

	nn = depth1 * depth2 * depth3;
	mm= fread(tempdata,(params[1]*nn),1,fpin); /* params[1] = datasize, in bytes */

	if(mm!=1) { sprintf(message,"%s [Error]: problem reading binary data (errno=%d)",thisfunc,ferror(fpin)); nn=0; goto END1; }

	/********************************************************************************
	CLOSE FILE, POINT *DATA AT THE FILLED MEMORY AND RETURN nn
	********************************************************************************/
END1:
	(*data)=tempdata;

	//float *p1=tempdata; for(i=0;i<10;i++) printf("*** %g\n",p1[i]);

	return(nn);

}
