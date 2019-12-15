/*******************************************************************************
<TAGS>file</TAGS>
DESCRIPTION:
	Returns the size of a file in bytes - compatible with large files sizes (>2GB)
	NOTE: for windows, use xf_filesize_win.cpp instead
********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
off_t xf_filesize(FILE *fpin)
{
	off_t aaa,bbb;
	aaa=ftello(fpin);		// remember current position
	fseeko(fpin,0L,SEEK_END); 	// go to end of file
	bbb=ftello(fpin);		// remember final position (bytes)
	fseeko(fpin,0L,aaa); 		// go back to original position
	return(bbb);			// return the byte-count
}
