/*
<TAGS>programming</TAGS>

DESCRIPTION:
	Simple error-handling function (prints error message and exits)

DEPENDENCY TREE:
	No dependencies

ARGUMENTS:
	char *message : input message to print before exit

RETURN VALUE:
	none

SAMPLE CALL:
	x= xf_auc1_d(data, nn, interval, result, );
	if(x!=0) xf_err1("PROG,NAME","this is the error",1);
*/

/***********************************************************************
Error handling function
Just send it the message you want displayed before system pause & exit
***********************************************************************/
#include <stdlib.h>
#include <stdio.h>

void xf_err1(char *setfunc, char *setmsg, int space) {

	if(space==1) fprintf(stderr,"\n");
	fprintf(stderr,"*** %s [ERROR: %s]\n",setfunc,setmsg);
	if(space==1) fprintf(stderr,"\n");
	exit(1);
}
