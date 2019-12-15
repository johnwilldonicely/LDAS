/******************************************************************************
- scans a NULL-terminated string for a target which follows a known trigger
- stores the target to a NULL-terminated string
- calling function mucst allocate sufficient (strlen(trigger)) memory for target 
- ignores newlines in trigger string
- USAGE..
- say a long header string contains a line -Resolution = 400 x 250 pixels
- the "trigger" is "-Resolution" and the data we want is "250" (4th word) 
- Sample call...
	int data; char header[2025],target[2025];
	if(hux_getword(header, "-Resolution", 4, target)>0) data=atoi(target);
- If "word" = -1, then everything (and SPACES!) up to (not including) the next newline is copied to "target"
******************************************************************************/
extern void hux_error(char message[]);
#include <stdio.h>
#include <stdlib.h>

int hux_getword(char *line,char *trigger, int word, char *target)
{ 
	int x=0,y=0,z=0,line_len=0,trigger_len=0,wordcount=0;
	/* find occurence of trigger word in string */
	while(line[z]!='\0' && trigger[x]!='\0') {
		if(line[z]==trigger[x]) x++; 
		else x=0;
		z++;
	}
	/* scan past trigger until right number of new words is reached */
	while(line[z]!='\0') {
		y=z-1;
		if((line[y]==' '||line[y]=='\t'||line[y]=='\n') && (line[z]!=' '||line[z]!='\t'||line[z]!='\n')) wordcount++;
		if(wordcount>=word) break;
		else z++; /* note that position counter does not incriment on itteration when word is found */
	}
	/* store target - if trigger is not found, target is simply a NULL character and zero is returned */
	if(word==-1) {
		while(line[z]!='\0'&&line[z]!=10&&line[z]!=12) {
		target[line_len]=line[z]; 
		z++; line_len++;
	}}
	else {
		while(line[z]!='\0'&&line[z]!=' '&&line[z]!='\t'&&line[z]!='\n') {
		target[line_len]=line[z]; 
		z++; line_len++;
	}}

	target[line_len]='\0';
	return(line_len);
	
}
