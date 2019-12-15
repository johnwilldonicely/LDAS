/*
DESCRIPTION:
	Read Neuralynx CSC (.ncs) files into memory - continuously sampled data

DEPENDENCIES:
	neuralynx.h

ARGUMENTS:
	char *infile : name of the input file
	float eegscale : user-defined multiplier for data - can be negative to "flip" data
	float *result : pre-allocated array to hold results - must allow at least 4 elements
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	success:
		- pointer to a float array of the data values on success
		- results array will be filled as follows:

			result[0]= (float) recordtot : number of EEG records
			result[1]= (float) validsamps : number of EEG samples per record
			result[2]= (float) eegtot : number of EEG samples
			result[3]= (float) samprate : EEG record sampling rate - accounts for number of valid samples per record
	error:
		- NULL
		- message array will hold explanatory text (if any)

SAMPLE CALL:
	x= xf_auc1_d(data, nn, interval, result, );
	if(x!=0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

<TAGS> file neuralynx</TAGS>
*/


#include <stdlib.h>
#include <stdio.h>
#include "neuralynx.h"

float *xf_readnlx_ncs(char *infile, float eegscale, long *result, char *message) {

	char *thisfunc="xf_readnlx_ncs\0";
	struct CRRec EEGRec; /* declare EEG structure of type "CRRec" - defined in neuralynx.h */
	int EEGSize= sizeof(struct CRRec); /* define size of record structure */
	int x,y,z,samprate=0,samprateref=0,sizeofeeg,sampsperrec;
	uint validsamps;
	long ii,jj,kk,time0,timeref,recordtot=0,eegtot=0,headerlength;
	float *eeg_val=NULL;
	size_t memreq;
	FILE *fpin;

	headerlength=16384; /* 2Kbytes - this should be a fixed value for Neuralynx files */
	sampsperrec= 512;
	sizeofeeg= sizeof(*eeg_val);

	fpin=fopen(infile,"rb");
	if(fpin==NULL) { sprintf(message,"%s [ERROR]: can't open input \"%s\"",thisfunc,infile); return(NULL); }

	fseek(fpin,headerlength,SEEK_SET);
	while(fread(&EEGRec,EEGSize,1,fpin)!=0)  {

		samprate= EEGRec.dwSampleFreq;
		validsamps= EEGRec.dwNumValidSamples;
		// time0= EEGRec.qwTimeStamp; // unused at present - could check for gaps in recording

		/* check for consistency of sample-rate */
		if(recordtot++==0) samprateref= samprate;
		if(samprate!=samprateref) { sprintf(message,"%s [ERROR]: inconsistent sample-rate at record %ld (%d vs %d Hz)",thisfunc,recordtot,samprateref,samprate); return(NULL); }

		/* dynamically allocate memory */
		memreq= (eegtot+validsamps) * sizeofeeg;
		eeg_val= realloc(eeg_val,(size_t)memreq);
		if(eeg_val==NULL) { sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); return(NULL); }

		/* store data values */
		for(ii=0;ii<validsamps;ii++) {
			eeg_val[eegtot++]= eegscale * (float)EEGRec.snSamples[ii];
		}
	}
	fclose(fpin);

	result[0]= (float) recordtot; /* number of EEG records */
	result[1]= (float) sampsperrec; /* number of EEG samples per record */
	result[2]= (float) eegtot; /* number of EEG samples */
	result[3]= (float) samprate; /* EEG record sampling rate - accounts for number of valid samples per record */

	return(eeg_val);

}
