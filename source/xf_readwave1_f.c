/*
<TAGS>file</TAGS>
DESCRIPTION:
	- Read .wfm files (multi-channel spike waveform means for clustered data)
	- updated: 6.February.2017 [JRH]
	- assumes file has header something like this...

		PROBE 0
		N_CHANNELS 16
		CHANNEL_LIST 7,10,6,8,4,11,5,9,3,12,1,14,2,13,0,15
		SAMPLES_PER_CHANNEL 40
		SAMPLES_PRE_PEAK 8
		SAMPLE_RATE 19531.25
		WAVES_START

	- ...with one row per cluster to follow...

		id   count  v[1]  v[2]  v[3]  v[4] ... v[n]

	- ... where n= N_CHANNELS x SAMPLES_PER_CHANNEL, and the order is defined by CHANNEL_LIST


DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *infile    : input, name of .wfm file to read
	short **id      : output, pointer to array for cluster-ids for each waveform
	long **count    : output, pointer to array for spike-counts for each waveform
	long **data     : output, pointer to array for actual waveforms
	short **chanlist: output, pointer to array for order of channels in waveform
	long *result_l  : output, array to hold statistics related to the data
	char *message   : output, a string array to hold the status message

RETURN VALUE:
	on success: the sample-rate (Hz) as read from the header
	on failure: -1

	- also...

	- data[] will be assigned memory and filled with mean multi-channel waveforms
	- count[] will record the number of orignal waveforms contributing to the mean
	- id[] will hold the cluster-id for each waveform
	- chanlist[] will hold, in depth-order, the original channels contributing to the waveforms
	- result_l[] will hold information on the data
		result_l[0]= total waveforms read
		result_l[1]= number of channels contributing to each compound waveform
		result_l[2]= the number of samples corresponding to the waveform on a single channel
		result_l[3]= the number of samples preceding the peak
		result_l[4]= the total samples in the multi-channel waveform
		result_l[5]= largest cluster-number
		result_l[6]= the probe-number

SAMPLE CALL:

	short *cluid=NULL,*chanlist=NULL;
	long *count=NULL,result_l[16];
	float *data=NULL;

	samprate= xf_readwave1_f(filename,&cluid,&count,&data,&chanlist,result_l,message);
	if(samprate<0) { fprintf(stderr,"\b\n\t*** %s/%s\n\n",thisprog,message); exit(1); }

	for(ii=0;ii<result_l[0];ii++) printf("cluster[%ld]= %ld\n",ii,cluid[ii]);

	free(data);free(id);free(count);free(chanlist);
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

double xf_readwave1_f(char *infile, short **id, long **count, float **data, short **chanlist, long *result_l, char *message) {


	char *thisfunc="xf_readwave1_f\0",key[256],*ptempc=NULL,*pcol;
	short *pid=NULL,*pchanlist=NULL;
	int x,y,z,wavestart=0;
	long ii,jj,kk,nn,probe,clumax,clutot,wavelen,spklen,spkpre,nchan;
	long *pcount=NULL;
	float *pdata=NULL;
	double srate;
	FILE *fpin;

	if((fpin=fopen(infile,"r"))==NULL) {
		sprintf(message,"%s [ERROR]: could not open file %s",thisfunc,infile);
		return(-1);}

	/********************************************************************************/
	/* READ THE FILE HEADER */
	/********************************************************************************/
	clumax=wavelen=spklen=spkpre=srate=nchan=wavestart=-1;
	while(!feof(fpin)) {
		fscanf(fpin,"%s",&key);
		if(strcmp(key,"PROBE")==0) fscanf(fpin,"%ld",&probe);
		if(strcmp(key,"N_CHANNELS")==0) fscanf(fpin,"%ld",&nchan);
		if(strcmp(key,"CHANNEL_LIST")==0) fscanf(fpin,"%ms",&ptempc);
		if(strcmp(key,"SAMPLES_PER_CHANNEL")==0) fscanf(fpin,"%ld",&spklen);
		if(strcmp(key,"SAMPLES_PRE_PEAK")==0) fscanf(fpin,"%ld",&spkpre);
		if(strcmp(key,"SAMPLE_RATE")==0) fscanf(fpin,"%lf",&srate);
		if(strcmp(key,"WAVES_START")==0) {wavestart=1; break;}
	}
	if(spkpre==-1) { sprintf(message,"%s [ERROR]: file %s has no SAMPLES_PRE_PEAK in header",thisfunc,infile); return(-1);}
	if(spklen==-1) { sprintf(message,"%s [ERROR]: file %s has no SAMPLES_PER_CHANNEL in header",thisfunc,infile); return(-1);}
	if(srate==-1)  { sprintf(message,"%s [ERROR]: file %s has no SAMPLES_RATE in header",thisfunc,infile); return(-1);}
	if(nchan==-1)  { sprintf(message,"%s [ERROR]: file %s has no N_CHANNELS in header",thisfunc,infile); return(-1);}
	if(wavestart==-1) { sprintf(message,"%s [ERROR]: file %s has no WAVES_START in header",thisfunc,infile); return(-1);}
	if(ptempc==NULL) { sprintf(message,"%s [ERROR]: file %s has no CHANNEL_LIST in header",thisfunc,infile); return(-1);}
	//TEST:  fprintf(stderr,"kk=%ld\tptempc=%s\n",kk,ptempc);

	wavelen= nchan*spklen;

	/********************************************************************************/
	/* PARSE THE CHANNEL LIST */
	/********************************************************************************/
	pchanlist= realloc(pchanlist,nchan*sizeof(pchanlist));
	if(pchanlist==NULL) { sprintf(message,"%s [ERROR]: failed memory allocation",thisfunc); return(-1); }
	for(ii=0;(pcol=strtok(ptempc,",\n\r"))!=NULL;ii++) {
		if(ii>=nchan) { sprintf(message,"%s [ERROR]: items in CHANNEL_LIST exceeds N_CHANNELS (%ld)",thisfunc,nchan); return(-1);}
		ptempc=NULL;
		z= sscanf(pcol,"%hi",&pchanlist[ii]);
		if(z!=1) { sprintf(message,"%s [ERROR]: non-integer in channel list (%s)",thisfunc,pcol); return(-1);}
	}
	if(ii!=nchan) { sprintf(message,"%s [ERROR]: items in CHANNEL_LIST (n=%ld) does not match N_CHANNELS (%ld)",thisfunc,ii,nchan); return(-1);}
	//TEST: for(kk=0;kk<ii;kk++) fprintf(stderr,"pchanlist[%ld]=%d\n",kk,pchanlist[kk]);

	/********************************************************************************/
	/* READ THE MULTI-CHANNEL WAVEFORM FOR EACH CLUSTER */
	/********************************************************************************/
	nn=0;
	while(!feof(fpin)) {
		/* dynamic memory allocation */
		pid= realloc(pid,(nn+1)*sizeof(pid));
		pcount= realloc(pcount,(nn+1)*sizeof(pcount));
		pdata= realloc(pdata,(nn+1)*wavelen*sizeof(pdata));
		/* read the cluster ID and spike-count */
		z=fscanf(fpin,"%hi %ld",&pid[nn],&pcount[nn]);
		if(z!=2) break; // probably end of file
		/* update the max cluster id */
		if(pid[nn]>clumax) clumax=pid[nn];
		/* read the waveform data */
		for(ii=0;ii<wavelen;ii++) {
			z=fscanf(fpin,"%f",&pdata[nn*wavelen+ii]);
			if(z!=1) { sprintf(message,"%s [ERROR]: file %s ended before all waveform samples were read",thisfunc,infile); return(-1); }
		}
		nn++;
	}
	fclose(fpin);
	if(nn==0) { sprintf(message,"%s [ERROR]: no waveform records in file %s",thisfunc,infile); return(-1); }

	/********************************************************************************/
	/* ASSIGN NEWLY RESERVED MEMORY TO THE INPUT POINTERS */
	/********************************************************************************/
	(*id)= pid;
	(*count)= pcount;
	(*data)= pdata;
	(*chanlist)= pchanlist;

	/********************************************************************************/
	/* FILL THE RESULTS ARRAY */
	/********************************************************************************/
	result_l[0]=nn;      // total waveforms read
	result_l[1]=nchan;   // number of channels contributing to each compound waveform
	result_l[2]=spklen;  // the number of samples corresponding to the waveform on a single channel
	result_l[3]=spkpre;  // the number of samples preceding the peak
	result_l[4]=wavelen; // total samples in each multi-channel waveform
	result_l[5]=clumax;  // highest cluster ID
	result_l[6]=probe;   // probe number

	/********************************************************************************/
	/* FREE MEMORY FOR TEMPORARY CHANNEL LIST ASSIGNED BY FSCANF() */
	/********************************************************************************/
	free(ptempc);

	/********************************************************************************/
	/* RETURN THE SAMPLE-RATE: THE ONLY DOUBLE-FLOAT VARIABLE IN THE HEADER */
	/********************************************************************************/
	return(srate);
}
