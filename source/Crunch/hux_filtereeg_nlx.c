/************************************************************************
Filter Neuralynx EEg records - output filtered version of file
	- EEG occurring during behaviourally filtered epochs is set to zero
	- this way, the data has a null impact on power spectra
	- also, there are no "gaps" in the data to confuse other programs
	- function returns the number of "filtered" EEG points set to zero
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "neuralynx.h"

void hux_error(char message[]);

int hux_filtereeg_nlx (
					   char *infile,	/* eeg input filename */
					   char *outfile,	/* filtered eeg output filename*/
					   double *pos_time, /* array of position timestamps*/
					   int postot,		/* total number of position samples in array */
					   char *pos_filter /* array of values indicating whether position data passed behavioural filter (0) or not (1) */
					   )
{
	struct CRRec EEGRec; /* declare Neuralynx EEG structure of type "CRRec" */
	int timedivisor=1000000,headerlength=16384; /* set Neuralynx parameters */
	char c,message[256];
	int i=0,posrec=0,filter=0,EEGSize,filtertot=0;
	double eeg_time=0.0;
	FILE *fpin,*fpout;
	EEGSize=sizeof(struct CRRec); /* define size of video record structure */

	if((fpin=fopen(infile,"rb"))==NULL) {sprintf(message,"hux_filtereeg_nlx: can't open \"%s\"",infile);hux_error(message);}
	if((fpout=fopen(outfile,"wb"))==NULL) {sprintf(message,"hux_filtereeg_nlx: can't open \"%s\"",outfile);hux_error(message);}

	/* copy header, unaltered, to output file */
	i=0;while(fread(&c,sizeof(char),1,fpin)!=0 && i<headerlength) {fputc(c,fpout);i++;}
	fseek(fpin,headerlength,SEEK_SET);
	/* set filter to last pos_filter setting before first eeg record */
	fread(&EEGRec,EEGSize,1,fpin);
	eeg_time = (double) (EEGRec.qwTimeStamp)/timedivisor;
	while(pos_time[posrec]<eeg_time) {
		filter=pos_filter[posrec];
		posrec++;
	    if(posrec>=postot) hux_error("hux_filtereeg_nlx: no position records beyond start of EEG records");
	}
	fseek(fpin,headerlength,SEEK_SET);
	/* now, read eeg until time exceeds next pos sample - when it does, set new filter value */
	while (fread(&EEGRec,EEGSize,1,fpin)!=0) {
		eeg_time = (double) (EEGRec.qwTimeStamp)/timedivisor; 
		if(eeg_time>pos_time[posrec]) {
			while(pos_time[posrec]<eeg_time && posrec<postot) {
				filter=pos_filter[posrec];
				posrec++;
			}
		}
		if(filter==1) {for(i=0;i<512;i++) EEGRec.snSamples[i]=0; filtertot++;}
		fwrite(&EEGRec,EEGSize,1,fpout);
	}

	fclose(fpin);
	fclose(fpout);
	return (filtertot*512); /* total filtered EEG samples */
}
