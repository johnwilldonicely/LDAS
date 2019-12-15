#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define thisprog "xe-test1"
#define TITLE_STRING thisprog" v 1: 26.September.2016 [JRH]"

/*
<TAGS> math </TAGS>

v 1: 26.September.2016 [JRH]
*/

/* external functions start */
/* external functions end */


int main (int argc, char *argv[]) {

	/* general variables */
	long int ii,jj,kk,ll,mm,nn;
	double aa,bb,cc,dd,ee, result_d[64];

	/* program-specific variables */

	/* arguments */
	char settest[8];
	int setlong=0;
	long setval1_l, setval2_l;
	double setval1_d, setval2_d;

	/* PRINT INSTRUCTIONS IF THERE IS NO FILENAME SPECIFIED */
	if(argc<4) {
		fprintf(stderr,"\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"%s\n",TITLE_STRING);
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"Perform a test on a pair of values\n");
		fprintf(stderr,"USAGE:\n");
		fprintf(stderr,"	%s [val1] [test] [val2] [options]\n",thisprog);
		fprintf(stderr,"	[val1]: value to compare\n");
		fprintf(stderr,"	[val2]: reference value\n");
		fprintf(stderr,"	[test]: test to apply\n");
		fprintf(stderr,"		-eq: equals\n");
		fprintf(stderr,"		-ne: not-equal\n");
		fprintf(stderr,"		-lt: less than\n");
		fprintf(stderr,"		-le: les than or equal to\n");
		fprintf(stderr,"		-gt: greater than\n");
		fprintf(stderr,"		-ge: greater than or equal to\n");
		fprintf(stderr,"VALID OPTIONS: defaults in []\n");
		fprintf(stderr,"	-long: treat inputs as long-integers (0=NO 1=YES [%d])\n",setlong);
		fprintf(stderr,"		- (0) treats input as double-precision float\n");
		fprintf(stderr,"		- (1) use long-integer math for greater precision\n");
		fprintf(stderr,"EXAMPLES:\n");
		fprintf(stderr,"	Test if 1.23 is greater than 4.56:\n");
		fprintf(stderr,"		%s 123 -gt 456\n",thisprog);
		fprintf(stderr,"	Test if 11111 is less than 99999 using long integers:\n");
		fprintf(stderr,"		%s 11111 -lt 99999 -long 1\n",thisprog);
		fprintf(stderr,"OUTPUT:\n");
		fprintf(stderr,"	1: test is true\n");
		fprintf(stderr,"	nothing: test is false\n");
		fprintf(stderr,"----------------------------------------------------------------------\n");
		fprintf(stderr,"\n");
		exit(0);
	}



	/* READ THE FILENAME AND OPTIONAL ARGUMENTS  */
	setval1_l=atol(argv[1]);
	setval1_d=atof(argv[1]);
	snprintf(settest,8,"%s",argv[2]);
	setval2_l=atol(argv[3]);
	setval2_d=atof(argv[3]);

	for(ii=4;ii<argc;ii++) {
		if( *(argv[ii]+0) == '-') {
			if((ii+1)>=argc) {fprintf(stderr,"\n--- Error[%s]: missing value for argument \"%s\"\n\n",thisprog,argv[ii]); exit(1);}
			else if(strcmp(argv[ii],"-long")==0) setlong=atoi(argv[++ii]);
			else {fprintf(stderr,"\n*** %s [ERROR: invalid command line argument \"%s\"]\n\n",thisprog,argv[ii]); exit(1);}
	}}
	if(setlong!=0 && setlong!=1) { fprintf(stderr,"\n--- Error [%s]: invalid -long [%d] must be 0 or 1\n\n",thisprog,setlong);exit(1);}



	if(setlong==0) {
		if(strstr(settest,"-eq")!=NULL) {if(setval1_d==setval2_d) printf("1\n");}
		else if(strstr(settest,"-ne")!=NULL) {if(setval1_d!=setval2_d) printf("1\n");}
		else if(strstr(settest,"-lt")!=NULL) {if(setval1_d<setval2_d) printf("1\n");}
		else if(strstr(settest,"-gt")!=NULL) {if(setval1_d>setval2_d) printf("1\n");}
		else if(strstr(settest,"-le")!=NULL) {if(setval1_d<=setval2_d) printf("1\n");}
		else if(strstr(settest,"-ge")!=NULL) {if(setval1_d>=setval2_d) printf("1\n");}
		else { fprintf(stderr,"\n--- Error [%s]: invalid test [%s]\n\n",thisprog,settest);exit(1);}
	}
	else if(setlong==1) {
		if(strstr(settest,"-eq")!=NULL) {if(setval1_l==setval2_l) printf("1\n");}
		else if(strstr(settest,"-ne")!=NULL) {if(setval1_l!=setval2_l) printf("1\n");}
		else if(strstr(settest,"-lt")!=NULL) {if(setval1_l<setval2_l) printf("1\n");}
		else if(strstr(settest,"-gt")!=NULL) {if(setval1_l>setval2_l) printf("1\n");}
		else if(strstr(settest,"-le")!=NULL) {if(setval1_l<=setval2_l) printf("1\n");}
		else if(strstr(settest,"-ge")!=NULL) {if(setval1_l>=setval2_l) printf("1\n");}
		else { fprintf(stderr,"\n--- Error [%s]: invalid test [%s]\n\n",thisprog,settest);exit(1);}
	}


	exit(0);
}
