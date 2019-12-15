/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:
	Determine the ratio of events in two symmetrical zones of a histogram
	Originally based on xf_histrefract1_d (February 2009 [JRH])
	- however in this version zero itself contributes to the negative and positive sums (zero is special)

USES:
	- determine refractoriness of cell-firing to asess quality of spike-sorting
		- neurons have a 2ms refractory period
		- typically we compare the proportion of spikes in a +-15ms window falling within a +-2ms zone
		- focusing on the inner +-15ms of the histogram ensures that bursty and non-bursty cells are treated similarly
		- a ratio of >0.08 should be used to automatically reject a histogram
		- a ratio of <0.01 represents an exceptionally clean refractory period
		- NOTE: bintot must be such as to allow at least 1ms precision (eg. 100 bins convering a 100ms range)
		- NOTE: 50-75 spikes are required in the +-15ms window to make a good decision
		- NOTE: for asymmetrical histograms (e.g. cross-corelograms) good estimates require 35-50 spikes in any half

	- autocorrelograms:
		- determine the quality of cell isolation
		- estimate of burst-firing
	- cross-correlograms:
		- monosynaptic coupling

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *histx  : input array, histogram timestamps (seconds)
	double *histy  : input array, histogram values (typically spike-counts, but doubles allowed)
	long bintot    : number of bins in the input histogram
	double z1:     : the reference zone (seconds: typically 0.015)
	double z2:     : the comparison zone (seconds: typically 0.002) - should be smaller than zone1!
	double *result : output array, must allow at least 5 elements

RETURN VALUE:
	: the proportion of zone1 events occurring in zone2
	: -1 if there are no events in zone2

*/

double xf_histratio1_d(double *histx, double *histy, long bintot, double z1, double z2, double *result) {

	long ii;
	double aa=0.0,bb=0.0,cc=0.0,dd=0.0,ee=0.0,tt,ratio;

	for(ii=0;ii<bintot;ii++) {
		tt=histx[ii];
		ee+=histy[ii];
		if( tt >= -z1 && tt <= 0.0 ) aa+= histy[ii];
		if( tt <=  z1 && tt >= 0.0 ) bb+= histy[ii];
		if( tt >= -z2 && tt <= 0.0 ) cc+= histy[ii];
		if( tt <=  z2 && tt >= 0.0 ) dd+= histy[ii];
	}
	result[0]=(float)aa; // count in negative zone1 window
	result[1]=(float)bb; // count in positive zone1 window
	result[2]=(float)cc; // count in negative zone2 window
	result[3]=(float)dd; // count in positive zone2 window
	result[4]=(float)ee; // total spike count in histogram

	/* calculate the ratio (proportion of zone1 in zone2) */
	if((aa+bb)>0.0) ratio=(float)((cc+dd)/(aa+bb));
	else ratio=(-1.0);

	return(ratio);
}
