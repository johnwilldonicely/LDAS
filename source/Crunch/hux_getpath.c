/******************************************************************************
Path parser: takes pointer to character array and terminates array after the 
last "\" (Windows) or "/" (Unix/Linux) - that is, it truncates a full file 
name path to the lowest directory
******************************************************************************/
#include <stdio.h>
int hux_getpath(char *line)
{
    unsigned int i=0,end=0;
	int systemtype=-1;

	/* determine if path is of Windows or Unix type */
	while(line[i]!=0) {
		if(line[i]=='/') {systemtype=0;break;} /* Only Unix-type file or path names can contain forward slashes */
		else if(line[i]=='\\') {systemtype=1;break;} /* Only Windows-type file or path names can contain forward slashes */
		i++;
	}

	i=0;
	if(systemtype==-1) {line[0]=0;return(systemtype);} /* if no "\" or "/" was found, input is filename, no changes made */
	else if(systemtype==0) while(line[i]!=0) {if(line[i]=='/') end=i; i++;}
	else if(systemtype==1) while(line[i]!=0) {if(line[i]=='\\') end=i; i++;}
    line[end+1]=0; /* terminate input string after last "\" or "/" */
	return(systemtype);
}
