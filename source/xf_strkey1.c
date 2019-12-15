/*
<TAGS>string database</TAGS>

DESCRIPTION:
	- scans a NULL-terminated string for value-text which follows a user-specified key
	- stores the output to a NULL-terminated string

USES:
	Get the value associated with a keyword in a file header

DEPENDENCIES:
	None

ARGUMENTS:
	char *input    : input, null-terminated array to scan (can contain newlines)
	char *key      : input, null-terminated keyword to find, newlines ignored
	int word       : input, white-space delimited item-number (after key) to store (-1 = everything up to the newline)
	char *output   : output, the stored text

RETURN VALUE:
	- length of output, or 0 on error (key not found or word not found)
	- the output array will hold the result

SAMPLE CALL:
	- say a long header string contains a line -Resolution = 400 x 250 pixels
	- the "trigger" is "-Resolution" and the data we want is "250" (4th word)

	int data; char header[2025],output[2025];
	x= xf_strkey1(header, "-Resolution", 4, output)>0)
	if(x<=0) { fprintf(stderr,"ERROR: key or word not found\n"); exit(1); }
*/

#include <stdio.h>
#include <stdlib.h>

int xf_strkey1(char *input, char *key, long word, char *output) {

	long x=0,y=0,z=0,outlength=0,wordcount=0;

	/* find occurence of key word in string */
	while(input[z]!='\0' && key[x]!='\0') {
		if(input[z]==key[x]) x++;
		else x=0;
		z++;
	}
	/* scan past key until right number of words is reached */
	while(input[z]!='\0') {
		y=z-1;
		if((input[y]==' '||input[y]=='\t'||input[y]=='\n') && (input[z]!=' '||input[z]!='\t'||input[z]!='\n')) wordcount++;
		if(wordcount>=word) break;
		else z++; /* note that position counter does not incriment on itteration when word is found */
	}
	/* store output - if key is not found, output is simply a NULL character and zero is returned */
	if(word==-1) {
		while(input[z]!='\0'&&input[z]!=10&&input[z]!=12) {
		output[outlength]=input[z];
		z++; outlength++;
	}}
	else {
		while(input[z]!='\0'&&input[z]!=' '&&input[z]!='\t'&&input[z]!='\n') {
		output[outlength]=input[z];
		z++; outlength++;
	}}

	output[outlength]='\0';
	return(outlength);

}
