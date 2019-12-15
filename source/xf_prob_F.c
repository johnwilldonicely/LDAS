/******************************************************************************
<TAGS>stats</TAGS>
DESCRIPTION:
	F-test function: Calculate F-distribution probability given F,df1, and df2

	Based on betai function from...
	NUMERICAL RECIPES IN C: THE ART OF SCIENTIFIC COMPUTING (2nd Edition)
	William H. Press, Saul A. Teukolsky, William T. Vetterling, & Brian P.
	Flannery. Cambridge University Press, 1992.
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAXDAT 1000
#define MAXIT 1000
#define EPS 3.0e-7
#define FPMIN 1.0e-30

float xf_prob_F(float F, int df1,int df2) {
	// F = F-value, df1 & df2 = degrees of freedom, numerator & denominator, respectively

	float betacf(float aa, float bb, float cc);
	float gammln(float xx);
	float a,b,x,bt;

	a= 0.5 * df2;
	b= 0.5 * df1;
	x= df2/(df2+df1*F);

	if (x<0.0 || x>1.0) return(-1.0);

	if (x==0.0 || x==1.0) bt=0.0;
	else bt=exp(gammln(a+b)-gammln(a)-gammln(b)+a*log(x)+b*log(1.0-x));

	if (x<(a+1.0)/(a+b+2.0))
		return(bt*betacf(a,b,x)/a);
	else
		return(1.0-bt*betacf(b,a,1.0-x)/b);
}


float betacf(float a, float b, float x) {

	void hux_error(char error_text[]);
	int m,m2;
	float aa,c,d,del,h,qab,qam,qap;

	qab=a+b;
	qap=a+1.0;
	qam=a-1.0;
	c=1.0;
	d=1.0-qab*x/qap;
	if (fabs(d) < FPMIN) d=FPMIN;
	d=1.0/d;
	h=d;

	for (m=1;m<=MAXIT;m++)  {
		m2=2*m;
		aa=m*(b-m)*x/((qam+m2)*(a+m2));
		d=1.0+aa*d;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=1.0+aa/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		h *= d*c;
		aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
		d=1.0+aa*d;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=1.0+aa/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}

	if (m > MAXIT) {
		fprintf(stderr,"\n--- Error: xf_prob_F/betacf: a (%.3f) or b (%.3f) too big, or MAXIT (%d) too small",a,b,MAXIT);exit(1);
	}
	return(h);
}


float gammln(float xx) {

	double x,y,tmp,ser;
	static double cof[6]={76.18009172947146,-86.50532032941677,24.01409824083091,-1.231739572450155,0.1208650973866179e-2,-0.5395239384953e-5};
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;

	for (j=0;j<=5;j++) ser += cof[j]/++y;

	return(-tmp+log(2.5066282746310005*ser/x));

}

/* // TEST CODE TO MAKE THIS AN EXECUTEABLE
int main(int argc, char *argv[]) {
	double F=1.5; long df;
	for(df=1;df<10;df++) printf("t=%g	df=%ld  p=%g\n",F,df,xf_prob_F((F*F),1,df));

} */
