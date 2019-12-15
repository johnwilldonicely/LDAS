/************************************************************************
hux_readspike_csicsvari_asc reads Csicsvari lab ascii spike-time records
- requires spike-times file (.res)
- also looks for (requires) corresponding .clu (cluster definition) files
- pass pointer to array of input filenames, and float data
- creates a timestamp-sorted set of spike data arrays from all input files
- returns total number of spikes 
- NOTE!!: assumes memory has been allocated by calling function
	- first call hux_checkcsicsvari to check...
			- .res file validity
			- presence of matching .clu file
			- that .res and .clu files have same number of records
			- to determine memory requirements
- NOTE: for this datatype probe_maxclusters is set by contents of .clu files
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "neuralynx.h"

void hux_error(char message[]);
int hux_getext(char *line);
int hux_substring(char *source,char *target,int casesensitive);

int hux_readspike_csicsvari_asc (
					   char *spikefile[64],	/* array of up to 64 spike file (.res) names */
					   int spikefiletot,	/* total number of spike files */
					   int MAXPROBES,	/* should be defined in calling function - usually 100+2 */
					   int MAXCLUSTERS,	/* should be defined in calling function - usually 64+2 */
					   double *spike_time,	/* destination array for spike times in seconds */
					   int *spike_probe, 	/* destination array for probe IDs */
					   int *spike_clust,	/* destination array for cluster IDs */
					   unsigned int *spikecount,	/* array to hold spike counts for each probe & cluster */
					   int *probe_file_id,	/* file number associated with each original probe number */
					   int *probe_maxcluster,	/* maximum cluster number for each probe */
					   float spikefreq		/* spike sampling frequency, to convert times to seconds */
					   )
{
char command[256],cfile[256];
int i,x,y,filesleft,spiketot=-1,minmarker,*endfile,*temp_cluster,*probeid;
unsigned long int min,*temp_time;
double samp_interval;
FILE **fpres,**fpclu;

endfile = (int *) malloc(MAXPROBES*sizeof(int));
temp_time = (unsigned long int *) malloc(MAXPROBES*sizeof(unsigned long int)); /* to hold spike times */
temp_cluster = (int *) malloc(MAXPROBES*sizeof(int));	/* to hold cluster numbers */
probeid = (int *) malloc(MAXPROBES*sizeof(int));
fpres = (FILE **) malloc(MAXPROBES*sizeof(FILE *)); /* array of file pointers to .res files */
fpclu = (FILE **) malloc(MAXPROBES*sizeof(FILE *)); /* array of file pointers to .clu files */
if(endfile==NULL||temp_time==NULL||temp_cluster==NULL||probeid==NULL||fpres==NULL||fpclu==NULL) hux_error("hux_readspike_csicsvari_asc: problem allocating memory");

samp_interval= (double)(1.0/(double)spikefreq);
filesleft=spikefiletot;

/* open all tetrode files and set file pointers to start of data*/
for(x=0;x<spikefiletot;x++) {
	fpres[x]=fopen(spikefile[x],"r");
	if(fpres[x]==NULL) {sprintf(command,"hux_readspike_csicsvari_asc: can't open \"%s\"\n\n",spikefile[x]);hux_error(command);}

	/* create filename of .clu file which corresponds to the given .res file */
	strcpy(cfile,spikefile[x]);
	i=hux_substring(cfile,".res",0);
	cfile[i+1]='c';cfile[i+2]='l';cfile[i+3]='u';
	fpclu[x]=fopen(cfile,"r");
	if(fpclu[x]==NULL) {sprintf(command,"hux_readspike_csicsvari_asc: can't open \"%s\"\n\n",cfile);hux_error(command);}
	probeid[x]=atoi(cfile+hux_getext(cfile)); /* probe number is value of extension on .clu file - zero if there is no .[number] extension */
	probe_file_id[probeid[x]]=x; /* set file number associated with each probe number */
	endfile[x]=0;
}

/* read first record of each .res and .clu file - set potential minimum value */
for(x=0;x<spikefiletot;x++) {
	/* read first .res file record */
	if(fscanf(fpres[x],"%li",&temp_time[x])!= 1) hux_error("hux_readspike_csicsvari_asc: could not read first .res record");
	/* read first (extra) record in .clu file - not a cluster number, but indicates total number of clusters */
	if(fscanf(fpclu[x],"%d",&probe_maxcluster[probeid[x]])!= 1) hux_error("hux_readspike_csicsvari_asc: could not read .clu cluster-count record");
	/* read first proper .clu file record */
	if(fscanf(fpclu[x],"%d",&temp_cluster[x])!= 1) hux_error("hux_readspike_csicsvari_asc: could not read first .clu record");
	min = temp_time[x]+1; /* set feasible minimum value */
	minmarker=x;
}

/* Now main loop begins */
while(filesleft>0) {
	/* see which temp file has lowest time index (starting min value from last itteration) & set minmarker */
	for(x=0;x<spikefiletot;x++) if(endfile[x]==0 && temp_time[x]<min) {min=temp_time[x];minmarker=x;}
	/* store appropriate values in spike arrays */
	spiketot++; /* first will be zero */
	spike_time[spiketot] = (double)temp_time[minmarker]*samp_interval; /* temp_time is just a 20 KHz sample number */
	spike_probe[spiketot] = probeid[minmarker];
	spike_clust[spiketot] = temp_cluster[minmarker];
	spikecount[spike_probe[spiketot]*MAXCLUSTERS + spike_clust[spiketot]]++;

	/* for file with smallest time record, scan next record and set new (probably inaccurate) minimum value */
	if(fscanf(fpres[minmarker],"%d",&temp_time[minmarker])==1) {
		fscanf(fpclu[minmarker],"%d",&temp_cluster[minmarker]);
		min=temp_time[minmarker];
	}

	/* if that was the last record for the "minimum" .res and .clu files, close them and find new minimum */
	else {
		endfile[minmarker]=1;
		filesleft--;
		fclose(fpres[minmarker]); 	
		fclose(fpclu[minmarker]);
		for(y=0;y<spikefiletot;y++) if(endfile[y]==0)	{min=temp_time[y];minmarker=y;} /* find a potential min value */
		for(y=0;y<spikefiletot;y++) if(endfile[y]==0 && temp_time[y]<min) {min=temp_time[y];minmarker=y;} /* determine the actual min value */
	}
}

free(endfile);
free(temp_time);
free(temp_cluster);
free(probeid);
free(fpres);
free(fpclu);

spiketot+=1;
return (spiketot);
}
