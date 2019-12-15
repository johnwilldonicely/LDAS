/******************************************************************************
Find filename extention (find position of final "." in filename
Returns "-1" if no "." was found, or strlen(line) "." was last character in filename
Eg. to print extention... printf("Extention: %s\n",filename+hux_getext(filename));
******************************************************************************/
int hux_getext(char *line)
{
	unsigned int i=0;
	int extpos=-1;
	while(line[i]!=0) {
		if(line[i]=='.') extpos=i; /* zero-offset index of "." character - cannot actually be zero for valid filenames! */ 
		i++; /* at end of loop, this will be the index to the final null character */
	}
	extpos++; /* this now indicates first character after last "." */
	if(extpos==0) extpos=-1; /* result if "." is not found */
	//if(extpos==i) extpos=0; /* result if "." is last non-NULL character i nfilename */
	return(extpos);
}
