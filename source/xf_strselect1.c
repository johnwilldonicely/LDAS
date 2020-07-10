/*
<TAGS>string</TAGS>

DESCRIPTION:
	Reduce a string by keeping only a set of delimited "words"

DEPENDENCIES:
	None

ARGUMENTS:
	char *input : pointer to a character array
	char delimiter : a single delimiter which separates "input" into words
	long *index1 ; a list (zer0-offset array) of the words in "input" to be kept
	long nn ; size of the index1 array

RETURN VALUE:
	on success: 0, input will be modified

SAMPLE CALL:
	char input[]= "zero,one,two,three,four,five,siz,seven\n";
	long ii,keep[]= {1,3,5};
	ii= xf_strselect1(input,',',keep,3);
	printf("input=%s\n",input);

--------------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int xf_strselect1(char *input, char delimiter, long *index1, long nn) {
	int found;
	long ii,jj,kk,word,len1,indexmax=-1;

	/* DETERMINE LENGTH OF INPUT */
	len1= strlen(input); if(len1<1) return(-1);

	/* DETERMINE THE HIGEST INDEX - ERROR IF ANY INDEX IS <0 */
	for(ii=0;ii<nn;ii++) if(index1[ii]>indexmax) indexmax=index1[ii];

	word=0; // at the start, we assume this is word-0
	kk=0;	// pointer to write-position in input
	for(ii=0;ii<len1;ii++) {
		if(input[ii]==delimiter) { word++ ; continue; }
		found= 0;
		for(jj=0;jj<nn;jj++) {
			if(word==index1[jj]) {
				found= 1;
				for(ii=ii;ii<len1;ii++) {
					input[kk]= input[ii];
					kk++;
					if(input[ii]==delimiter) {
						word++;
						if(word>=indexmax) input[ii]='\0';
						break;
			}}}
			if(found==1) break;
	}}

	if(kk<len1) input[kk]='\0';
	return(0);
}
