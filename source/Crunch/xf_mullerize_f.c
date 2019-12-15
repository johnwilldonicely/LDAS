/***********************************************************************
DESCRIPTION: 
	Determine rate cutoffs for Robert-Muller style 6-colour firing-rate maps

USES: 

DEPENDENCIES:
	xf_compare1_d

ARGUMENTS: 
	float *data : the original data array
	long n : total elements in the input array
	float *result : pointer to an array with reserved memory for at least 16 elements to hold the cutoff values
	
RETURN VALUE: 

	0 on success, -1 on memory allocation fail
	
	result[0]: always zero - cutoff for base-colour pixels
	result[1-5]:   upper limits for colours 1-5
	result[6-10]:  median rates for colours 1-5
	result[11-15]: mean rates for colours 1-5
	
	To use this to determine the colour of any given datum rate[i]: 

		c[i]=5; 
		for(j=4;j>=0;j--) if(rate[i]>result[j]) c[i]=j;
		
ADDITIONAL NOTES:

	A fixed proportion of non-zero-rate pixels assigned to each colour...
	Each step up the rate ladder has 80% of the pixels of the lower colour 

	orange = initial proportion (x)
	red = orange*.8
	green = red*.8 = orange*.64
	blue = green*.8 = orange*.512
	purple = blue*.8 = orange*.4096

	1x +.8x + .64x + 0.512x + .4096x = 1
	3.3616 * x = 1
	x = 1/3.3616 = 0.29747739171822941456449309852451

	so typically...

	29.7% of pixels are orange  - cumulative 29.7 
	23.8% of pixels are red 	- cumulative 53.5
	19.0% of pixels are green	- cumulative 72.5
	15.2% of pixels are blue	- cumulative 87.8
	12.2% of pixels are purple  - cumulative 97.8
	
	...and all "zero" pixels will be yellow

************************************************************************/
#include<stdlib.h>

/* external function for qsort comparison */
int xf_compare1_f(const void *a, const void *b);

int xf_mullerize_f(float *data, long n, float *result) 
{ 
	int i,x,y,z,count;
	long n2=0;
	float *temp;
	double a,sum,proportion = 1.0/3.3616; /* the magic start number so 5 iterations sum to 100% */

	/* set up temporary array */
	temp = (float *) malloc(n*sizeof(float)); 
	if(temp==NULL) return(-1);
	/* fill array with non-zero rates */
	for(i=0;i<n;i++) if(data[i]>0.0000000001) temp[n2++] = data[i];
	
	/* sort the temporary array */ 
	qsort(temp,n2,sizeof(float),xf_compare1_f); 

	x = y = z = 0;
	a = result[0] = 0.0;
	
	for(i=1;i<=5;i++) 
	{
		a= a+proportion;
		z= (int)(n2*a);
		sum=0.0;
		count=0;
		for(x=y;x<z;x++) 
		{
			sum += temp[x];
			count++;
		}
		result[i]= temp[z];
		result[i+5]= temp[(int)((y+z)/2)];
		result[i+10]= sum/count;
		
		proportion *= 0.8; /* reduce the fraction of pixels represented by next colour */ 
		y = z; /* set previous index mark */
	}
	
	free(temp);
	return(0);
}
