/************************************************************************
Calculate instantaneous firing rate (irate), interspike intervals (ISI), and presence of burst firing
irate calculated using using "2xtheta" averaging window (8Hz, total 0.250 ms)
NOTE: this MUST be done before any reassignment of spikes to cluster zero due to behavioural filter
NOTE: cell burstiness is calculated post-behavioural-filtering
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
void hux_error(char message[]);

void hux_irate (
	int findspike,	/* unique cell id to analyse */
	long int spiketot,	/* total number of  spikes */
	int *spike_id,	/* input arary: unique cell id for spike */
	double *spike_time,	/* input arary: time of spike */
	float set_burst,	/* maxmimum isi to consider a spike a part of a burst (typically 0.006 or 0.010 seconds */
	float *spike_isi,	/* output array: inter-spike interval for each spike */
	int *spike_burst,	/* output array: number of spike in a burst */
	float *spike_irate	/* output array: instantaneous firing rate at time of spike*/
			   )
{
	long i,j,count=0,prev=0;
	int burstcount=0;
	spike_isi[0] = spike_irate[0] = -1.0; spike_burst[0] = -1;	
	for(i=1;i<spiketot;i++) { /* NOTE: must start at "1" to prevent indexing error */
		if(spike_id[i]!=findspike) continue;
		count++; /* count number of identified spikes in current cluster */
		spike_isi[i]=-1.0; 
		spike_burst[i]=-1;
		spike_irate[i]=0.0;
		/* Calculate ISI and determine position of spike in a burst */
		if(count>0) spike_isi[i]=spike_time[i]-spike_time[prev]; /* prev = index to the previous spike from this cluster */
		if(spike_isi[i]<=set_burst) {burstcount++; spike_burst[prev]=burstcount;} /* After Ranck, 1973, & Harris & Hirase, 2001 */
		else { /* if interval is too long... */
			if(burstcount>0) spike_burst[prev]=burstcount+1; /* ..but a burst has ended, assign spike-in-burst number to previous spike */
			burstcount=0; /* reset spikes-in-burst counter to zero */
		}
		prev=i; /* set index to "previous" spike to current spike */
		/* Now calculate instantaneous firing rate, averaged over 0.25 seconds (2x interval for 8 Hz theta) */

		for(j=i-1;j>=0;j--) {
			if(spike_id[j]!=findspike) continue;
			else if((spike_time[i]-spike_time[j])>0.125) break;
			else spike_irate[i]++;			
		}
		for(j=i+1;j<spiketot;j++) {
			if(spike_id[j]!=findspike) continue;
			else if((spike_time[j]-spike_time[i])>0.125) break;
			else spike_irate[i]++;			
		}
		spike_irate[i]=spike_irate[i]*4; /* multiplying n_spikes by 4 is faster than dividing by 0.250, = 2xtheta period */
	}
	return;
}
