#include <stdio.h>
#include <stdlib.h>
#define xmax_INTP 10000
/*
<TAGS>signal_processing</TAGS>

DESCRIPTION:
	Performs SPLine INTerpolation, based on the splint and spline functions from Numerical Recipes in C
		xin[] and yin[] are the original X and Y arrays, of size "nin"
		xout[] will be filled with n="nout" evenly spaced values filling the original range
		yout[] will be filled with the interpolated arrays of size "nout"
		
NOTE_1: Sufficient memory must be pre-alocated for xout and yout, based on the size of nout
*/

void xf_spline1_d(double xin[],double yin[],long nin,double xout[],double yout[],long nout) {

	long ii,jj,kk,klo,khi;
	double xmin,xmax,h,b,a,p,qn,sig,un,yp1,ypn,*u,*yfunc;
	if (nin>xmax_INTP) {printf("Too many items to intepolate\n");exit(1);}

	/********************************************************************************/
	/* STEP 0: PRE-FILL ARRAY FOR INTERPOLATED X-VALUES */
	/********************************************************************************/
	xmin=xmax=xin[0];
	for(ii=0;ii<nin;ii++) {if(xin[ii]>xmax) xmax=xin[ii]; if(xin[ii]<xmin) xmin=xin[ii];} // find range
	a=(xmax-xmin)/(nout-1); // this is the interval between points to evenly fill the original range
	for(ii=0;ii<nout;ii++) xout[ii]=xmin+a*(double)ii; // fill xout array evenly

	/********************************************************************************/
	/* STEP 1: CALCULATE DERIVATIVE OF INTERPOLATING FUNCTION FOR EACH DATA POINT */
	/********************************************************************************/
	if((yfunc= malloc((nin+1)*sizeof(*yfunc))) == NULL) {fprintf(stderr,"insufficient memory");exit(0);}
	if((u= malloc((nin+1)*sizeof(*u))) == NULL) {fprintf(stderr,"insufficient memory");exit(0);}

	yp1=(yin[1]-yin[0])/(xin[1]-xin[0]);
	ypn=(yin[nin-1]-yin[nin-2])/(xin[nin-1]-xin[nin-2]);

	if(yp1 > 0.99e30) yfunc[0]= u[0]= 0.0;
	else {
		yfunc[0]= -0.5;
		u[0]=(3.0/(xin[1]-xin[0]))*((yin[1]-yin[0])/(xin[1]-xin[0])-yp1);
	}

	for (ii=1;ii<nin-1;ii++) { // skip first and last data points
		sig= (xin[ii]-xin[ii-1])/(xin[ii+1]-xin[ii-1]);
		p= sig*yfunc[ii-1]+2.0;
		yfunc[ii]= (sig-1.0)/p;
		u[ii]= (yin[ii+1]-yin[ii])/(xin[ii+1]-xin[ii]) - (yin[ii]-yin[ii-1])/(xin[ii]-xin[ii-1]);
		u[ii]= (6.0*u[ii]/(xin[ii+1]-xin[ii-1])-sig*u[ii-1])/p;
	}

	if (ypn > 0.99e30) qn=un=0.0;
	else {
		qn=0.5;
		un=(3.0/(xin[nin-1]-xin[nin-2]))*(ypn-(yin[nin-1]-yin[nin-2])/(xin[nin-1]-xin[nin-2]));
	}

	yfunc[nin-1]= (un-qn*u[nin-2])/(qn*yfunc[nin-2]+1.0);
	for(kk=(nin-2);kk>=0;kk--) yfunc[kk]= yfunc[kk]*yfunc[kk+1]+u[kk];

	/********************************************************************************/
	/* STEP 2: APPLY INTERPOLATING FUNCTION TO XIN AND YIN, RESULTS IN X AND Y (SIZE NOUT) */
	/********************************************************************************/
	for(ii=0;ii<nout;ii++) {
		klo= 0;
		khi= nin-1;
		while((khi-klo) > 1) {
			kk= (khi+klo) >> 1;
			if(xin[kk] > xout[ii]) khi= kk;
			else klo= kk;
		}
		h= xin[khi]-xin[klo];
		if (h == 0.0) {printf("Bad xin input to routine splint");exit(1);}
		a= (xin[khi]-xout[ii])/h;
		b= (xout[ii]-xin[klo])/h;
		yout[ii]= a*yin[klo]+b*yin[khi]+((a*a*a-a)*yfunc[klo]+(b*b*b-b)*yfunc[khi])*(h*h)/6.0;
	}

	free(yfunc);
	free(u);
}
