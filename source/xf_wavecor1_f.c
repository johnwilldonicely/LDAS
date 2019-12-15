/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION :
	Find the mean cross-channel waveform correlation for a multi-channel waveform
	- correlation calculated for every possible pair of channels, and average is taken
	- will ignore non-finite values within a channel

USES:
	A measure of the degree to which the mena waveform for a clas of action-potentials is noise
	- that is, noise tends to be common to all channels, so the correlation between channels will be very high (>0.99)

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	float *wave    : array of voltage values representing a multi-channel waveform
	long nchan     : the number of channels in the compound-waveform (eg. 4 for a tetrode waveform)
	long spklen    : the number of samples from each channel: length(wave)= nchan*spklen
	char *message  : a string array to hold a status message

RETURN VALUE:
	on success: mean Pearson's r
	on failure: NAN


*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double xf_wavecor1_f(float *wave, long nchan, long spklen, char *message) {

	char *thisfunc="xf_wavecor1_f\0";
	long ii,jj,kk,mm,nn,goodsamps,goodpairs;
	float *wavep1,*wavep2;
	double aa,bb;
	double SUMx,SUMy,SUMx2,SUMy2,SUMxy;
	double SSx,SSy,SPxy,r,rsum;

	sprintf(message,"%s [OK]",thisfunc);

	goodpairs=0;
	rsum=0.0;

	for(ii=0;ii<nchan;ii++) {
		wavep1= wave+ ii*spklen;
 		for(jj=(ii+1);jj<nchan;jj++) {
			wavep2= wave+ jj*spklen;

			SUMx=SUMy=SUMx2=SUMy2=SUMxy=0.00;
			SSx=SSy=SPxy=0.00;

			goodsamps=0;
			for(kk=0;kk<spklen;kk++) {
				aa=(double)wavep1[kk];
				bb=(double)wavep2[kk];
				if(!isfinite(aa)||!isfinite(bb)) continue;
				SUMx+= aa;
				SUMy+= bb;
				SUMx2+= aa*aa;
				SUMy2+= bb*bb;
				SUMxy+= aa*bb;
				goodsamps++;
			}
			if(goodsamps==0) continue;
			SSx = SUMx2-((SUMx*SUMx)/goodsamps);
			SSy = SUMy2-((SUMy*SUMy)/goodsamps);
			SPxy = SUMxy-((SUMx*SUMy)/goodsamps);

			if(SSy==0.0) r=0.0; /* if data describes horizontal line... */
			else if(SSx==0.0) r=0.0;  /* if data describes vertical line (or single point) ... */
			else r = SPxy/(sqrt(SSx)*sqrt(SSy)); /* otherwise, if there is variability in both x and y... */

			rsum+= r; // accumulate correlation for this channel-pair
			goodpairs++; // record the number of good channel-pairs
	}}

	if(goodpairs>0) {
		return(rsum/(double)goodpairs);
	}
	else {
		sprintf(message,"%s [ERROR]: no valid data for any channel-pairs",thisfunc);
		return(NAN);
	}
}
