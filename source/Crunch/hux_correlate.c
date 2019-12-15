/*
This fills a "results" array with details from a Pearson's correlation
Requires two parallel arrays of floating point values
Returns the number of valid sample-pairs included in the analysis
*/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>

float xf_prob_F(float F,int df1,int df2);

int hux_correlate(
					float *x, 
					float *y, 
					int N, 
					int invalid, 
					float *result /* should point to array space of at least 18 float elements */
					)
{
int  i,N_valid=0,dfa,dfb;
float betai(float a,float b,float x);
double prob,b0,b1,
	SUMx=0.0,SUMy=0.0,SUMx2=0.0,SUMy2=0.0,SUMxy=0.0,MEANx=0.0,MEANy=0.0,
	SSx=0.0,SSy=0.0,SDx=0.0,SDy=0.0,SSreg=0.0,SSres=0.0,SPxy=0.0,r=0.0,r2=0.0,r2adj=0.0,SE=0.0,PIP=0.0,F=0.0;

for(i=0;i<=16;i++) result[i]=0.0;

for(i=0;i<N;i++) if(x[i]!=invalid && y[i]!=invalid) {
	N_valid++;
	SUMx += x[i]; 
	SUMy += y[i];
	SUMx2 += x[i]*x[i];
	SUMy2 += y[i]*y[i];
	SUMxy += x[i]*y[i];
}

MEANx = SUMx/N_valid; 
MEANy = SUMy/N_valid;
SSx   = SUMx2-((SUMx*SUMx)/N_valid); 
SSy  = SUMy2-((SUMy*SUMy)/N_valid);
SPxy = SUMxy-((SUMx*SUMy)/N_valid);
SDx  = sqrt(SSx/(N_valid-1.0));
SDy  = sqrt(SSy/(N_valid-1.0));
b1   = SPxy/SSx;
b0   = MEANy-(b1*MEANx);
r     = SPxy/(sqrt(SSx)*sqrt(SSy));
r2    = r*r;
r2adj = 1-((1-r2)*(N_valid-1)/(N_valid-2));
dfa   = 1;
dfb   = N_valid-2;
F     = (r2*dfb)/(1-r2);

if(F>0&&dfa>0&&dfb>0&&SDx!=0) prob = xf_prob_F(F,dfa,dfb);
else {r=r2=F=prob=-1.0;N_valid=-1;} // this may happen if, for example, there is no variance in the predictor
result[0] = (float) N_valid;
result[1] = (float) r;	/* Pearson's r */
result[2] = (float) r2; /* r-squared */
result[3] = (float) r2adj; /* adjusted r-squared */
result[4] = (float) F;
result[5] = (float) dfa;
result[6] = (float) dfb;
result[7] = (float) prob; /* p-value (probability) */
result[8] = (float) MEANx; 
result[9] = (float) MEANy;
result[10] = (float) SSx; 
result[11] = (float) SSy;
result[12] = (float) SPxy;
result[13] = (float) SDx;
result[14] = (float) SDy;
result[15] = (float) b1; /* slope */
result[16] = (float) b0; /* intercept */
result[17] = (float) N_valid;
return(N_valid);
}
