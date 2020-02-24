/*
DESCRIPTION:
	Calculate negative and positive area under the curve - typically fr a time-series

USES:

DEPENDENCIES:
	None

ARGUMENTS:
	double *data : input holding data
	size_t nn : size of data array
	double interval:  the interval between samples, for integrating area
	double *result : pre-allocated array to hold results - must allow at least 6 elements
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	0 on success, -1 on error
	result array will hold statistics
	message array will hold explanatory text (if any)

SAMPLE CALL:
	x= xf_auc1_d(data, nn, interval, result, );
	if(x!=0) { fprintf(stderr,"\b\n\t%s/%s\n\n",thisprog,message); exit(1); }

<TAGS> programming ldas </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
int xf_auc1_d(double *data, size_t nn, double interval, double **newdat, double *result, char *message) {

	char *thisfunc="xf_auc1_d\0";
	int status=0;
	long i,j,k,tot=0;

	size_t ii,Xn=0,Xp=0;
	double aa,bb,An,Ap,Yn,Yp;
	double *pnewdat=NULL; // unallocated space for input array memory allocation

	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,nn); status=-1; goto END; }

	/* CHECK INTEGRITY OF DATA */
	for(ii=0;ii<nn;ii++) if(isfinite(data[ii])) break;
	if(ii>=nn)) { sprintf(message,"%s [ERROR]: no valid data",thisfunc); status=-1; goto END; }

	An=Ap=0.0;
	Xn=Xp=ii;
	Yn=Yp=data[ii];

	for(ii=ii;ii<nn;ii++) {
		aa=data[ii];
		if(aa>=0) {
			Ap+= interval*aa;
			if(aa>Yp) {
				Xp=ii;
				Yp=aa;
			}
		}
		else  {
			An-= interval*aa;
			if(aa<Yn) {
				Xn=ii;
				Yn=aa;
			}
		}
	}

	// example dynamic allocation of memory for **newdat to point to upon completion
	for(ii=0;ii<nn;ii++) {
		pnewdat= realloc(pnewdat,(ii+1)*sizeof(*pnewdat));
		if(pnewdat==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc); status=-1; goto END;}
		pnewdat[ii]= data[ii];
	}
	(*newdat)= pnewdat; // assign pointer. in calling function, free(newdat)

END:
	if(pnewdat!=NULL) free(pnewdat);
	return (status);
}
