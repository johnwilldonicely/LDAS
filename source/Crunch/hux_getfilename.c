/******************************************************************************
Filename parser: return index to filename in full path string
eg. "C:\Data\March23\trial_1.dat" will return 16
Returns zero if no backslashes were found at all
Returns "-1" if backslash was final character in input string 
******************************************************************************/
int hux_getfilename(char *line)
{
    unsigned int i=0;
	int systemtype=-1, filenamestart=-1;

	/* determine if path is of Windows or Unix type */
	while(line[i]!=0) {
		if(line[i]=='/') {systemtype=0;break;} /* Only Unix-type file or path names can contain forward slashes */
		else if(line[i]=='\\') {systemtype=1;break;} /* Only Windows-type file or path names can contain forward slashes */
		i++;
	}

	if(systemtype==-1) return(0); /* if no "\" or "/" found, filename starts at first character */

	i=0;
	if(systemtype==0) while(line[i]!=0) {
		if(line[i]=='/') filenamestart=i; /* zero-offset index of backslash character */ 
		i++; /* at end of loop, this will be the index to the final null character */
	}
	if(systemtype==1) while(line[i]!=0) {
		if(line[i]=='\\') filenamestart=i; /* zero-offset index of backslash character */ 
		i++; /* at end of loop, this will be the index to the final null character */
	}

	filenamestart++; /* filename start is next character after last backslash, or zero if no backslashes were found  */
	if(i<=0||filenamestart==i) return(-1); /* if input length = 0 or if "\" or "/" is the final null character, return -1 error */
	else return(filenamestart);
}
