/************************************************************************
hux_readspike_axona reads axona lab ascii spike-time records
- requires spike files (eg. tr1.1, tr1.2 etc - extension = probe number)
- also looks for (requires) corresponding cluster definition files (e.g. tr1.1.cut, tr1.2.cut)
- pass pointer to array of input filenames, and float data
- creates a timestamp-sorted set of spike data arrays from all input files
- returns total number of spikes 
- NOTE!!: assumes memory has been allocated by calling function
	- first call hux_checkaxona to check...
			- spike file validity
			- presence of matching cluster file
			- that .res and .clu files have same number of records
			- to determine memory requirements
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "neuralynx.h"
#define HEADERMAX 1000

void hux_error(char message[]);
int hux_getext(char *line);
int hux_getfileheader(FILE *fpin, char *header,	char *header_stop, int headermax);
int hux_getword(char *line, char *trigger, int word, char *target);
int hux_substring(char *source,char *target,int casesensitive);
typedef union {char ch[4];int i;} swap_i32;
int i32_swap_(int input){int output;swap_i32 inp,out;inp.i=input;out.ch[3]=inp.ch[0];out.ch[2]=inp.ch[1];out.ch[1]=inp.ch[2];out.ch[0]=inp.ch[3];return out.i;}

int hux_readspike_axona (
					   char *spikefile[64],	/* array of up to 64 spike file (.res) names */
					   int spikefiletot,		/* total number of spike files */
					   int MAXPROBES,	/* should be defined in calling function - usually 100+2 */
					   int MAXCLUSTERS,	/* should be defined in calling function - usually 64+2 */
					   double *spike_time,	/* destination array for spike times */
					   int *spike_probe, 	/* destination array for probe IDs */
					   int *spike_clust,	/* destination array for cluster IDs */
					   unsigned int *spikecount, /* array to hold spike counts for each probe & cluster */
					   int *probe_file_id		/* file number associated with each original probe number */
					   )
{
char line[1000],temp_str[256],cfile[256],header[HEADERMAX],header_stop[64];
int i,x,y,headerlength,recsize,filesleft,spiketot=-1,minmarker,*endfile,*temp_cluster,*probeid;
int num_chans=-1,bytes_per_timestamp=-1,samples_per_spike=-1,bytes_per_sample=-1,*num_spikes;
unsigned long int min_time,*temp_time,timedivisor;
float time_interval;
FILE **fpspk,**fpclu;
int i32_swap_(int input);

/* define structure for reading spike records - 4x(4+50) bytes per record */
struct AxonaTetrodeProto	{ 
	unsigned long t1; 
	char w1[50];
	unsigned long t2; 
	char w2[50]; 
	unsigned long t3; 
	char w3[50]; 
	unsigned long t4; 
	char w4[50]; 
	} *AxonaTetrode;

recsize=sizeof(AxonaTetrodeProto);
AxonaTetrode = (AxonaTetrodeProto *) malloc(MAXPROBES*recsize); 
if(AxonaTetrode==NULL) hux_error("hux_readspike_axona: problem allocating memory");

sprintf(header_stop,"data_start\0");

endfile = (int *) malloc(MAXPROBES*sizeof(int));
temp_time = (unsigned long int *) malloc(MAXPROBES*sizeof(unsigned long int)); /* to hold spike times */
temp_cluster = (int *) malloc(MAXPROBES*sizeof(int));	/* to hold cluster numbers */
probeid = (int *) malloc(MAXPROBES*sizeof(int));
num_spikes = (int *) malloc(MAXPROBES*sizeof(int));
fpspk = (FILE **) malloc(MAXPROBES*sizeof(FILE *)); /* array of file pointers to spike files */
fpclu = (FILE **) malloc(MAXPROBES*sizeof(FILE *)); /* array of file pointers to cluster (cut) files */
if(endfile==NULL||temp_time==NULL||temp_cluster==NULL||probeid==NULL||fpspk==NULL||fpclu==NULL) hux_error("hux_readspike_axona: problem allocating memory");

filesleft=spikefiletot;

/* open all tetrode files and set file pointers to start of data */
for(x=0;x<spikefiletot;x++) {
	probeid[x]=atoi(spikefile[x]+hux_getext(spikefile[x])); /* probe no. = spike file extension - zero if there is no .[number] extension */
	probe_file_id[probeid[x]]=x; /* set file number associated with each probe number */
	endfile[x]=0; /* set end status for each file to "not finished yet" */
	fpspk[x]=fopen(spikefile[x],"rb");
	if(fpspk[x]==NULL) {sprintf(temp_str,"hux_readspike_axona: can't open spiek file \"%s\"\n\n",spikefile[x]);hux_error(temp_str);}
	headerlength=hux_getfileheader(fpspk[x],header,header_stop,HEADERMAX)*sizeof(char);
	if(headerlength==-1) {sprintf(temp_str,"hux_readspike_axona: no file header ending in %s",header_stop); hux_error(temp_str);}
	hux_getword(header,"timebase",1,temp_str); timedivisor=atoi(temp_str);
	hux_getword(header,"num_chans",1,temp_str); num_chans=atoi(temp_str);
	hux_getword(header,"bytes_per_timestamp",1,temp_str); bytes_per_timestamp=atoi(temp_str);
	hux_getword(header,"samples_per_spike",1,temp_str); samples_per_spike =atoi(temp_str);
	hux_getword(header,"bytes_per_sample",1,temp_str); bytes_per_sample=atoi(temp_str);
	hux_getword(header,"num_spikes",1,temp_str); num_spikes[x]=atoi(temp_str);

	/* create filename of cluster (.cut) file which corresponds to the given spike file */
	strcpy(cfile,spikefile[x]);
	sprintf(cfile,"%s.cut\0",spikefile[x]);
	fpclu[x]=fopen(cfile,"r");
	if(fpclu[x]==NULL) {sprintf(temp_str,"hux_readspike_axona: can't open cluster file \"%s\"\n\n",cfile);hux_error(temp_str);}

	while(fgets(line,1000,fpclu[x])!=NULL) {if(hux_substring(line,"Exact_cut_for",0)!=-1) break;}
	hux_getword(line,"Exact_cut_for:",3,temp_str); /* determine how many total probe-spikes the cluster file reports*/
	if(atoi(temp_str)!=num_spikes[x]) hux_error("hux_readspikes_axona: spike and cut files report different spike counts");
	/* now the file pointer should be at the first cluster definition */

	//??? TO DO: in hux_checkaxona, make sure number of records in cluster file exact cut list is appropriate
}

spiketot=-1;
filesleft=spikefiletot;
time_interval=1.0/(float)timedivisor;

/* read first record of each spike and cluster file to set potential minimum time value */
for(x=0;x<spikefiletot;x++) {
	/* read first spike file record */
	fread(&AxonaTetrode[x],recsize,1,fpspk[x]);
	temp_time[x] = i32_swap_(AxonaTetrode[x].t1);
	/* read first cluster record */
	if(fscanf(fpclu[x],"%d",&temp_cluster[x])!= 1) hux_error("hux_readspike_axona: could not read first cluster record");
	min_time = temp_time[x]+1; /* set feasible minimum value */
	minmarker=x;
}

/* Now main loop begins - note that previously read spike record will be first to be assigned to permanent array */
while(filesleft>0) {
	/* see which temp file has lowest time index (starting min value from last itteration) & set minmarker */
	for(x=0;x<spikefiletot;x++) if(endfile[x]==0 && temp_time[x]<min_time) {min_time=temp_time[x];minmarker=x;}
	/* store appropriate values in spike arrays */
	spiketot++; /* first will be zero */
	spike_time[spiketot] = temp_time[minmarker]*time_interval; /* convert timestamp to seconds */ 
	spike_probe[spiketot] = probeid[minmarker];
	spike_clust[spiketot] = temp_cluster[minmarker];
	spikecount[spike_probe[spiketot]*MAXCLUSTERS + spike_clust[spiketot]]++;

	/* for file with smallest time record, scan next record and set new (probably inaccurate) minimum value */
	if(fread(&AxonaTetrode[minmarker],recsize,1,fpspk[minmarker])==1) {	
		temp_time[minmarker] = i32_swap_(AxonaTetrode[minmarker].t1);
		fscanf(fpclu[minmarker],"%d",&temp_cluster[minmarker]);
		min_time=temp_time[minmarker];
	}

	/* if that was the last record for the "minimum" .res and .clu files, close them and find new minimum */
	else {
		endfile[minmarker]=1;
		filesleft--;
		fclose(fpspk[minmarker]); 	
		fclose(fpclu[minmarker]);
		for(y=0;y<spikefiletot;y++) if(endfile[y]==0)	{min_time=temp_time[y];minmarker=y;} /* find a potential min value */
		for(y=0;y<spikefiletot;y++) if(endfile[y]==0 && temp_time[y]<min_time) {min_time=temp_time[y];minmarker=y;} /* determine the actual min value */
	}
}

for(x=0;x<spikefiletot;x++) fclose(fpspk[x]);
free(temp_time);
free(temp_cluster);
free(endfile);
free(probeid);
free(num_spikes);
free(fpspk);
free(fpclu);
free(AxonaTetrode);

spiketot+=1;
return (spiketot);
}
