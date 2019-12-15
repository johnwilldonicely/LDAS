/***********************************************************************
Error handling function
Just send it the message you want displayed before system pause & exit
***********************************************************************/
#include <stdlib.h>
#include <stdio.h>

void hux_error(char message[])
{
	int i=0, stat=0; /* "stat" determines exit status - if no message is specified, exit ok (0) otherwise 1 */
	while(message[i]!=0) i++;
	if(i>0) {stat=1;fprintf(stderr,"\n\n\t --- Error: %s\n\n",message);}
	else printf("\n\n");
	printf("Press ENTER to finish\n"); 
	getc(stdin); 
	exit(stat);
}

