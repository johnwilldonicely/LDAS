/*******************************************************************************
- fill target array (B) with value of most recent value from source array (A)
- this version made for single byte int data which simply holds "flag" type info
- value of flags should not be interpolated, hence previous value is added here
********************************************************************************/
#include <stdio.h>
int hux_fillprev_char (double *Atime, double *Btime, char *Aval, char *Bval, int Atot, int Btot, char invalid)
{
	int a=0,b=0,count=0;
	/* if target data starts first, fill with invalid values */
	while(Btime[b]<Atime[0] && b<Btot) {Bval[b++]=invalid;}
	for(b;b<Btot;b++) {
		while(Atime[a] < Btime[b]) if(++a>=Atot) {for(b;b<Btot;b++) Bval[b]= invalid; return(count);}
		Bval[b] = Aval[a-1];
		count++;
	}
	return(count);
}
