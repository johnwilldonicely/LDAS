/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Reverse the Fisher transformation for an array of Pearson's correlation coefficients
	see http://en.wikipedia.org/wiki/Fisher_transformation

	NOTE: for type-2 correlations (range 0-1), this translates to a range of 0 to 7.6

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data : input array of data (will be transformed)
	long n : number of elements in the array
	int type :
		1: correlations can range from -1 to 1
		2: correlations can range from 0 to 1

RETURN VALUE: NONE

	Oringinal input data will be transformed

*/

#include <math.h>
void xf_fishertransformrev2_d(double *data, long n, int type) {

	long i;
	double aa;
	if(n<1) return;

	if(type==1) {
		for(i=0;i<n;i++) {
			aa=data[i];
			if(isfinite(aa)) {
				aa=tanh(aa);
				if(aa<-1.0) aa=-1.0;
				else if(aa>1.0) aa=1.0;
				data[i]=aa;
			}
		}
	}

	else if(type==2) {
		for(i=0;i<n;i++) {
			aa=data[i];
			if(isfinite(aa)) {
				if(aa<0.0) aa=0.0;
				if(aa>7.6) aa=7.6;
				aa= 0.5*(tanh(aa-3.8)+1);
				data[i]=aa;
			}
		}
	}

	return;
}
