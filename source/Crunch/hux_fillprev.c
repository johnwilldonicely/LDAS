/*******************************************************************************
- fill target array (B) with value of most recent value from source array (A)
- appropriate for data with large number of invalid samples (eg. theta phase)
	- otherwise, use similar routine, hux_fillinterp for more accurate results
********************************************************************************/
int hux_fillprev (double *A_time,double *B_time,float *A_val,float *B_val,int Atot,int Btot,int invalid)
{
int w=0,z=0,count=0;
if(B_time[0] < A_time[0]) /* if target data starts first, fill early records with invalid */
	while(B_time[z] < A_time[w] && z<Btot) {B_val[z] = invalid; z++;}
for(z;z<Btot;z++)
	{
	while(A_time[w] < B_time[z]) if(++w>=Atot) {for(z;z<Btot;z++) B_val[z]= invalid; return(count);}
	B_val[z] = A_val[w-1];
	count++;
	}
return(count);
}
