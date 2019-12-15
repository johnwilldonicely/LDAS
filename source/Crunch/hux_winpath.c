/******************************************************************************
Path format converter - replaces Windows "\" directory designator with "\\"
This ensures that programs reading the path don't interpret the first letter 
   of the file or directory following the "\" as a special character
Note that original array must be allocated enough memory to hold the extra "\"s
******************************************************************************/
#include <string.h>
#include <stdio.h>
void hux_winpath(char *line)
{
	char newline[256];
	unsigned int x=0,y=0,length=0,dirdepth=0;
	if(strlen(line)<254)
		{
		while(line[length]!=0){if(line[x]=='\\') dirdepth++;length++;}
		for(x=y=0;x<length;x++)
			{
			if(line[x]=='\\') newline[y++]=line[x];
			newline[y++]=line[x];
			}
		newline[y]='\0';
		strcpy(line,newline);
		return;
		}
	else fprintf(stderr,"Error in hux_winpath: file path name too long!");
}
