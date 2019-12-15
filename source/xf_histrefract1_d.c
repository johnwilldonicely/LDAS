/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:
	Determine simple refractoriness in a histogram (% of spikes in +-2ms vs +-15ms window)

USES:
	Appropriate for making QC decisions on autocorrelograms for spike firing of a given neuron

DEPENDENCY TREE:
	None

ARGUMENTS:
	double *tsec   : input, array of timestamps for histogram (seconds)
	double *val    : input, array of values for histogram (counts, typically, but could be proportion or probability)
	long bintot    : input, the number of tsec and val pairs comprising the histogram
	double *result : input, array of values representing the result - memory must be allocated by calling function
		result[0]= proportion (0-1) of 0-2ms spikes in the 0-15ms zone
		result[1]= proportion (0-1) of 0-2ms spikes in the total histogram
		result[2]= spike count in negative 0-15ms window
		result[3]= spike count in positive 0-15ms window
		result[4]= spike count in negative 0-2ms window
		result[5]= spike count in positive 0-2ms window
		result[6]= total spike count in histogram

RETURN VALUE:
	none

NOTES:
	Focusing on spikes within +-15ms of reference spike ensures that bursty and non-bursty cells are treated similarly

	A ratio of >0.08 should be used to automatically reject a histogram
	A ratio of <0.01 represents an exceptionally clean refractory period

	Total number of bins (bintot) must allow at least 1ms precision in calculation of times
	50-75 spikes are required in the 30ms window to make a good decision
		- however, it would also suffice if half the histogram has 35-50 spikes


*/

void xf_histrefract1_d(double *tsec, double *val, long bintot, double *result) {

	long ii;
	double aa=0.0,bb=0.0,cc=0.0,dd=0.0,ee=0.0,tt;

	for(ii=0;ii<bintot;ii++) {
		tt=tsec[ii];
		ee+=val[ii];
		if(tt>-0.015 && tt<=-0.0) aa+=val[ii];
		if(tt< 0.015 && tt> 0.0) bb+=val[ii];
		if(tt>-0.002 && tt<=0) cc+=val[ii];
		if(tt< 0.002 && tt> 0) dd+=val[ii];
	}

	result[2]=(float)aa; // spike count in negative 0-15ms window
	result[3]=(float)bb; // spike count in positive 0-15ms window
	result[4]=(float)cc; // spike count in negative 0-2ms window
	result[5]=(float)dd; // spike count in positive 0-2ms window
	result[6]=(float)ee; // total spike count in histogram
	if(ee>0.0)	{
		result[1]=(float)((cc+dd)/ee); // percentage of refractory spikes in the total histogram
	}
	else result[6]=-1.0;

	if((aa+bb)>0.0) {
		result[0]=(float)((cc+dd)/(aa+bb)); // percentage of refractory spikes in the 0-15ms zone
	}
	else result[0]=(-1.0);
	return;
}
