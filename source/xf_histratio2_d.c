/*
<TAGS>stats signal_processing</TAGS>

DESCRIPTION:
	Calculate statistics relating to the refractoriness of an auto- or cross-corellogram
	of the spiking for a cell or cell-pair. That is, the tendency for the +-2ms period in
	the middle of the histogram to be empty while the rest of the histogram is populated.
	This occurs because most neurons cannot fire 2 action potentials within 2ms.

	Modification of original code (calc_srefractoriness) written by Andrew Macpherson of PrismTC, August 2010
	Successor to xf_histrefract2_d (2017.02.15)

ARGUMENTS:
	double histy      : input, pointer to array containing histogram counts
	long nbins_tot    : number of bins used for histogram
	long nbins_c      : size of refractory period (bins, typically corresponding to 2ms)
	long nbins_s      : size of zone outside refractory period for comparison
	double *result_d  : results array (must be preallocated to hold 40 items)
				- diff: difference between the mean event-counts, central versus side
				- ratio: centre/(centre+side) - range 0-1, expected value depends on "nbins_c" & "nbins_s"
				- t-statistic: one-sided, based on diff, should be +ive if refractoriness exists
				- probability for the t-statistic

	char *message     : errort message (if any)

RETURN VALUE:
	- success: histogram total for the regions of interest (centre + left + right)
	- error: -1

SAMPLE CALL:
	refract = xf_histratio3_d(histy,100,2,8,result_d,message);
	- this assumes a 100 bin histogram, presumably covering a range of +-50ms (so 1ms per bin)
	- the refractory period is set as 2ms, so bins 48,49,50 &51 will constitute the refractory zone (+-2ms)
	- the comparison zone is the next 8 bins outside the refractory zone
		- for the left hand side, bins 40-47 (-10 to -3 ms)
		- for the right hand side, bins 52-59 (3 to 10 ms)


*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

double xf_prob_T1(double t, long df, int tails);

double xf_histratio2_d(double *histy, long nbins_tot, long nbins_c, long nbins_s, double *result_d, char *message) {

	char *thisfunc="xf_histratio2_d\0";
	int z,histstart_sl, histstart_sr, histstart_cl, histstart_cr, df=(nbins_c + nbins_s -2);
	long ii,jj,kk,nn;
	double aa,bb,cc;
	double sum_s,sum_sl,sum_sr,sum_c,sum_cl,sum_cr;
	double avg_s,avg_sl,avg_sr,avg_c,avg_cl,avg_cr;
	double ss_s,ss_sl,ss_sr,ss_c,ss_cl,ss_cr;
	double var_s,var_sl,var_sr,var_c,var_cl,var_cr;
	double diff_tot=NAN,diff_l=NAN,diff_r=NAN,ratio_tot,ratio_l,ratio_r, tval_tot,tval_l,tval_r, prob_tot,prob_l,prob_r;

	//make sure nbins_c and nbins_s do not add up to more than 0.5 * nbins tot
	ii=(2*(nbins_c+nbins_s));
	if(ii>nbins_tot) {
		sprintf(message,"%s [ERROR]: bins in middle(%ld)+side(%ld) > total bins/2 (%ld)",thisfunc,nbins_c,nbins_s,(nbins_tot/2));
		return(-1);
	}

	// determine where data starts for left, centre and right datasets
	histstart_cl= nbins_tot/2 - nbins_c;
	histstart_cr= nbins_tot/2;
	histstart_sl= nbins_tot/2 - nbins_c - nbins_s;
	histstart_sr= nbins_tot/2 + nbins_c;

	// calculate sums and sums of squares for the refractory zone, left and right sides
	sum_c= sum_cl= sum_cr= ss_c= ss_cl= ss_cr= 0.0;
 	for(ii=0;ii<nbins_c;ii++) {
		aa= histy[histstart_cl + ii];
		bb= histy[histstart_cr + ii];
		cc= aa+bb;
		sum_cl+= aa;
		ss_cl+=  aa*aa;
		sum_cr+= bb;
		ss_cr+=  bb*bb;
		sum_c+=  cc;
		ss_c+=   cc*cc;
	}
	// calculate sums and sums of squares for areas just outside refractory zone
	sum_s= sum_sl= sum_sr= ss_s= ss_sl= ss_sr= 0.0;
	for(ii=0;ii<nbins_s;ii++) {
		aa= histy[histstart_sl + ii];
		bb= histy[histstart_sr + ii];
		cc= aa+bb;
		sum_sl+= aa;
		sum_sr+= bb;
		ss_sl+= (aa*aa);
		ss_sr+= (bb*bb);
		sum_s+=  cc;
		ss_s+=   cc*cc;
	}
	// calculate averages & variances for data arrays
	avg_c=  sum_c/(nbins_c*2);
	avg_cl= sum_cl/nbins_c;
	avg_cr= sum_cr/nbins_c;
	avg_s=  sum_s/(nbins_s*2);
	avg_sl= sum_sl/nbins_s;
	avg_sr= sum_sr/nbins_s;
	var_c=  (ss_c  - ((sum_c * sum_c)/(nbins_c*2))) / ((nbins_c*2)-1.0) ;
	var_cl= (ss_cl - ((sum_cl*sum_cl)/nbins_c)) / (nbins_c-1.0) ;
	var_cr= (ss_cr - ((sum_cr*sum_cr)/nbins_c)) / (nbins_c-1.0) ;
	var_s=  (ss_s  - ((sum_s * sum_s)/(nbins_s*2))) / ((nbins_s*2)-1.0) ;
	var_sl= (ss_sl - ((sum_sl*sum_sl)/nbins_s)) / (nbins_s-1.0) ;
	var_sr= (ss_sr - ((sum_sr*sum_sr)/nbins_s)) / (nbins_s-1.0) ;

	// calculate ratio, t & p values for data
	if((sum_sl+sum_cl)>0) {
		ratio_l = sum_cl/(sum_sl + sum_cl);
		diff_l= avg_sl - avg_cl;
		tval_l= diff_l / ( sqrt(var_cl/nbins_c + var_sl/nbins_s) );
		prob_l=  xf_prob_T1(fabs(tval_l), df, 1); // 1-tailed probability that refractory spike counts are lower than the shoulder
	}
	else { ratio_l= -1; tval_l= 0.0; prob_l= 1.0; 	}

	// now do the same for the right hand side of the histogram
	if((sum_sr+sum_cr)>0) {
		ratio_r= sum_cr/(sum_sr + sum_cr);
		diff_r= avg_sr - avg_cr;
		tval_r= diff_r / ( sqrt(var_cr/nbins_c + var_sr/nbins_s) );
		prob_r=  xf_prob_T1(fabs(tval_r), df, 1); // 1-tailed t-test
	}
	else { ratio_r= -1; tval_r= 0.0; prob_r= 1.0;	}

	// now do the same for both sides combined
	if((sum_s+sum_c)>0) {
		ratio_tot = sum_c/sum_s;
		diff_tot= avg_s - avg_c;
		tval_tot= diff_tot / ( sqrt(var_c/(nbins_c*2) + var_s/(nbins_s*2)) );
		prob_tot=  xf_prob_T1(fabs(tval_tot), df, 1); // 1-tailed probability that refractory spike counts are lower than the shoulder
	}
	else { ratio_tot= -1; tval_tot= 0.0; prob_tot= 1.0; }



	/* FILL THE RESULTS ARRAY */
	/* summed events on the (l)eft and (r)ight side of the histogram, (c)entral and (s)ide regions */
	result_d[0]= sum_cl;
	result_d[1]= sum_sl;
	result_d[2]= sum_cr;
	result_d[3]= sum_sr;

	/* stats on the left-hand side */
	result_d[10]= diff_l;
	result_d[11]= ratio_l;
	result_d[12]= tval_l;
	result_d[13]= prob_l;

	/* stats on the right-hand side */
	result_d[20]= diff_r;
	result_d[21]= ratio_r;
	result_d[22]= tval_r;
	result_d[23]= prob_r;

	/* sttas based on treating the left and right together */
	result_d[30]= diff_tot;
	result_d[31]= ratio_tot;
	result_d[32]= tval_tot;
	result_d[33]= prob_tot;

	// return the total spike count
	return(sum_s + sum_c);
}
