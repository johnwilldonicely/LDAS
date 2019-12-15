/***********************************************************************
<TAGS>stats signal_processing matrix</TAGS>
DESCRIPTION:
	Determine rate cutoffs for Robert-Muller style 10-colour firing-rate maps
	A fixed proportion of non-zero-rate pixels assigned to each colour...
	Each step up the rate ladder has 80% of the pixels of the lower colour

DEPENDENCIES:
	xf_compare1_d

ARGUMENTS:
	double *data : the original data array
	long n : total elements in the input array
	double *result : pointer to an array with reserved memory for at least 16 elements to hold the cutoff values

USES:

RETURN VALUE:
	0 on success, -1 on memory allocation fail

	The result array is filled with cutoffs for bands of increasing values in the data

	result[] 	proportion	cumulative
	----------------------------------
	result[0] 	UNKNOWN 	UNKNOWN  - result[0] is always zero - cutoff for base-colour pixels
	result[1]	0.224059	0.224059
	result[2]	0.179247	0.403306
	result[3]	0.143398	0.546704
	result[4]	0.114718	0.661422
	result[5]	0.0917746	0.753197
	result[6]	0.0734197	0.826616
	result[7]	0.0587357	0.885352
	result[8]	0.0469886	0.932341
	result[9]	0.0375909	0.969932
	result[10]	0.0300727	1.000000

	To use this to determine the colour (z) of any given datum:

		z=0; for(j=0;j<10;j++) if(data[i]>result[j]) z=j+1



************************************************************************/
#include<stdlib.h>

/* external function for qsort comparison */
int xf_compare1_d(const void *a, const void *b);

int xf_mullerize_d10(double *data, long n, double *result)
{
	int i,x,y,z,count;
	long n2=0;
	double *temp,a,sum;
	double proportion = 0.224059; /* the magic start number so 10 iterations sum to 100% */

	/* set up temporary array */
	temp = malloc(n*sizeof(double));
	if(temp==NULL) return(-1);
	/* fill array with non-zero rates */
	for(i=0;i<n;i++) if(data[i]>0.0000000001) temp[n2++] = data[i];

	/* sort the temporary array */
	qsort(temp,n,sizeof(double),xf_compare1_d);

	x = y = z = 0;
	a = result[0] = 0.0000000001;

	for(i=1;i<=10;i++) {
		a= a + proportion;
		z= (int)(n * a);
		result[i]= temp[z];
		proportion *= 0.8; /* reduce the fraction of pixels represented by next colour */
		y = z; /* set previous index mark */
	}

	free(temp);
	return(0);
}
