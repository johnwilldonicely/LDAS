/* Calculate statistics on cell firing rate map pattern...

Pi = probability of being n bin i, = dwell time in bin i divided by trial length
Ri = firing rate in bin i
R = mean firing rate over entire trial

- cohererence of place cell firing (how well does one bin rate predict the neighbouring bin rate)
	coherence = z-transform of correlation between binrates and the mean of surrounding binrates,  

- information content (how well does cell firing predict location - high info = small fields)
	= SUM {Pi*(Ri/R) log2*(Ri/R)}
	NOTE: log2(x) = log(x)/log(2)

- sparsity is roughly equivalent to the proportion of the environment the cell is likely to be active in
	= (SUM{Pi*Ri})^2 * SUM(Pi*Ri^2)
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

void hux_error(char message[]);
void hux_linearstats (float* array,int arraysize,float missing,float *result);
int hux_correlate(float *x, float *y, int N, int invalid, float *tempresult);

void hux_ratestats (
	float *dwell,	/* dwelltime array */
	float *rate,	/* firing rate array */
	int bintot,		/* width of array (total elements = bintot*bintot) */
	float dwelltot,	/* total duration of time in all "valid" bins */
	int ztransp,	/* "precision" (iterations) for z-transform calculation - zero = no z-transform at all (9-11 recommended) */
	int invalid,	/* invalid value for firing rates and averages. NOTE: if(dwell<=0) rate should be invalid!!*/
	float *result	/* array of float to hold outputs - should point to 32-element array */
	)

{
	int i,w,x,y,z=0,n=0,N=0,N2=0,p1,p2,xbin,ybin;
	float *avg,r=0.0,info=0.0,temprate,tempresult[32],baserate=0.0,meanrate=0.0, sdrate=0.0, medianrate=0.0, skewrate=0.0, peakrate=0.0, maxrate=0.0;
	double a=0.0,SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0,SSx=0.0,SSy=0.0,SPxy=0.0;
	double dwellratio,rateratio,s1=0.0,s2=0.0,info_content,coherence,sparsity;

	avg = (float *) malloc(1+bintot*bintot*sizeof(float)); /* assign memory for temporary average array */
	if(avg==NULL) hux_error("hux_ratestats: insufficient memory");
	N=bintot*bintot;
	if(N<=0) hux_error("hux_ratestats: inappropriate specification of number of bins");

	/* calculate basic rate statistics */
	hux_linearstats(rate,N,invalid,tempresult);
	N2 = (int) tempresult[0]; /* this is the number of bins with valid firing rates (dwelltime > 0) */
	meanrate = tempresult[1];
	sdrate = tempresult[3];
	medianrate = tempresult[7];
	baserate = tempresult[13];
	skewrate = tempresult[10];
	peakrate = tempresult[19]; // 97.5th percentile = midpoint of 95th percentile
	maxrate = tempresult[20];

	info_content = coherence = sparsity = 0.0;
	if(meanrate>0) {
		for(ybin=0;ybin<bintot;ybin++) {
			for(xbin=0;xbin<bintot;xbin++)	{
				p1=ybin*bintot+xbin; /* set pointer to central (current) bin */
				if(dwell[p1]<=0) {avg[p1]=invalid;continue;}
				temprate = rate[p1];
				rateratio = temprate/meanrate;
				dwellratio = dwell[p1]/dwelltot;
				if(rateratio>0) info_content += (dwellratio)*(rateratio)*log2(rateratio); /* add up information content */
				s1 += dwellratio*temprate;	/* numerator for sparsity calculation */
				s2 += dwellratio*temprate*temprate; /* denominator for sparsity calculation */
				
				/* create the avg array - average rate of up to 8 bins around central bin (omit unvisited bins) */
				n=0; avg[p1]=0.00; w=xbin-1; z=xbin+1;
				/* do top row */
				y=ybin-1; if(y>=0) {for(x=w;x<=z;x++) {p2 = y*bintot+x; if(dwell[p2]>0) {avg[p1]+=rate[p2]; n++;}}}
				/* do bottom row */
				y=ybin+1; if(y<bintot) {for(x=w;x<=z;x++) {p2 = y*bintot+x; if(dwell[p2]>0) {avg[p1]+=rate[p2]; n++;}}}
				/* do left side (xbin-1 = w)*/
				if(w>=0) {p2= ybin*bintot+w; if(dwell[p2]>0) {avg[p1]+=rate[p2]; n++;}}
				/* do right side (xbin+1 = z)*/
				if(z<bintot) {p2 = ybin*bintot+z; if(dwell[p2]>0) {avg[p1]+=rate[p2]; n++;}}
				if(n>0)	avg[p1] = avg[p1]/n;
				else avg[p1] = (float) invalid;
			}
		}

		sparsity = (s1*s1)/s2;
		/* calculate coherence (Pearson's r for correlation between binrate and avg rate*/
		hux_correlate(rate,avg,N,invalid,tempresult);
		r=tempresult[1];
		/* perform the Fisher z' transform if necessary */
		if(ztransp==0) coherence=r;
		else coherence = atanhf(r);
	}
	else info_content = sparsity = coherence = -1.0;
		
	free(avg);

	/* calculate basic rate statistics */
	result[0] = (float)N2;
	result[1] = meanrate;
	result[2] = sdrate;
	result[3] = medianrate;
	result[4] = skewrate;
	result[5] = peakrate; // 97.5th percentile = midpoint of 95th percentile
	result[6] = (float)info_content;
	result[7] = (float)sparsity;
	result[8] = (float)coherence;
	result[9] = (float)dwelltot;
	result[10] = maxrate;
	result[11] = baserate; // 10th percentile
	return;
}
