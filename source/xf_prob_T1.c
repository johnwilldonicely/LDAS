#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
/*
<TAGS>stats</TAGS>
DESCRIPTIOPN:
	Returns approximation to t probability
	Matches Excel's TDIST function to 4 decimal places
	Modified version of estimate_tdist,
		originally created by Paul Nelson and Andrew Macpherson of PrismTC, September 2010

		//??? PROBLEM WITH LARGE DF (IF n>302, NUMERICAL LIMITS ARE EXCEEDED)!!!
		- alt_fact function will go out of bounds
		- stopping before tmp
TEST CODE TO MAKE THIS AN EXECUTEABLE
	int main(int argc, char *argv[]) {
	double t=1.5; long df=10;int tails=1;
	for(df=1;df<10;df++) printf("t=%g df=%ld tail%d p=%g\n",t,df,tails,xf_prob_T1(t,df,tails));
	}
*/

// DECLARE INTERNAL FUNCTIONS
double alt_fact(long n);

// MAIN
double xf_prob_T1(double t, long df, int tails) {


	/* make sure t is positive */
	if(t<0.0) t=(0.0-t);

	if(df<1) return(-1);

	// max_x & n variables set to provide closest match to Excel TDIST function
	double max_x = 20.0;	// upper limit of area under t-dist curve to calculate
	long i,maxit = 500;	// number of iterations / "slices" of area
	double h = (max_x - t) / (double)maxit; // width of "slices"
	double probt=0, t1,t2,res1,res2,intval;
	double gamma,pi = atan(1.0)*4.0, dfd=(double)df;

	// calculate gamma function (different for odd/even degrees of freedom)
	if(df % 2 == 0) {// degrees of freedom is even
		gamma= (alt_fact(df-1)) / (2*sqrt(dfd)*alt_fact(df-2));
	}
	else {// degrees of freedom is odd
		gamma= (alt_fact(df-1)) / (pi*sqrt(dfd)*alt_fact(df-2));
	}

	for(i=0;i<maxit;i++) {
		t1= t + i*h;
		t2= t + (i+1)*h;
		res1= gamma*pow( (1. + t1*t1 / dfd), (-(dfd+1.) / 2.) );
		res2= gamma*pow( (1. + t2*t2 / dfd), (-(dfd+1.) / 2.) );
		probt += ( res1+res2 ) / 2. * h; // cumulative area of the slices
	}
	probt *= tails; // adjust for 1 or 2 tail probability
	return(probt);
}


double alt_fact(long n) {
// if n is even: returns n * (n-2) * (n-4) * ... * 4 * 2
// if n is odd:  returns n * (n-2) * (n-4) * ... * 5 * 3
	long i;
	long double tmp=1.0;

	for(i=n;i>0;i=i-2) tmp *= (double) i;

	return(tmp);
}
