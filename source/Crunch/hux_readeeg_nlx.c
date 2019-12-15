/************************************************************************
- read_eeg_nlx reads Neuralynx EEG records
- at this time, I believe EEG units in files are given in microovolts (May 2006)
- returns EEG sampling frequency as calculated from first 10 samples
- pass pointers to input filename, and float data
- requires #include "neuralynx.h" in calling function
- first call hux_checknlx and allocate required memory
- float is used for storage to facilitate data transformations
- eegscale can be used to invert EEG, if a value of -1 is chosen 
- NOTE!!: assumes memory has been allocated by calling function
*************************************************************************/
/*

TO DO:
	- rename function xf_readnlx_CSC
	- use long-int
	- remove error reporting - return -1 is sufficient


*/

#include <stdlib.h>
#include <stdio.h>
#include "neuralynx.h"

int hux_readeeg_nlx ( char *infile, double *eeg_time, float *eeg_val, float eegscale, float *result) {
 
	struct CRRec EEGRec; /* declare EEG structure of type "CRRec" */
	int EEGSize=sizeof(struct CRRec); /* define size of video record structure */
	
	int i,recordtot=0,eegtot=-1,timedivisor=1000000,validsamples=0;
	int s_ui64=sizeof(unsigned __int64),	s_ui32=sizeof(unsigned __int32), s_si16=sizeof(signed __int16);
	long headerlength=16384; /* size of ascii header on binary Neuralynx files */
	float eegfreq=-1.0;
	double time0=0.0,timeprev=0.0,interval=0.0;
	FILE *fpin;
	
	fpin=fopen(infile,"rb");
	if(fpin==NULL) {fprintf(stderr,"\n** ERROR in hux_readeeg_nlx: can't open \"%s\"\n\n",infile);return(-1);}

	/* read first 2 records to determine sample interval */
	/* very consistent throughout trial and more accurate than SampleFrequency variable in the header */	
	fseek(fpin,headerlength,SEEK_SET);
	for(i=0;i<2 && fread(&EEGRec,EEGSize,1,fpin)!=0;i++)  {
		time0 = (double) EEGRec.qwTimeStamp/timedivisor;
		validsamples = EEGRec.dwNumValidSamples;
		timeprev=time0;				
	}
	interval = (double)(time0-timeprev)/validsamples; 		

	/* now go back and read data */	
	fseek(fpin,headerlength,SEEK_SET);
	while (fread(&EEGRec,EEGSize,1,fpin)!=0) {
		recordtot++;
		time0 = (double) (EEGRec.qwTimeStamp)/timedivisor; 
		for(i=0;i<validsamples;i++) {
			eegtot++;
			eeg_time[eegtot] = (double) time0 + i*interval;
			eeg_val[eegtot] = eegscale * (float) EEGRec.snSamples[i];
		}
	}
	
	fclose(fpin);

	result[0]=(float) recordtot; /* number of EEG records */
	result[1]=(float) validsamples; /* number of EEG samples per record */
	result[2]=(float) 1.0/interval; /* EEG record sampling rate - accounts for number of valid samples per record */
	return (eegtot); /* eegtot = recordtot*validsamples */

}
