/**********************************************************************
Calculate basic stats a float array of data
Results are stored in an array (result) 
***********************************************************************/
#include<math.h>
#include<stdlib.h>
#include<stdio.h>

void hux_error(char message[]);
int xf_compare1_f(const void *a, const void *b); 

void hux_linearstats (float *array,int arraysize,float missing,float *result)
{
int i,j;
unsigned long n=0;
float *temp,min,max,a;
long double nn,sum=0.0,ss=0.0,var=0.0,sd=0.0,sem=0.0,mean=0.0,median=0.0,skew=0.0,aa;

temp = (float *) calloc(arraysize+2,sizeof(float));
if(temp==NULL) hux_error("hux_linearstats: insufficient memory to sort input array");
for(i=0;i<32;i++) result[i]=-1;
for(i=0;i<arraysize;i++) if(array[i]!=missing) {sum += array[i]; ss += array[i]*array[i]; temp[n++]=array[i];}
if(n<2) {
	if(n<=0) {
		for(i=0;i<21;i++) result[i]=(float)0;
		free(temp);
		return;
	}
	else {
		result[0]=(float)n; 
		result[2]=result[3]=result[4]=result[10]=(float)0;
		result[1]=result[5]=result[6]=result[7]=result[8]=result[9]=array[0];
		free(temp);
		return;
	}
}

/* sort the temporary array in order to find median (and quartiles) */
qsort(temp,n,sizeof(float),xf_compare1_f);

/* calculate stats */
min=temp[0]; max=temp[n-1]; 
median=(temp[(int)(n/2.0)]+temp[(int)(n/2.0)-1])/2.0; /* average of bins on either side of cutoff */
nn=(long double)n;
mean= sum/n;
var	= (ss-(sum*sum/n))/(n-1.0);
sd	= sqrt((double)var);
sem	= sd/sqrt((double)n);

for(i=0;i<n;i++) {aa=temp[i]-mean;skew+=aa*aa*aa;}
skew = (n/((n-1.0)*(n-2.0))) * (skew/(sd*sd*sd));

result[0] = (float) n;
result[1] = (float) mean;
result[2] = (float) var;
result[3] = (float) sd;
result[4] = (float) sem;
result[5] = (float) min;
result[6] = (float) max;
result[7] = (float) median;
result[8] = (float) sum;
result[9] = (float) ss;
result[10] = (float) skew;

i=(int)(n*0.025); result[11] = (temp[i]+temp[i-1])/2.0;  // cutoff for 2.5th percentile of firing rates 
i=(int)(n*0.05); result[12] = (temp[i]+temp[i-1])/2.0;  // cutoff for 5th percentile of firing rates 
i=(int)(n*0.10); result[13] = (temp[i]+temp[i-1])/2.0;  // cutoff for 10th percentile of firing rates 
i=(int)(n*0.25); result[14] = (temp[i]+temp[i-1])/2.0;  // cutoff for 25th percentile of firing rates 
i=(int)(n*0.50); result[15] = (temp[i]+temp[i-1])/2.0;  // cutoff for 50th percentile of firing rates 
i=(int)(n*0.75); result[16] = (temp[i]+temp[i-1])/2.0;  // cutoff for 75th percentile of firing rates 
i=(int)(n*0.90); result[17] = (temp[i]+temp[i-1])/2.0;  // cutoff for 90th percentile of firing rates 
i=(int)(n*0.95); result[18] = (temp[i]+temp[i-1])/2.0;  // cutoff for 95th percentile of firing rates 
i=(int)(n*0.975); result[19] = (temp[i]+temp[i-1])/2.0;  // cutoff for 97.5th percentile of firing rates 
result[20]=max;

free(temp);
return;
}
