/**********************************************************************
Calculate circular mean for a float array
Results are stored in an array (result) 
NOTE: modified Rayleigh may not be accurate
To Do: Rao's spacing test?
***********************************************************************/
#include<math.h>
#include<stdio.h>

float hux_prob_R(float R,int n);
void hux_circmean(
				  float* array,	 /* pointer to a floating point array of angle data (0-359) */
				  int arraysize, /* number of elements in the array (including array[0]) */
				  int missing,	 /* a data value which should be ignored (eg. -1) */
				  float *result  /* pointer to a float array of at least 7 elements which holds results */
				  )
{
int i;
unsigned long n=0;
long double sintot=0.0,sinmean=0.0,hypmean,hypmean2,costot=0.0,cosmean=0.0,temp=0.0,
	mean=0.0,sd,min=360.0,max=0.0;
float rayleigh,p;


for(i=0;i<arraysize;i++) {
	if(array[i]!=missing) {
		if(array[i]>max)max=array[i]; if(array[i]<min)min=array[i];	
		sintot += sin(array[i]*0.017453292519943295769236907684886); /* number is PI/180*/
		costot += cos(array[i]*0.017453292519943295769236907684886);  /* number is PI/180*/
		n++;
	}}

if(n<=0){
	for(i=0;i<6;i++) result[i]=(float)0; 
	return;
}
sinmean=sintot/n;	/* mean sin-transformed phase value */
cosmean=costot/n;	/* mean cos-transformed phase value */
/* now convert back into a single mean value */
if(cosmean<0) mean=atan(sinmean/cosmean)+3.1415926535897932384626433832795;
else if(sinmean<0) mean=atan(sinmean/cosmean)+6.283185307179586476925286766558; /* number is 2*Pi */
else mean=atan(sinmean/cosmean); 
temp=pow(sinmean,2)+pow(cosmean,2);
hypmean=sqrt(temp);
sd=sqrt(-2*log(sqrt(temp)));

rayleigh= n*hypmean*hypmean;
p=hux_prob_R(hypmean,n);

result[0] = (float) n;
result[1] = (float) hypmean; /* mean length of the resultant angular vector (0-1) */ 
result[2] = (float) mean*57.295779513082320876798154814105;	/* convert mean back to degrees - number is 180/Pi*/
result[3] = (float) sd*57.295779513082320876798154814105;	/* convert std.dev. back to degrees */
result[4] = (float) rayleigh; /* the Rayleigh statistic to test for clustering around the mean */ 
result[5] = p;
return;
}
