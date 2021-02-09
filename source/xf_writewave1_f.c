/*
<TAGS>file</TAGS>

DESCRIPTION:
	- Write .wfm files (multi-channel spike waveform means for clustered wmean)

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *outfile : name of .wfm file to write
	short *id     : pointer to array for cluster-ids for each waveform
	long *wn      : pointer to array for spike-counts for each waveform
	float *wmean  : pointer to array for actual waveforms
	short *wchans : pointer to array for order of channels in waveform
	long *params  : array holding additional statistics related to the wmean
	long makez    : if >0, make a "NAN" line specifying <makez> spikes in cluster zero
	double srate  : sample rate of input
	char *message : output, a string array to hold the status message

RETURN VALUE:
	on success: the sample-rate (Hz) as read from the header
	on failure: -1

SAMPLE CALL:
	z= xf_writewave1_f(filename,&cluid,&wn,&wmean,&wchans,params,19531.25,0,message);
	if(z<0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int xf_writewave1_f(char *outfile, short *wid, long *wn, float *wmean, short *wchans, long *params, long makez, double srate, char *message) {


	char *thisfunc="xf_writewave1_f\0";
	int x,y,z,wavestart=0;
	long ii,jj,kk,nn,*pwn=NULL,probe,nchan,clutot,spklen,spkpre,wavelen;
	FILE *fpout;

	if((fpout=fopen(outfile,"w"))==NULL) {
		sprintf(message,"%s [ERROR]: could not write to file %s",thisfunc,outfile);
		return(-1);}

	if(makez<0) {
		sprintf(message,"%s [ERROR]: invalid makez (%ld): must be >=0",thisfunc,makez);
		return(-1);}

	nn= params[0];     // number of waveforms to be output
	nchan= params[1];  // number of channels per waveform
	spklen= params[2]; // number of samples per channel
	spkpre= params[3]; // number of samples before the waveform "time zero"
	probe= params[4];  // the probe number
	wavelen= spklen*nchan; // total length of the multi-channel waveform

	/* print the header */
	fprintf(fpout,"PROBE %ld\n",probe);
	fprintf(fpout,"N_CHANNELS %ld\n",nchan);
	fprintf(fpout,"CHANNEL_LIST ");
	fprintf(fpout,"%d",wchans[0]);
	for(ii=1;ii<nchan;ii++) fprintf(fpout,",%d",wchans[ii]);
	fprintf(fpout,"\n");
	fprintf(fpout,"SAMPLES_PER_CHANNEL %ld\n",spklen);
	fprintf(fpout,"SAMPLES_PRE_PEAK %ld\n",spkpre);
	fprintf(fpout,"SAMPLE_RATE %.18g\n",srate);
	fprintf(fpout,"WAVES_START\n");

	/* output (or replace) cluster-zero waveform using NAN  */
	if(makez>0) {
		fprintf(fpout,"0\t%ld\t",makez);
		fprintf(fpout,"nan");
		for(ii=1;ii<wavelen;ii++) fprintf(fpout," nan");
		fprintf(fpout,"\n");
		/* reset wn for cluster-zero so it will be skipped in the next step */
		for(ii=0;ii<nn;ii++) if(wid[ii]==0) wn[ii]=0;

	}

	/* output the waveforms in memory */
	for(ii=0;ii<nn;ii++) {
		if(wn[ii]<1) continue;
		fprintf(fpout,"%hd\t%ld\t",wid[ii],wn[ii]);
		kk=ii*wavelen; // indext to start of waveform for this cluster
		fprintf(fpout,"%.3f ",wmean[kk++]);
		for(jj=1;jj<wavelen;jj++) fprintf(fpout,"%.3f ",wmean[kk++]);
		fprintf(fpout,"\n");
	}
	fclose(fpout);

	return(0);
}
