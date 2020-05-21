/*
<TAGS>signal_processing detect</TAGS>

DESCRIPTION:
	Detect inflections in a series
	Assumes signal is filtered to reduce number of inflections to a reasonable number

USES:

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	double *data   : input holding data
	long n1       : size of data array
	long **itime : address for time-stamp array (samples) - calling function must free()
	int **isign  : address for inflection-sign array (-1=negative, 1=positive) - calling function must free()
	char *message : arrray to hold message in the event of an error

RETURN VALUE:
	number of detected inflections (>=0) on success
	-1 on error
	result arrays *itime and *isign will have been assigned memory and will hold event times (sample) and sign (-1 or +1)

SAMPLE CALL:
	#include <stdio.h>
	#include <stdlib.h>
	#include <math.h>
	int main (int argc, char *argv[]) {
		char message[1000];
		long *itime=NULL,n1=350,n2,ii;
		int *isign=NULL;
		double data[1000];
		double aa=M_PI/10;
		for(ii=0;ii<n1;ii++) data[ii]= sin(ii*aa);
		for(ii=0;ii<n1;ii++) printf("%ld %g\n",ii,data[ii]);
		n2= xf_detectinflect1_f(data,n1,&itime,&isign,message);
		if(n2<0) {fprintf(stderr,"\n--- Error: %s\n\n",message); exit(1);}
		for(ii=0;ii<n2;ii++) printf("%ld\t%d\n",itime[ii],isign[ii]);
		if(itime!=NULL) free(itime);
		if(isign!=NULL) free(isign);
	}
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long xf_detectinflect1_d(double *data,long n1,long **itime,int **isign,char *message) {

	char *thisfunc="xf_detectinflect1_d\0";
	int *tempsign=NULL,detect,sizeoftime,sizeofsign;
	long *temptime=NULL,ii,jj,kk,nminus1,itot;

	sizeoftime= sizeof(*temptime);
	sizeofsign= sizeof(*tempsign);
	nminus1= n1-1;
	itot=0;

	for(ii=1;ii<nminus1;ii++) {
		/* detect positive inflections */
		if(data[ii]>data[ii-1] && data[ii]>data[ii+1]) {
			temptime= realloc(temptime,((itot+1)*sizeoftime));
			tempsign= realloc(tempsign,((itot+1)*sizeofsign));
			if(temptime==NULL || tempsign==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
			temptime[itot]= ii;
			tempsign[itot]= 1;
			itot++;
		}
		/* detect negative inflections */
		else if(data[ii]<data[ii-1] && data[ii]<data[ii+1]) {
			temptime= realloc(temptime,((itot+1)*sizeoftime));
			tempsign= realloc(tempsign,((itot+1)*sizeofsign));
			if(temptime==NULL || tempsign==NULL) {sprintf(message,"%s: memory allocation failed",thisfunc); return(-1); }
			temptime[itot]= ii;
			tempsign[itot]= -1;
			itot++;
		}
	}

	(*itime)=temptime;
	(*isign)=tempsign;
	return(itot);
}
