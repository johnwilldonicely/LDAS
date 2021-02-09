/*
DESCRIPTION:
	Return an array of the unique elements in an array

USES:

DEPENDENCIES:
	None

ARGUMENTS:
	double *data  : input data array
	long *nn      : address to size of data array (pass as &nn)
	char *message : pre-allocated array to hold error message

RETURN VALUE:
	pointer to unique array on success, NULL on error
	nn will be updated
	message array will hold explanatory text (if any)

SAMPLE CALL:
	x= xf_auc1_d(data, nn, interval, result, );
	if(x!=0) { fprintf(stderr,"\n\t--- %s/%s\n\n",thisprog,message); exit(1); }

<TAGS> math stats </TAGS>
*/

#include <stdio.h>
#include <stdlib.h>
double *xf_unique_d(double *data, long *nn, char *message) {

	char *thisfunc="xf_unique_d\0";
	long ii,jj,kk,mm,usize;
	double *unique=NULL,aa;

	/* CHECK VALIDITY OF ARGUMENTS */
	if(nn==0) { sprintf(message,"%s [ERROR]: invalid size of input (%ld)",thisfunc,*nn); return(NULL); }

	/* PRE-DEFINE SIZE OF UNIQUE ARRAY */
	usize= sizeof(*unique);

	/* FIND UNIQUE ELEMENTS  */
	mm= 0;
	for(ii=0;ii<(*nn);ii++) {
		/* see if the current value has occurred before */
		aa= data[ii];
		for(jj=0;jj<mm;jj++) if(aa==unique[jj]) break;
		/* if the current value was not found previously, add it to the unique-array */
		if(jj==mm) {
			unique= realloc(unique,(mm+1)*usize);
			if(unique==NULL) {sprintf(message,"%s [ERROR]: insufficient memory",thisfunc);return(NULL);}
			unique[mm]= data[ii];
			mm++;
		}
	}

	/* UPDATE NN AND RETURN UNIQUE */
	*nn= mm;
	return (unique);
}
