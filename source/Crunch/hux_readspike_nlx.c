/************************************************************************
hux_readspike_nlx reads Neuralynx spike records
- requires #include "neuralynx.h" in calling function
- pass pointer to array of input filenames, and float data
- creates a timestamp-sorted set of spike data arrays from all input files
- strategy: 
	- opens all spike files
	- read each one, looking for smallest timestamp
	- file with smallest timestamp record is stored, file pointer is advanced
	- for the other spike files, file pointer is rolled back so same data is read next time
- returns total number of spikes 
- NOTE!!: assumes memory has been allocated by calling function
	- first call hux_checknlx and allocate required memory
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "neuralynx.h"

void hux_error(char message[]);

int hux_readspike_nlx (
					   char *spikefile[64],	/* array of up to 64 spike file names */
					   int spikefiletot,		/* total number of spike files */
					   int MAXPROBES,	/* should be defined in calling function - usually 100+2 */
					   int MAXCLUSTERS,	/* should be defined in calling function - usually 65 */
					   double *spike_time,	/* destination array for spike times */
					   int *spike_probe, 	/* destination array for probe IDs */
					   int *spike_clust,	/* destination array for cluster IDs */
					   unsigned int *spikecount, /* array to hold spike counts for each probe & cluster */
					   int *probe_file_id			/* file number associated with each original probe number */
					   )
{
int x,y,z,size,headerlength,timedivisor,filesleft,spiketot,minmarker,channelnumber,*endfile;
long long min,*data;
struct TTRec *TetrodeRec; /* declare tetrode type record structure */
FILE **fpin;

endfile = (int *) malloc(MAXPROBES*sizeof(int));
data = (long long int *) malloc(MAXPROBES*sizeof(long long int));
TetrodeRec = (TTRec *) malloc(MAXPROBES*sizeof(TTRec));
fpin = (FILE **) malloc(MAXPROBES*sizeof(FILE *));
if(endfile==NULL||data==NULL||TetrodeRec==NULL||fpin==NULL) hux_error("hux_readspike_csicsvari_asc: problem allocating memory");

timedivisor=1000000; /* Neuralynx timestamps are in microseconds units */
headerlength=16384; /* size of ascii header on binary Neuralynx files */
size=sizeof(struct TTRec); /* define size of tetrode record structure */

/* open all tetrode files and set file pointers to start of binary data*/
for(x=0;x<spikefiletot;x++) {
	fpin[x]=fopen(spikefile[x],"rb");
	if(fpin[x]==NULL) {
		fprintf(stderr,"\n\t--- Error in hux_readspike_nlx: can't open \"%s\"\n\n",spikefile[x]);
		fclose(fpin[x]);
		return(-1);
	}
	else fseek(fpin[x],headerlength,SEEK_SET); /* skip to binary data in spike file */
}

spiketot=-1;
filesleft=spikefiletot;
for(x=0;x<spikefiletot;x++) endfile[x]=0; /* this variable gets set to "1" for each file as reading is completed */

while(filesleft>0) {
	for(x=0;x<spikefiletot;x++) { /* read all tetrode files */

		/* if this file has already been finished, skip it */
		if(endfile[x]==1) continue; 
		
		/* otherwise read a value from this file - if it's invallid, consider the file finished */ 
		if(fread(&TetrodeRec[x],size,1,fpin[x])==0) { 
			endfile[x]=1; 
			filesleft--; 
			continue;	
		}
		/* if the value is good, make the timestamp a potential minimum */
		else {
	
			min = data[x] = TetrodeRec[x].qwTimeStamp; 	/* set a minimum timestamp value that is at least valid if not accurate */
			channelnumber=TetrodeRec[x].dwScNumber; 	/* identify probe number associated with this file */
	
			if(min<0 || channelnumber<0 || channelnumber>MAXPROBES) {
				fprintf(stderr,"\n\t--- Error in hux_readspike_nlx: invalid data in %s at record %d\n\n",spikefile[x],spiketot);
				endfile[x]=1;
				filesleft--; 
				spiketot--;
				continue;
			}
			else {
				minmarker = x;
				probe_file_id[ channelnumber ]=x; 
			}
		}
	}
	
	if(filesleft==0) break;

	/* now identify the smallest timestamp amongst open tetrode files*/
	for(x=0;x<spikefiletot;x++) {
		if(endfile[x]==1) continue;
		if(data[x]<=min) {min=data[x]; minmarker = x;}
	}
	
	/* rewind file pointers for other files*/
	for(x=0;x<spikefiletot;x++) if(x!=minmarker) fseek(fpin[x],-1*size,SEEK_CUR);

	spiketot++; /* increment grand total spikecount */
	spike_time[spiketot] = (double) (data[minmarker])/timedivisor;
	spike_probe[spiketot] = TetrodeRec[minmarker].dwScNumber;
	spike_clust[spiketot] = TetrodeRec[minmarker].dwCellNumber;
	spikecount[spike_probe[spiketot]*MAXCLUSTERS + spike_clust[spiketot]]++;
}

for(x=0;x<spikefiletot;x++) fclose(fpin[x]);

free(data);
free(fpin);
free(endfile);
free(TetrodeRec);

spiketot+=1;
return (spiketot);
}

