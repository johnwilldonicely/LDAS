/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION :
	Find the correlation of peak-values the mean multi-channel waveforms for two cells
	- this is a proxy for the similarity in the multi-channel profile for two cells (the "tetrode effect")
	- this should be more reliable than using all the samples from each channel
	- that's because there can be a lot of variability unrelated to the action potential itself
	- will ignore non-finite values within a channel

USES:
	Determine whether two cells are actually the same cell based on the similarity of their multi-channel profile

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *wave1   : array of voltage values for multi-channel waveform #1
	float *wave2   : array of voltage values for multi-channel waveform #2
	long nchan     : the number of channels in the compound-waveform (eg. 4 for a tetrode waveform)
	long spklen    : the number of samples from each channel: length(wave)= nchan*spklen
	long zero      : the within-channel sample-number representing the peak (zero-offset, must be <spklen)
	char *message  : a string array to hold a status message

RETURN VALUE:
	on success: Pearson's r
	on failure: NAN

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double xf_wavecor2_f(float *wave1, float *wave2, long nchan, long spklen, long zero, char *message) {

	char *thisfunc="xf_wavecor2_f\0";
	long ii,jj,kk,mm,nn,goodsamps;
	double aa,bb;
	double SUMx,SUMy,SUMx2,SUMy2,SUMxy;
	double SSx,SSy,SPxy,r;

	/* Make sure zero is within-range */
	if(zero>=spklen) { sprintf(message,"%s [ERROR]: zero-sample (%ld) must be less than spklen (%ld)",thisfunc,zero,spklen); return(NAN); }

	/* Accumulate sums (across channels) for each wave at time-zero for that channel */
	SUMx=SUMy=SUMx2=SUMy2=SUMxy=0.00;
	goodsamps=0;
	for(ii=0;ii<nchan;ii++) {
		aa= (double)wave1[zero];
		bb= (double)wave2[zero];
		if(isfinite(aa)&&isfinite(bb)) {
			SUMx+= aa;
			SUMy+= bb;
			SUMx2+= aa*aa;
			SUMy2+= bb*bb;
			SUMxy+= aa*bb;
			goodsamps++;
		}
		zero+= spklen;
	}

	/* If there are no good values, there can be no correlation */
	if(goodsamps==0) return(NAN);

	/* Calculate Pearson's r */
	SSx = SUMx2-((SUMx*SUMx)/goodsamps);
	SSy = SUMy2-((SUMy*SUMy)/goodsamps);
	SPxy = SUMxy-((SUMx*SUMy)/goodsamps);
	if(SSy==0.0) r=0.0; /* if data describes horizontal line... */
	else if(SSx==0.0) r=0.0;  /* if data describes vertical line (or single point) ... */
	else r = SPxy/(sqrt(SSx)*sqrt(SSy)); /* otherwise, if there is variability in both x and y... */

	/* Set message and return */
	sprintf(message,"%s [OK]",thisfunc);
	return(r);
}
