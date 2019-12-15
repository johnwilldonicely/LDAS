/*******************************************************************************
- Fill target array (B) with interpolated values from source array (A)
- Looks for data in array A which fall before & after each data point in array B
- assigns interpolated value of A to B
- invalid data points only assigned to B where it is out of range of A (beginning & end)
- NOT appropriate for data with large number of invalid samples (eg. theta phase)
	- in this case, use similar routine, hux_fillprev instead
********************************************************************************/
# include <string.h>
# include <stdio.h>
int hux_fillinterp (
	double *A_time,
	double *B_time,
	float *A_val,
	float *B_val,
	int Atot,
	int Btot,
	int invalid,
	char *type)
{
int indexA,indexB,first,second,count,circular;
float a;

indexA=indexB=count=0;
if(strcmp(type,"circular")==0) circular=1;
else circular=0;

/* determine first valid source record */
while(A_val[indexA]==invalid && indexA<Atot) indexA++; 
/* from there, determine target data just after first valid source */
if(B_time[0] <= A_time[indexA]) {
	while(B_time[indexB] <= A_time[indexA] && indexB<Btot) {
		B_val[indexB] = (float) invalid; 
		indexB++;
		}
}

/* find valid A records bounding each B records - start indexA with first valid A */
first=second=indexA;
for(indexB;indexB<Btot;indexB++) {
	/* find valid source sample indexA with bigger timestamp than target*/
	while(A_time[second] < B_time[indexB] || A_val[second] == invalid) {
		if(A_val[second]!=invalid) first=second;
		second++; 
		if(second>=Atot) {
			for(indexB;indexB<Btot;indexB++) B_val[indexB] =  invalid;
			return(count);
		}
	}
	/* calc. proportion of inter-source-sample interval elapsed since last source sample*/
	a = (B_time[indexB]-A_time[first]) / (A_time[second]-A_time[first]);
	/* use this time proportion to calculate proportions of other variables elapsed */
	B_val[indexB] = A_val[first] + ((A_val[second]-A_val[first]) * a);
	count++;
	/* determine corrected proportion of heading change at target record time */
	if(circular==1 && B_val[indexB]!=invalid) {
		if(B_val[indexB]<0) B_val[indexB] = 360 + B_val[indexB];
		if(B_val[indexB]>359.999999) B_val[indexB] = 360 - B_val[indexB];
	}
}

return(count);
}
