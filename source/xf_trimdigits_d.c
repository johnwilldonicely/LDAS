/************************************************************************************
<TAGS>math</TAGS>
DESCRIPTION:
	Discard "redundant" digits in a number, depending on the number of digits_to_keep
	Eg. if digits_to_keep=3:
		124596 becomes 124000
		12.456 becomes 12.4
		0.0051970 becomes 0.005
	This is useful for simplifying numerical displays
************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double xf_trimdigits_d(double number_to_trim, int digits_to_keep) {
	char line[256],newline[256];
	int w,x,y,z,i;
	snprintf(line,255,"%lf",number_to_trim);

	w=x=y=z=0;
	for(i=0;i<256&&line[i]!='\0';i++) {
		w=line[i];
		if(w>48&&w<58) x=1; // a non-zero digit has been found - now start counting
		if(w==46) y=1; // a decimal has been found
		if(w>47&&w<58&&x==1) z++; // counter for digits after first non-zero
		if(z<=digits_to_keep) {newline[i]=w; continue;}
		else if(y!=1) {newline[i]='0';continue;}
		else break;
	}
	newline[i]='\0';
	return(atof(newline));
}
