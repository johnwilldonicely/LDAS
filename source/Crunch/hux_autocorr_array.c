/**********************************************************************
Fill an array of n bins with counts for isi or autocorrelation intervals
***********************************************************************/
#include<stdio.h>
#include <stdlib.h>
#include <string.h>

void hux_error(char message[]);

int hux_autocorr_array(
				  double *time,	/* pointer to a floating point array of time data (seconds) */
				  int n,		/* number of elements in the time array */
				  int *count, /* array of size bintot to hold histogram counts */
				  int bintot,	/* total number of bins in histogram (suggested: 100) */
				  int winsize,	/* width of count window in milliseconds (suggested: 10)*/
				  float scale, /* max value for scaled bin counts: 0 = raw values (no scaling), 100 = 0-100, 150 = 0-150, etc. */
				  char *mode	/* isi or acor for inter-spike interval or autocirrelation */
				  )
{
	char command[256];
	int i,p,max=-1,bin,end=n-1;
	float timeadjust,winsize_s = winsize/1000.0;
	double diff;

	if(n<2) {sprintf(command,"hux_autocor_array: inappropriate n (%d) < 2",n); hux_error(command);}
	if(bintot<2) {sprintf(command,"hux_autocor_array: inappropriate bintot (%d) < 2",bintot); hux_error(command);}
	if(winsize <2) {sprintf(command,"hux_autocor_array: inappropriate winsize (%d) < 2",winsize); hux_error(command);}

	timeadjust = 1000*((float)bintot/(float)winsize); /* converts seconds to miliseconds, and then bin number */
	/* method for calculating  temporal autocorrelation */
	if(strcmp(mode,"acor")==0) { 
		i=1; while(i<n && (time[i]-time[0])<=winsize_s) i++;
		for(i;i<end;i++) for(p=i-1;p>=0;p--) {
				diff=(time[i]-time[p]);
				if(diff<winsize_s) {
					bin=(int)(diff*timeadjust);
					if(bin<bintot) count[bin]++;
				}
				else break;
			}}
	/* method for calculating interspike interval histogram */
	else if(strcmp(mode,"isi")==0) { 
		for(i=1;i<end;i++) {bin=(int)((time[i]-time[i-1])*timeadjust); if(bin<bintot) count[bin]++;}}
	
	/* autoscale count to a range of 0-1 */
	for(bin=0;bin<bintot;bin++) if(count[bin]>max) max=count[bin]; /* calculate highest count */
	if(scale!=0.0) for(bin=0;bin<bintot;bin++) if(count[bin]>0) count[bin]=(int)((count[bin]/(float)max)*scale); /* normalise */
	return(max);
}
