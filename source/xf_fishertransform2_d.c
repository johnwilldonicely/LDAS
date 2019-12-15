/*
<TAGS>signal_processing filter</TAGS>

DESCRIPTION:
	Calculate the Fisher transformation for an array of Pearson's correlation coefficients
	Simply the inverse hyperbolic function of each value
	see http://en.wikipedia.org/wiki/Fisher_transformation

	NOTE: a limit of +-3.8 is put on the transform, which is sufficient accuracy for
	correlation coefficients to a precision of 0.001

	NOTE: for type-2 correlations (range 0-1), this translates to a range of 0 to 7.6


USES:
	Allows hypothesis testing for population correlation coefficients

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
void xf_fishertransform2_d(double *data, long n, int type) {

	long i;
	double aa;
	if(n<1) return;

	if(type==1) {
		for(i=0;i<n;i++) {
			aa=data[i];
			if(isfinite(aa)) {
				aa=atanh(aa);
				if(aa<-3.8) aa=-3.8;
				else if(aa>3.8) aa=3.8;
				data[i]=aa;
			}
		}
	}

	else if(type==2) {
		for(i=0;i<n;i++) {
			aa=data[i];
			if(isfinite(aa)) {
				aa = atanh ( (2.0*aa) - 1.0 ) ;
				if(aa<-3.8) aa=-3.8;
				else if(aa>3.8) aa=3.8;
				aa+=3.8;
				data[i]=aa;
			}
		}
	}

	return;
}
