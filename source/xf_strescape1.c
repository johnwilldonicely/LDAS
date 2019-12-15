/*
<TAGS>string</TAGS>

DESCRIPTION:
	Replace literal escape sequences in a string with the appropriate ascii codes
	A slash followed by any of the following will be replaced by it's escape character:

		\\  (back-slash)
		\'  (single-quote)
		\"  (double-quote)
		\b  (back-space)
		\n  (new-line)
		\f  (form-feed)
		\t  (tab)
		\?  (question mark)

USES:
	Converting command-line arguments

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *string
		- input, pointer to a null-terminated character array

RETURN VALUE:
	A pointer to the converted string
	- always same length or shorter than original

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *xf_strescape1(char *line) {

	long i,j,k;
	size_t n;

	n=strlen(line)-1;
	if(n<0||line[0]=='\n') {return(line);}

	for(i=j=0;i<n;i++) {
		if(line[i]==92) {
			k=i+1;
			if		(line[k]=='\\'){ line[j]='\\' ; i++; }
			else if	(line[k]=='\''){ line[j]='\'' ; i++; }
			else if (line[k]=='"') { line[j]='\"' ; i++; }
			else if (line[k]=='?') { line[j]='\?' ; i++; }
			else if	(line[k]=='b') { line[j]='\b' ; i++; }
			else if (line[k]=='f') { line[j]='\f' ; i++; }
			else if (line[k]=='n') { line[j]='\n' ; i++; }
			else if (line[k]=='t') { line[j]='\t' ; i++; }
			else line[j]='\\';
		}
		else line[j]=line[i];
		j++;
	}
	line[j]=line[i];
	line[j+1]='\0';
	return(line);
}
