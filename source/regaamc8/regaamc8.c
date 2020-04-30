/* JOZSEF CSICSVARI'S DAT/EEG  FILE VIEWER

TO COMPILE:
	gcc regaamc8.c xnsubs.c -o ./regaamc8 -lm -lX11 -L /usr/X11R6/lib -Wno-write-strings
	also requires these files in the compile-directory
		xnsubs.c
		xnsubs.h
		icon_bitmap

DEPENDENCIES:
	xorg-x11-fonts-misc.noarch

	Potential dependencies:
		imlib.i686
		imlib-devel.i686

		...or the 64-bit equivalents...
		imlib.x86_64
		imlib-devel.x86_64

RUNNING THE PROGRAM:
	regaamc8 [binaryfile] [nchans] [ms] [configfile]

	- binaryfile: multi-channel binary timeseries, typically short-integers
	- nchans: the number of interlaced channels (typically 16, 64, etc)
	- ms: the sample-interval (1/samplrate) - eg 50 = 20KHz
	- config: config file...

# CONFIG FILE LAYOUT (COMMENTS MUST BE EXCLUDED FOR FILE TO WORK)
# THIS EXAMPLE IS FOR A 16-CHANNEL RECORDING
	regaa2.0
	0			# prtypelo or prtypefl = ???
	965			# cx= window width
	1095			# cy= window height
	30			# xs= ???
	500			# ys= ???
	10			# res= starting zoom (time)
	1			# ncol= fraction of data to show in time-axis
	16			# number of channels
	0 0 64 0.02 1		# channel zero position gain colour
	1 0 128 0.02 1
	2 0 192 0.02 1
	3 0 256 0.02 1
	4 0 320 0.02 1
	5 0 384 0.02 1
	6 0 448 0.02 1
	7 0 512 0.02 1
	8 0 576 0.02 1
	9 0 640 0.02 1
	10 0 704 0.02 1
	11 0 768 0.02 1
	12 0 832 0.02 1
	13 0 896 0.02 1
	14 0 960 0.02 1
	15 0 1024 0.02 1
	0			# end of file padding
	0
	0
	0
	0
	0

KEYBOARD SHORTCUTS"
	[Esc]: quit "
	[left] [right]: navigate"
	[left mouse]: select and move trace"
	< >: increase decrease timescale"
	i d: increase devcrease y-axis"
	g: graph at bottom of screen"
	a: ? add new column
	A: ? add new quarter-column
	s: ? swap channel

	m: activate pointer mode"
		c: centre screen on pointer"
		l: align screen left to pointer"
		r: align screen right to pointer"
		s: ?
		x: leave pointer axis on he screen

	F8: save configuration"
	F9: load configuration"

	P: print to file: regaamcplot eps to produces ouput called \"jj.eps\""
	p: write a pointer file

	T: open spike-time file
	t: ? similar to above, no prompt?

	C: open cluster-ID file
	c: open a .clu file - no prompt

	H: open whl file
	h: ? similar to above, no prompt?

	R: ? raster file?
	r: as above?
	
	x,X: ? extract to file
	b: open bookmark file

	Note: when using bookmark files...
		- time values in the file will have to be divided by 16 for .eeg and .eegh files
		- divide by 4 for .fex files.

*/

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "xnsubs.h"
#define MAX_CH 128
#define MAX_CL 320
#define MARKCOLOR 2
#define FOREGRCOLOR 2
#define BACKGRCOLOR 0

#define DETECT_COLOR WHITE

#define ASK_X 200
#define ASK_Y 400
#define ASK_HEI 60

FILE *fppr;
char prfl=0;
int cxo,cyo,reso;
int lac;
char **lav;

/* raster plot variables*/
char rasfl=0;
char ralifl;
char **rasbuf;
int rapos[MAX_CL],raon[MAX_CL],raord[MAX_CL],rawi,raskip;
int rastart,raend;

int mulres=1; /* variable to devide time file */
int clno,clon;
int bitn;
int chno;
float smrate;
char expfl=0; /*expand mode fl in multicolom mode */
int expch=0; /*which colom to expand*/
int idchan=0; /* which colomn to show on th side */

int chon[MAX_CH];


#define MAX_TEXT 100 /* maximum custom text labels on screen */
struct screen_text {
	int x,y,col;
	char st[100];
} stx[MAX_TEXT];
int stxno=0;
char selflch=0; /*character selection flag */

struct tracetype{
	int nocol,cnon;
	int chposx[MAX_CH];
	int chposy[MAX_CH];
	int conpo[MAX_CH];
	int chcol[MAX_CH];
	float muly[MAX_CH];
} ;
struct tracetype tr1,tr2,*tr;
int cx,cy,res,cxread;
int xs,ys;
int skipv=0;
long long filepo=0L;
long long buffp;
char selfl;
int selch;
char poifl;
char rawifl=0;

char poimode=0;

int poipos=0;
FILE *fpd;

short *dbuff;
long long startpos=0L;
short **minx,**maxx;
char redrfl;
int detfl=0;
int clufl=0;
char *dtbuf;
FILE *fpdt;
FILE *fpcl;
char det_name[100];
char clu_name[100];
char xwindow_open=0;
int palette[MAX_COLORS];
int bmfl=0;
char bmflnm[100];
FILE *bmfp;
int pofl=0;
char poiflnm[100];
FILE *poifp;
int scrhead,scrheado; /*screan header*/
int cgap=10,cgapo; /* colom gap */


int prtype=1; /* processing type in load_buff */
int prtypefl=0;
char prfilename[100];
int chnoin;
int pr_ncol,pr_nrow;
int pr_id[16][128];
int pr_nwin;
float pr_win[128];
int pr_nwx,pr_nwy;
float pr_wxy[32][32];

#define NINTP 100 /* how many coloms max to intepolate */
#define NTOINTP 100 /* how many chnannels per coloms max to intepolate */

char intpfl=0; /* inteploate flag*/
int n_colom_to_intp;
int n_intp_incolom[NINTP];
int intp_which_colom[NINTP];
int intp_incolom[NINTP][NTOINTP];
static int pale[]={3,4,6,7,0,2,19,12,5}; /* colors order for traces */

int whlfl,whllen; /* wheel file loaded */
float *whlx,*whly; /* animal coordinates */
char whl_name[100];


int extract_fl=0; /* file extract flag */
FILE *fpextr;


int yaxison=0; /* flag to display y axis after pointer mode leaving by x */
int yaxispos;
char dataflnm[100],basenm[100];

char gridtype=1;

int print_only=0; /* print only without opening xwindow */

/*long long mftello(FILE *fp)
{
  fpos_t pos;
  void *po;
  long long a;
  fgetpos(fp,&pos);
  po=&pos;
  a=*(long long*)po;
  return a;
}
*/






long long mftello(FILE *fp)
{
  __off64_t pos;
  long long a;
  pos=ftello(fp);
  a=(long long)pos;
  return a;
}
/*int mfseeko(FILE *fp, long long posi, int whence)
{
  long int seg;
  long long a;
  int ret;
  fpos_t pos;
  void *po;

  if ((whence==SEEK_CUR)||(whence==SEEK_END))
    { seg=(long)posi; ret=fseeko(fp,seg,whence);}
  else {
    fgetpos(fp,&pos);
    po=&pos;
    a=posi;
    memcpy(po,&a,8);
    ret=fsetpos(fp,&pos);
  }
  return ret;
}*/


int mfseeko(FILE *fp, long long posi, int whence)
{
  int ret;
  __off64_t pos;
  pos=(__off64_t)posi;
  ret=fseeko(fp,pos,whence);
  return ret;
}

void setpalette()
{
  int i;
  palette[0]=0;
  palette[1]=31;
  palette[2]=1;
  palette[3]=19;
  palette[4]=17;
  palette[5]=28;
  palette[6]=12;
  palette[7]=20;
  palette[8]=29;
  palette[9]=10;
  palette[10]=11;
  palette[11]=22;
  palette[12]=7;
  palette[13]=9;
  palette[14]=30;
  palette[15]=8;
  palette[16]=5;
  palette[17]=13;
  palette[18]=16;
  palette[19]=24;
  palette[20]=25;
  palette[21]=26;
  palette[22]=27;
  palette[23]=15;
  palette[24]=23;
  for(i=25;i<MAX_COLORS;i++) palette[i]=i;
}



#define MAXCOLOR 9
/* reverse lookup for channel colr table */
int lookup_pale(int color)
{
  int po=0;
  while((pale[po]!=color)&&(po<MAXCOLOR)) po++;
  return po;
}



void msetcolor(int col,int win)
{
  if (prfl)
    fprintf(fppr,"C %i\n",col);
  else
  setcolor(col,win);
}

mlinewidth(int a,int b)
{
 if (!prfl)
   linewidth(a,b);
}
int mline(int x1,int y1,int x2,int y2,int win)
{
  static int prx=0,pry=0; /* previous end of line */
  if (prfl)
    {
      if ((x1!=x2)||(y1!=y2))
	{
	  if ((prx==x1)&&(pry==y1))
	  fprintf(fppr,"L %i %i\n",x2,-y2);
	  else fprintf(fppr,"M %i %i\nL %i %i\n",x1,-y1,x2,-y2);
	  prx=x2;pry=y2;
	}
      /*else  fprintf(fppr,"%i %i \n%i %i\n\n",x1,-y1,x2,-y2-1); */
    }
  else  line(x1,y1,x2,y2,win);

}

mtext(int x, int y,char str[],int win)
{
  if (prfl) {
    fprintf(fppr,"S %i %i %s\n",x,-y,str);
  }
 else outtextxy(x,y,str,win);
}




int sclen()
{
  if (res>0) return cx*res; else return -cx/res;
}

void buffprconf()
{
 FILE *fp;
 int i,j;
 if (prtypefl==0)
   {
     chno=chnoin;
   }
 else
   {

      if ((fp=fopen(prfilename,"r"))==NULL)
	{printf("Error opening process file name\n");exit(1);}
      if (fscanf(fp,"%i",&prtype)!=1)
	{printf("Error reading prtype \n");exit(1);}
      if ((prtype==2)||(prtype==4))
	{
	  chno=chnoin;
	  if (fscanf(fp,"%i %i",&pr_ncol,&pr_nrow)!=2)
	    {printf("Error reading process file1 \n");exit(1);}
	  for(i=0;i<pr_ncol;i++)
	    for(j=0;j<pr_nrow;j++)
	      {
		if (fscanf(fp,"%i",&pr_id[i][j])!=1)
		  {printf("Error reading process file 2\n");exit(1);}
		if (pr_id[i][j]>=chno) pr_id[i][j]=0;
	      }
	  if (fscanf(fp,"%i",&pr_nwin)!=1)
	    {printf("Error reading process file 3\n");exit(1);}
	  for(i=0;i<pr_nwin;i++)
	    if (fscanf(fp,"%f",&pr_win[i])!=1)
	      {printf("Error reading process file 4 \n");exit(1);}
	}
      else if (prtype==3)
	{
	  chno=chnoin;
	  if (fscanf(fp,"%i %i",&pr_ncol,&pr_nrow)!=2)
	    {printf("Error reading process file1 \n");exit(1);}
	  for(i=0;i<pr_ncol;i++)
	    for(j=0;j<pr_nrow;j++)
	      {
		if (fscanf(fp,"%i",&pr_id[i][j])!=1)
		  {printf("Error reading process file 2\n");exit(1);}
		if (pr_id[i][j]>=chno) pr_id[i][j]=0;
	      }
	  if (fscanf(fp,"%i %i",&pr_nwx,&pr_nwy)!=2)
	    {printf("Error reading process file 3\n");exit(1);}
	  for(i=0;i<pr_nwx;i++)
	    for(j=0;j<pr_nwy;j++)
	      if (fscanf(fp,"%f",&pr_wxy[i][j])!=1)
	       {printf("Error reading process file 4 \n");exit(1);}
	}

      if (prtype==4)
	{
	  intpfl=1;
	  if (fscanf(fp,"%i",&n_colom_to_intp)!=1)
	    {printf("Error reading process file 4-1 \n");exit(1);}
	  for(i=0;i<n_colom_to_intp;i++)
	    {
	      if (fscanf(fp,"%i",&n_intp_incolom[i])!=1)
		{printf("Error reading process file 4-2 \n");exit(1);}
	      if (fscanf(fp,"%i",&intp_which_colom[i])!=1)
		{printf("Error reading process file 4-3 \n");exit(1);}

	      for(j=0;j<n_intp_incolom[i];j++)
		if (fscanf(fp,"%i",&intp_incolom[i][j])!=1)
		  {printf("Error reading process file 4-4 \n");exit(1);}

	    }
	}
      fclose(fp);
   }

}
void load_config(FILE *fpc,int mwi)
{
 int chcr,i;
 int crnon;
 char st[100];
 int prtypelo;

 if (fscanf(fpc,"%s",st)!=1)  {printf("Error reading config file header\n");exit(1);}
 prtype=1;

 if (!strcmp(st,"regaa2.0")) {
     if (fscanf(fpc,"%i",&prtypelo)!=1) {printf("Error reading prtypefl\n");exit(1);}
     if (prtypelo) {
	 	if (fscanf(fpc,"%s",&st)!=1) {printf("Error process file name\n");exit(1);}
	 	if (prtypefl==0) {prtypefl=prtypelo; strcpy(prfilename,st);}
     }
   }
 buffprconf();
 if (fscanf(fpc,"%i",&cx)!=1)
   {
     printf("Error reading config file cx\n");
     exit(1);
   }
 if (fscanf(fpc,"%i",&cy)!=1)
   {
     printf("Error reading config file cy\n");
     exit(1);
   }

 if (fscanf(fpc,"%i",&xs)!=1)
   {
     printf("Error reading config file xs\n");
     exit(1);
   }
 if (fscanf(fpc,"%i",&ys)!=1)
   {
     printf("Error reading config file ys\n");
     exit(1);
   }
 if (fscanf(fpc,"%i",&res)!=1)
   {
     printf("Error reading config file res\n");
     exit(1);
   }
 if (fscanf(fpc,"%i",&(tr->nocol))!=1)
   {
     printf("Error reading config file nocol\n");
     exit(1);
   }
 if (fscanf(fpc,"%i",&(tr->cnon))!=1)
   {
     printf("Error reading config file cnon\n");
     exit(1);
   }


 for(i=0;i<chno;i++) chon[i]=0;
 for(i=0;i<(tr->cnon);i++)  {
     if (fscanf(fpc,"%i",&(tr->conpo)[i])!=1) {
	 printf("Error reading config file conpo\n");
	 exit(1);
 	}
     if (fscanf(fpc,"%i",&(tr->chposx)[i])!=1) {
	 printf("Error reading config file chposx\n");
	 exit(1);
       }
     if (fscanf(fpc,"%i",&(tr->chposy)[i])!=1) {
	 printf("Error reading config file chposy)\n");
	 exit(1);
       }
     if (fscanf(fpc,"%f",&(tr->muly)[i])!=1) {
	 printf("Error reading config file muly\n");
	 exit(1);
       }
     if (fscanf(fpc,"%i",&(tr->chcol)[i])!=1) {
	 printf("Error reading config file chcol\n");
	 exit(1);
       }
   }
 crnon=0;
 for(i=0;i<(tr->cnon);i++)
   {
     if ((tr->conpo)[i]<chno)
       {
         chon[(tr->conpo)[i]]++;
	 (tr->conpo)[crnon]=(tr->conpo)[i];
	 (tr->chposx)[crnon]=(tr->chposx)[i];
	 (tr->chposy)[crnon]=(tr->chposy)[i];
	 (tr->muly)[crnon]=(tr->muly)[i];
	 (tr->chcol)[crnon]=(tr->chcol)[i];
	 crnon++;
       }
   }
  (tr->cnon)=crnon;

 if ((tr->cnon)==0)
   {
     chon[0]=1;
     (tr->conpo)[0]=0;
     (tr->chposy)[0]=cy/2;
     (tr->chposx)[0]=0;
     (tr->muly)[0]=0.2;
     (tr->chcol)[0]=3;
     (tr->cnon)=1;
   }

 if (fscanf(fpc,"%li",&buffp)!=1)
   {
     printf("Error reading config file buffp\n");
     exit(1);
   }
 fscanf(fpc,"%i",&detfl);
 if (detfl)
   {
      fscanf(fpc,"%s\n",det_name);
      open_detfile(mwi);
      fscanf(fpc,"%i",&clufl);
      if (clufl)
	{
	  fscanf(fpc,"%s\n",clu_name);
	  open_clufile(mwi);
	  fscanf(fpc,"%i",&rasfl);
	  if (rasfl)
	    {
	      fscanf(fpc,"%i %i %i",&clon,&rastart,&raend);
	      for(i=1;i<=clno;i++)
		fscanf(fpc,"%i %i",&raon[i],&raord[i]);

	      init_rapos();
	    }
	}
      fscanf(fpc,"%i",&mulres);
   }
 bmfl=0;
 if (fscanf(fpc,"%i",&bmfl)!=1) bmfl=0;
 if (bmfl)
   {
      fscanf(fpc,"%s\n",bmflnm);
      open_bmfile(mwi);
   }
 pofl=0;
 if (fscanf(fpc,"%i",&pofl)!=1) pofl=0;
 if (pofl)
   {
      fscanf(fpc,"%s\n",poiflnm);
      open_poifile(mwi);
   }

 if (fscanf(fpc,"%i",&stxno)!=1) stxno=0;
 for(i=0;i<stxno;i++)
   {
     if (fscanf(fpc,"%i %i %i",&(stx[i].x),&(stx[i].y),&(stx[i].col))!=3)
       {
	 printf("Erorr redins screen text\n");
	 stxno=i;break;
       }
     fgetc(fpc); /* reads the separating space */
     fgets(stx[i].st,100,fpc);
     stx[i].st[strlen(stx[i].st)-1]=0;
   }

 if (fscanf(fpc,"%i",&yaxison)!=1) yaxison=0;

 if (yaxison)
   if(fscanf(fpc,"%i",&yaxispos)!=1)
     {
       printf("Erorr redins screen text\n");
       stxno=i;goto ki;
     }

 ki:
 fclose(fpc);
}





void init_config()
{
 int i;


 cx=800;
 cy=600;
 xs=30;
 ys=50;
 res=2;
 tr->nocol=1;
 tr->cnon=1;

 prtypefl=0;
 prtype=1;
 chno=chnoin;
 for(i=0;i<chno;i++) chon[i]=0;
 chon[0]=1;
 (tr->conpo)[0]=0;
 (tr->chposy)[0]=cy/2;
 (tr->chposx)[0]=0;
 (tr->muly)[0]=0.1;
 (tr->chcol)[0]=3;

 buffp=0;
 detfl=0;
 bmfl=0;
 pofl=0;
 stxno=0;
}

void save_config(FILE *fpc)
{
 int chcr,i;

 fprintf(fpc,"regaa2.0\n%i\n",prtypefl);
 if (prtype>1)
   fprintf(fpc,"%s\n",prfilename);
 fprintf(fpc,"%i\n",cx);
 fprintf(fpc,"%i\n",cy);


 fprintf(fpc,"%i\n",xs);
 fprintf(fpc,"%i\n",ys);
 fprintf(fpc,"%i\n",res);
 fprintf(fpc,"%i\n",tr->nocol);
 fprintf(fpc,"%i\n",(tr->cnon));
 for(i=0;i<(tr->cnon);i++)
   {
     fprintf(fpc,"%i ",(tr->conpo)[i]);
     fprintf(fpc,"%i %i ",(tr->chposx)[i],(tr->chposy)[i]);
     fprintf(fpc,"%f ",(tr->muly)[i]);
     fprintf(fpc,"%i\n",(tr->chcol)[i]);
   }
 fprintf(fpc,"%li\n",buffp);
 fprintf(fpc,"%i\n",detfl);
 if (detfl)
   {
      fprintf(fpc,"%s\n",det_name);
      fprintf(fpc,"%i\n",clufl);
      if (clufl) {
	fprintf(fpc,"%s\n",clu_name);
	fprintf(fpc,"%i\n",rasfl);
	if (rasfl)
	    {
	      fprintf(fpc,"%i %i %i\n",clon,rastart,raend);
	      for(i=1;i<=clno;i++)
		fprintf(fpc,"%i %i\n",raon[i],raord[i]);

	      init_rapos();
	    }
      }
      fprintf(fpc,"%i\n",mulres);
   }
 fprintf(fpc,"%i\n",bmfl);
 if (bmfl)
   {
     fprintf(fpc,"%s\n",bmflnm);
   }
 fprintf(fpc,"%i\n",pofl);
 if (pofl)
   {
     fprintf(fpc,"%s\n",poiflnm);
   }

 fprintf(fpc,"%i\n",stxno);
 for(i=0;i<stxno;i++)
   {
     fprintf(fpc,"%i %i %i %s\n",stx[i].x,stx[i].y,stx[i].col,stx[i].st);
   }

 fprintf(fpc,"%i\n",yaxison);
 if (yaxison==1)
    fprintf(fpc,"%i\n",yaxispos);

 fclose(fpc);
}

move_end()
{
  long aa;
  aa=(sclen()*2*chnoin);
  if (fseeko(fpd,-aa,SEEK_END)<0)
    {
      printf("Error seeking data file\n");
      exit(1);
    }
  buffp=filepo=mftello(fpd)/chnoin/sizeof(short);
}


update_raster(char *selel[],char mask[],int cx,int cy,int cy1,int inspo)
{
 int i,ii,po,idx;
  for(i=1,po=0;i<=clno;i++) selel[i][0]=mask[i]=0;
  for(i=cx*cy1,ii=1;i<cx*cy1+clno;i++,ii++)
    {
      sprintf(selel[i],"%i",ii);
      mask[i]=raon[ii];
    }

  if (clon<clno)
    {
      sprintf(selel[inspo],"ins");
      mask[inspo]=0;
    }
  for(i=1,po=0;i<=clno;i++)
    {
      if (raon[i])
	{
	  idx=raord[i]-1;
	  if (clon<clno)
	    {
	      if (inspo<=idx) idx++;
	    }
	  sprintf(selel[idx],"%i",i);
	  mask[idx]=1;
	}
    }
}
order_raster(int mwi,int mkey)
{
  char *selel[MAX_CL*3];
  char mask[MAX_CL*3];
  int i,j,k,po,inspo,jj,ii;
  int cx,cy,cy1,cdisp,rpos;
  int raono[MAX_CL],raordo[MAX_CL];
  cx=12;
  cdisp=clno+4;
  if ((cdisp%cx)==0) cy1=cdisp/cx;
  else cy1=cdisp/cx+1;
  if (cdisp<cx) {cx=cdisp;cy1=1;}
  cy=cy1*2;
  for(i=clon=1;i<=clno;i++) {
    if (raon[i]) clon++;
    raono[i]=raon[i];
    raordo[i]=raord[i];
  }
  clon--;
  inspo=clon;

  for(i=0;i<cx*cy;i++) selel[i]=(char*)malloc(10);
  for(i=0;i<cx*cy;i++) selel[i][0]=mask[i]=0;
  update_raster(selel,mask,cx,cy,cy1,inspo);
  strcpy(selel[cx*cy-2],"Cancel");
  strcpy(selel[cx*cy-1],"Done");
  strcpy(selel[cx*cy1-2],"Clear");
  strcpy(selel[cx*cy1-1],"Arr");
  select_out(mwi+1,cx,cy,mask,selel,"Channels",lac,lav);
  while(1)
    {
      select_wait(mwi+1,&ii,&jj);
      if (jj>=cy1)
	{
	  rpos=(jj-cy1)*cx+ii+1;
	  if (rpos<=clno)
	    {
	      if (raon[rpos])
		{
		  raon[rpos]=0;
		  for(i=1;i<=clno;i++)
		    if (raord[i]>raord[rpos]) raord[i]--;
		  clon--;
		}else if (clon<clno)
		  {
		    raon[rpos]=1;
		    for(i=0;i<=clno;i++)
		      if (raord[i]>inspo) raord[i]++;
		    raord[rpos]=inspo+1;
		    clon++;
		    inspo++;
		  }

	    }

	}
      rpos=cx*jj+ii;
      if ((rpos<=clon)&&(clon<clno))
	{
	  inspo=rpos;
	}
      if (rpos==cx*cy-1) break;
      if (rpos==cx*cy1-2) {
	for(i=clon=1;i<=clno;i++) {
	  raon[i]=0;
	  raord[i]=0;
	}
	clon=0;
      }
      if (rpos==cx*cy1-1) {
	for(i=1;i<=clno;i++) {
	  raon[i]=1;
	  raord[i]=i;
	}
	clon=clno;
      }
      if (rpos==cx*cy-2){
	for(i=clon=1;i<=clno;i++) {
	  raon[i]=raono[i];
	  raord[i]=raordo[i];
	}
	break;
      }

      if (inspo>clon) inspo=clon;
      update_raster(selel,mask,cx,cy,cy1,inspo);
      select_update(mwi+1,cx,cy,mask,selel);
    }
  redrfl=1;
  init_rapos();
  close_window(mwi+1);
}

#define MULRESLEV 6
which_mulres(int mwi,int mkey)
{
  char *selel[MULRESLEV];
  char mask[MULRESLEV];
  int i,j,k;
  int po;

  for(i=0;i<MULRESLEV;i++) selel[i]=(char*)malloc(10);
      po=0;
      sprintf(selel[po],"1");
      mask[po++]=0;
      sprintf(selel[po],"4");
      mask[po++]=0;
      sprintf(selel[po],"8");
      mask[po++]=0;
      sprintf(selel[po],"16");
      mask[po++]=0;
      sprintf(selel[po],"32");
      mask[po++]=0;
      sprintf(selel[po],"Cancel");
      mask[po++]=0;
      select_out(mwi+1,MULRESLEV,1,mask,selel,"Compression",lac,lav);
      select_wait(mwi+1,&i,&j);
     if (i==0) mulres=1;
     if (i==1) mulres=4;
     if (i==2) mulres=8;
     if (i==3) mulres=16;
     if (i==4) mulres=32;
  close_window(mwi+1);
}

load_detect()
{
  static int cxp=0;
  long int pdtpos=0L,pclpos=0L,tmp,tmpcl;
  long int stt,endt,tm;
  int i,j,pos,clu;
  static char raspr=2;

  if ((cxp<cx)||(raspr!=rasfl))
    {
      if (rasfl)
	{
	  if (rasbuf!=NULL)
	    {
	      for(i=0;i<=clno;i++) free(rasbuf[i]);
	      free(rasbuf);
	    }
	  rasbuf=(char**)calloc(clno+1,sizeof(char*));
	  if (rasbuf==NULL)
	    {
	      printf("Error allocating memory");
	      exit(1);
	    }
	  for(i=0;i<=clno;i++)
	    if ((rasbuf[i]=(char*)calloc(cx,sizeof(char)))==NULL)
	      {
		printf("Error allocating memory");
		exit(1);
	      }
	  cxp=cx;
	}
      else{
	if (dtbuf!=NULL)
	  free(dtbuf);

	dtbuf=(char*)calloc(cx,sizeof(char));

	if (dtbuf==NULL)
	  {
	    printf("Error allocating memory");
	    exit(1);
	  }
	cxp=cx;
      }
      raspr=rasfl;
    }

  stt=startpos;
  endt=stt+sclen();

  fseek(fpdt,pdtpos,SEEK_SET);
  if (clufl)   {
    fseek(fpcl,pclpos,SEEK_SET);
    if (pclpos==0L) fscanf(fpcl,"%i",&clno); /* reading cluster number */
	       }

  if (rasfl)
    for(i=0;i<cx;i++)
      for(j=0;j<=clno;j++) rasbuf[j][i]=0;
  else  for(i=0;i<cx;i++) dtbuf[i]=0;

  if (fscanf(fpdt,"%li",&tm)<1)
    {
      printf("Error reading detect file\n");
      exit(1);
    }
  tm=tm/mulres;
  if (clufl)
    if (fscanf(fpcl,"%i",&clu)<1)
      {
      printf("Error reading cluster file\n");
      exit(1);
    }

  if (tm>stt) { rewind(fpdt);
		if (clufl) {
		  rewind(fpcl);
		  fscanf(fpcl,"%i",&clno); /* reading cluster number */
		}
	      }
  do{

    tmp=ftell(fpdt);
    if (clufl) tmpcl=ftell(fpcl);
    if (fscanf(fpdt,"%li",&tm)<1)
    {
      break;
    }
    tm=tm/mulres;
    if (clufl)
      if (fscanf(fpcl,"%i",&clu)<1)
	{
	  break;
	}
    if (tm<stt) {
      if (!feof(fpdt)) { pdtpos=tmp;
			 if (clufl) pclpos=tmpcl;
		       }
      else break;
    }
  } while(tm<stt);

  while((tm>=stt)&&(tm<endt))
    {
      pos=tm-stt;
      pos=cx*pos/sclen();
      if ((rasfl)&&(raon[clu]))
	{
	  rasbuf[clu][pos]=1;
	}
      if (!rasfl){
	if (clufl) dtbuf[pos]=clu;
	else dtbuf[pos]=1;
      }
      if (fscanf(fpdt,"%li",&tm)<1)
	{
	  break;
	}
      tm=tm/mulres;
      if (clufl)
	if (fscanf(fpcl,"%i",&clu)<1)
	  {
	    break;
	  }
    }
}



check_minmax(int *clx)
{
  int i;
  static int cnonp=0,clxp=0;
  static char needfree=0;

  if (res<0)
    {
      *clx=-cx/res;
    } else *clx=cx;


  if (((tr->cnon)!=cnonp)||(cx!=clxp))
    {
      if (needfree)
	{
	  for(i=0;i<cnonp;i++) free(minx[i]);
	  free(minx);
	  for(i=0;i<cnonp;i++) free(maxx[i]);
	  free(maxx);
	}

      minx=(short**)calloc((tr->cnon),sizeof(short*));
      maxx=(short**)calloc((tr->cnon),sizeof(short*));
      if ((minx==NULL)||(maxx==NULL))
	{
	  printf("Error allociting memory");
	  exit(1);
	}
      for(i=0;i<(tr->cnon);i++)
	{
	  minx[i]=(short*)calloc(cx,sizeof(short));
	  maxx[i]=(short*)calloc(cx,sizeof(short));
	  if ((minx[i]==NULL)||(maxx[i]==NULL))
	    {
	      printf("Erro allociting memory");
	      exit(1);
	    }
	}
      needfree=1;
    }
  cnonp=(tr->cnon);
  clxp=cx;

}
startposition()
{
  long long ofs;
  if (buffp!=filepo)
    {
       ofs=sizeof(short)*chnoin*buffp;
       if (mfseeko(fpd,ofs,SEEK_SET)<0)
	 {
	   buffp=startpos;
	   ofs=sizeof(short)*chnoin*buffp;
	   if (mfseeko(fpd,ofs,SEEK_SET)<0)
	     {
	       printf("Error seeking data file startpos\n");
	       exit(1);
	     }
	 }
     }
}


void alloc_dbuff(int bnlen)
{
  static char nfree=0;
  static int buffln=0;
  if (buffln!=bnlen)
    {
      if (nfree) { free(dbuff);}
      dbuff=(short*)calloc(bnlen*chno,sizeof(short));
      buffln=bnlen;
      nfree=1;
    }
}



#define MAX_INTP 10000
void splin(float x[], float y[], int n,  float y2[])
{
	int i,k;
	float p,qn,sig,un,u[MAX_INTP];
	float yp1,ypn;



	yp1=(y[1]-y[0])/(x[1]-x[0]);
	ypn=(y[n-1]-y[n-2])/(x[n-1]-x[n-2]);

	if (yp1 > 0.99e30)
		y2[0]=u[0]=0.0;
	else {
		y2[0] = -0.5;
		u[0]=(3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
	}
	for (i=1;i<n-1;i++) {
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	if (ypn > 0.99e30)
		qn=un=0.0;
	else {
		qn=0.5;
		un=(3.0/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
	}
	y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0);
	for (k=n-2;k>=0;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
}



void spline(float xa[], float ya[], int n, float x[], float y[],int ni)
{
	int klo,khi,k,i;
	float h,b,a;
	float y2a[MAX_INTP];
	if (n>MAX_INTP) {printf("Too many items to intepolate\n");exit(1);}
	splin(xa,ya,n,y2a);
	for(i=0;i<ni;i++)
	  {
	    klo=0;
	    khi=n-1;
	    while (khi-klo > 1) {
	      k=(khi+klo) >> 1;
	      if (xa[k] > x[i]) khi=k;
	      else klo=k;
	    }
	    h=xa[khi]-xa[klo];
	    if (h == 0.0) {printf("Bad xa input to routine splint");exit(1);}
	    a=(xa[khi]-x[i])/h;
	    b=(x[i]-xa[klo])/h;
	    y[i]=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
	  }
}

interpolsub(short inbf[],int nread)
{

  /* extrenal input:
     n_colom_to_intp
     n_intp_incolom[n_colom_to_intp]
     intp_which_colom[n_colom_to_intp]
     intp_incolom[n_colom_to_intp][n_intp_incolom]
  */

  int ii,i,j,k,na,po;
  float xa[MAX_INTP],ya[MAX_INTP],xi[MAX_INTP],yi[MAX_INTP];


  for(ii=0;ii<nread;ii++)
    for(i=0;i<n_colom_to_intp;i++)
      {

	for(po=j=na=0;j<pr_nrow;j++)
	  {
	    if ((po>=n_intp_incolom[i])||(j<intp_incolom[i][po]))
	      {
		xa[na]=(float)j;
		ya[na++]=(float)inbf[ii*chnoin+pr_id[intp_which_colom[i]][j]];
	      } else xi[po++]=(float)j;

	  }

	spline(xa,ya,na,xi,yi,po);
	for(j=0;j<po;j++)
	  inbf[ii*chnoin+pr_id[intp_which_colom[i]][intp_incolom[i][j]]]=(short)yi[j];

      }
}


mread(short dbuff[],int nread,FILE *fp)
{
 static short *inbf;
 static char frefl=0;
 static int nreado=0;
 int ndread;
 int i,j,k,l,m;
 float szf;

 if (nread!=nreado)
   {
     if (frefl) free(inbf);
     inbf=(short*)calloc(nread*chnoin,sizeof(short));
     frefl=1;
     nreado=nread;
   }
 ndread=fread(inbf,sizeof(short),nread*chnoin,fpd);
 if (ndread>0)
   {
     for(i=0;i<nread*chnoin;i++) inbf[i]=bitn-inbf[i];

     if (intpfl) /* cubic spine intepolation */
       {
	 fprintf(stderr,"here\n");
	 interpolsub(inbf,nread);
       }
     if (prtype==1)
       {
	 for(i=0;i<nread;i++)
	   for(j=0;j<chnoin;j++)
	     dbuff[i*chnoin+j]=inbf[i*chnoin+j];

       }
     if  ((prtype==2)||(prtype==4))
       {
	 for(i=0;i<nread;i++)
	   for(j=0;j<pr_ncol;j++)
	     for(k=0;k<pr_nrow-pr_nwin+1;k++)
	       {
		 for(szf=0.,l=0;l<pr_nwin;l++)
		   szf+=(float)(inbf[i*chnoin+pr_id[j][k+l]])*pr_win[l];
		 dbuff[i*chno+j*(pr_nrow-pr_nwin+1)+k]=(short)szf;
	       }
       }
     if (prtype==3)
       {
	 for(i=0;i<nread;i++)
	   for(j=0;j<pr_ncol-pr_nwx+1;j++)
	     for(k=0;k<pr_nrow-pr_nwy+1;k++)
	       {
		 for(szf=0.,l=0;l<pr_nwx;l++)
		   for(m=0;m<pr_nwy;m++)
		     szf+=(float)(inbf[i*chnoin+pr_id[j+l][k+m]])*pr_wxy[l][m];
		 dbuff[i*chno+j*(pr_nrow-pr_nwy+1)+k]=(short)szf;
	       }
       }
   }
 if (extract_fl==1)
   {
     for(i=0;i<nread;i++)
       for(j=0;j<chno;j++)
	 dbuff[i*chno+j]=bitn-dbuff[i*chno+j];
     fwrite(dbuff,sizeof(short),ndread/chnoin*chno,fpextr);
     for(i=0;i<nread;i++)
       for(j=0;j<chno;j++)
	 dbuff[i*chno+j]=bitn-dbuff[i*chno+j];
   }else  if (extract_fl==2)
     {
       for(i=0;i<ndread/chnoin;i++,fprintf(fpextr,"\n"))
	 for(j=0;j<chno;j++)
	   fprintf(fpextr,"%i ",-dbuff[i*chno+j]);
     }
 return ndread;
}

void read_first_dbuff(int bnlen,int *dtread)
{
   long long ofs;
  *dtread=mread(dbuff,bnlen,fpd);
  if (*dtread<1) {

    buffp=startpos;
    ofs=sizeof(short)*chnoin*buffp;
    if (mfseeko(fpd,ofs,SEEK_SET)<0)
      {
	printf("Error seeking data file readfirst \n");
	exit(1);
      }

    *dtread=mread(dbuff,bnlen,fpd);
    if (*dtread<1) {
      printf("Error reading data file\n");
      exit(1);
    }
  }
}
#define MAX_BUFF 1000000

load_buff()
{
  int bnlen,dtread,bntoread;
  short ma[MAX_CH],mix[MAX_CH],dt;
  int i,j,k,ii;
  int clx;

  check_minmax(&clx);
  startposition();

  bnlen=sclen();
  if (bnlen<MAX_BUFF)
    {
      alloc_dbuff(bnlen);
      read_first_dbuff(bnlen,&dtread);
      startpos=buffp;
      filepo=mftello(fpd)/chnoin/sizeof(short);
      cxread=dtread/chnoin;
    }else
      {
	alloc_dbuff(MAX_BUFF);
	read_first_dbuff(MAX_BUFF,&dtread);
	startpos=buffp;
	filepo=mftello(fpd)/chnoin/sizeof(short);
	cxread=dtread/chnoin;
	bntoread=bnlen-cxread;
      }
      if (res<0)
	{

	  for(j=0,ii=0;(j<clx)&&(j<dtread/chnoin);j++,ii++)
	    {
	      for(i=0;i<(tr->cnon);i++)
		minx[i][ii]=maxx[i][ii]=dbuff[ii*chno+(tr->conpo)[i]];
	    }
	  cxread=j;
	}
      else
	{
	  ii=0;
	  for(j=0;j<cx;j++)
	    {
	      /*  if (feof(fpd)) break;*/

	      for(k=0;k<res;k++,ii++)
		{


		  if (ii==MAX_BUFF)
		    {
		      if (bntoread==0)
			{
			  printf("Program error\n");
			  exit(1);
			}
		      if (bntoread>MAX_BUFF)
			{
			  dtread=mread(dbuff,MAX_BUFF,fpd);
			  bntoread=bntoread-dtread/chnoin;
		      }else
			{
			  dtread=mread(dbuff,bntoread,fpd);
			  bntoread=bntoread-dtread/chnoin;
			}
		      filepo=mftello(fpd)/chnoin/sizeof(short);
		      cxread+=dtread/chnoin;
		      if (dtread<=0) goto out; /* file ended out of j cycle */
		      ii=0;
		    }
		  if (ii>=dtread/chnoin) goto out;
		  for(i=0;i<(tr->cnon);i++)
		    {
		      dt=dbuff[ii*chno+(tr->conpo)[i]];
		      if ((k==0)||(mix[i]>dt)) mix[i]=dt;
		      if ((k==0)||(ma[i]<dt)) ma[i]=dt;
		    }
		}
	      for(i=0;i<(tr->cnon);i++)
		{
		  minx[i][j]=mix[i];
		  maxx[i][j]=ma[i];
		  if (j>0)
		    {
		      if (minx[i][j]>maxx[i][j-1]) minx[i][j]=maxx[i][j-1];
		      if (maxx[i][j]<minx[i][j-1]) maxx[i][j]=minx[i][j-1];
		    }
		}

	    }
	out:;
	   cxread=j;
	}

  if (detfl)
    load_detect();

}



timestring(float tm,char st[])
{
  float tm1;
  if (tm<1000.)
    { sprintf(st,"%3.2fs-6",tm); return;}
  tm1=tm/1000;
  if (tm1<1000.)
    { sprintf(st,"%3.2fms",tm1);  return;}
  tm1=tm1/1000.;
  if (tm1<1000.)
    { sprintf(st,"%3.2fs",tm1);  return;}
  tm1=tm1/1000.;
  if (tm1<1000.)
    { sprintf(st,"%3.2fKs",tm1);  return;}
  tm1=tm1/1000.;
  if (tm1<1000.)
    { sprintf(st,"%3.2fMs",tm1);  return;}
  sprintf(st,"%3.0e2s",tm1);

}
#define NMGRID 5.
#define GRIP (cy/35) /* scale pos */
#define MGHI (cy/150)
#define SGHI (cy/300)
#define NSG 5.
#define TPOS (cy/200) /* grid character pos */
#define DIVB (cx/15)
#define ENDB (cx/18)

float getgridscale(float sctime)
{
  double rou,scale,tens,barref;

  barref=(double)sctime/10.*tr->nocol;
  tens=floor(log10(barref));
  rou=pow(10.,tens);
  if (barref/rou<2.5) scale=rou;
  else if (barref/rou<5.) scale=2.5*rou;
  else scale=5.*rou;

  return scale;
}


grid(int mwi, int sch)
{
  switch(gridtype){
    case 0: grid1(mwi,sch);
      break;
    case 1: grid2(mwi,sch);
  }
}

grid2(int mwi,int sch)
{
  float sctime,scaletm,barlen; /* in micro sec*/
  float bgrid,bgtm;
  char str[100],str1[100];
  int i,j,timofx;

  sctime=(float)sclen()*smrate;

  scaletm=getgridscale(sctime);
  barlen=(float)cx*(scaletm/sctime);


  bgrid=sctime/NMGRID;

  mlinewidth(2,mwi);


  timofx=(tr->nocol>1)?(cx*9/10*(tr->nocol-1)):cx*9/10*tr->nocol;
  mline(sch+timofx,cy-GRIP,sch+timofx+(int)barlen,cy-GRIP,mwi);
  mlinewidth(1,mwi);

  timestring(scaletm,str);

  mtext(sch+timofx,cy-TPOS,str,mwi);

  bgtm=(float)startpos*smrate;
  timestring(bgtm,str1);

  strcpy(str,dataflnm);
  strcat(str,"+");
  strcat(str,str1);
  mtext(sch,cy-TPOS,str,mwi);

  if (poimode)
    {
      strcpy(str,"POINT");
      mtext(10,cy-TPOS,str,mwi);
    }
}



grid1(int mwi,int sch)
{
  float sctime; /* in micro sec*/
  float bgrid,bgtm;
  char str[100];
  int i,j;

  sctime=(float)sclen()*smrate;
  bgrid=sctime/NMGRID;

  mlinewidth(2,mwi);

  mline(sch,cy-GRIP,sch+cx,cy-GRIP,mwi);
  for(i=0;i<=(int)NMGRID;i++)
    {
      mlinewidth(1,mwi);
      if (i<(int)NMGRID)
      for(j=1;j<(int)NSG;j++)
	mline(sch+i*cx/NMGRID+j*cx/NMGRID/NSG,cy-GRIP,
	      sch+i*cx/NMGRID+j*cx/NMGRID/NSG,cy-GRIP+SGHI,mwi);
      mlinewidth(2,mwi);
      mline(sch+i*cx/NMGRID,cy-GRIP,sch+i*cx/NMGRID,cy-GRIP+MGHI,mwi);
    }
  mlinewidth(1,mwi);
  bgtm=(float)startpos*smrate;
  timestring(bgtm,str);
  strcat(str," ");
  strcat(str,dataflnm);
  mtext(sch,cy-TPOS,str,mwi);
  timestring(bgrid,str);
  strcat(str,"/div");
  mtext(sch+cx/2-DIVB,cy-TPOS,str,mwi);

  timestring(bgtm+sctime,str);
  mtext(sch+cx-ENDB,cy-TPOS,str,mwi);
  if (poimode)
    {
      strcpy(str,"POINT");
      mtext(10,cy-TPOS,str,mwi);
    }
}
point_texts(int mwi,int sch,char colfl)
{
  int i;
  long k,k1;
  char st[100],str[100];
  float tm;
  mlinewidth(2,mwi);
  for(i=0;i<(tr->cnon);i++)
    {
      if (tr->chposx[i]/cx==idchan)
	{
	  if (colfl)
	    msetcolor((tr->chcol)[i],mwi);
	  else msetcolor(BACKGRCOLOR,mwi);
	  sprintf(st,"%5i:%2i",(maxx[i][poipos]+minx[i][poipos])/2,(tr->conpo)[i]);
	  mtext(2,(tr->chposy)[i]+5,st,mwi);
	  mline(sch-10,(tr->chposy)[i],sch,(tr->chposy)[i],mwi);
	}
    }
  msetcolor(FOREGRCOLOR,mwi);
  mlinewidth(1,mwi);
  if (colfl)
    msetcolor((tr->chcol)[i],mwi);
  else msetcolor(BACKGRCOLOR,mwi);

  if (res>0) k=startpos+poipos*res; else k=startpos+poipos;
  if (res>0) k1=poipos*res; else k1=poipos;
  sprintf(st,"col:%2i pos:%10li ",idchan+1,k);
  strcat(st,"Time:");
  tm=(float)startpos*smrate;
  timestring(tm,str);
  strcat(st,str);
  strcat(st," + ");
  tm=(float)k1*smrate;
  timestring(tm,str);
  strcat(st,str);
  mtext(2,15,st,mwi);
}
scr_texts(int mwi,int sch)
{
  int i;
  char st[100];
  mlinewidth(2,mwi);
  if (tr->nocol<=idchan) idchan=0;
  for(i=0;i<(tr->cnon);i++)
    {
      if (tr->chposx[i]/cx==idchan)
	{
	  msetcolor((tr->chcol)[i],mwi);
	  sprintf(st,"%4.6fX:%2i",(tr->muly)[i],(tr->conpo)[i]);
	  mtext(2,(tr->chposy)[i]+5,st,mwi);
	  mline(sch-10,(tr->chposy)[i],sch,(tr->chposy)[i],mwi);
	}
    }
  msetcolor(FOREGRCOLOR,mwi);
  mlinewidth(1,mwi);
  sprintf(st,"col:%2i",idchan+1);
  mtext(2,10,st,mwi);
}
draw_channel(int chan,int mwi,int sch)
{
  int j,clx;
  if (res<0)
      {
	clx=cxread;
	for(j=0;j<clx-1;j++)
	  mline(-j*res+sch+(tr->chposx)[chan]*(cgap+cx)/cx,
		(tr->chposy)[chan]+minx[chan][j]*(tr->muly)[chan],
		-(j+1)*res+sch+(tr->chposx)[chan]*(cgap+cx)/cx,
		(tr->chposy)[chan]+minx[chan][j+1]*(tr->muly)[chan],mwi);
    }else{
      for(j=0;j<cxread;j++)
	mline(j+sch+(tr->chposx)[chan]*(cgap+cx)/cx,
	      (tr->chposy)[chan]+minx[chan][j]*(tr->muly)[chan],
	      j+sch+(tr->chposx)[chan]*(cgap+cx)/cx,
	      (tr->chposy)[chan]+maxx[chan][j]*(tr->muly)[chan],mwi);
    }
}


draw_whl(int mwi,int sch)
{
  int whlfr,whlto,i,schh;
  long int stt,endt;
  float prx,pry;
  int x1,x2,y1,y2;
  stt=startpos;
  endt=stt+sclen();
  whlfr=stt*mulres/16/32; /*assuming 20kHz*/
  whlto=endt*mulres/16/32; /*assuming 20kHz*/

  schh=sch+(tr->nocol-1)*cx;

  msetcolor(4,mwi);
  prx=-1;pry=-1;
  for(i=0;i<whllen;i++)
    {
      x1=prx*cx/5/200;
      x2=whlx[i]*cx/5/200;
      y1=pry*cy/4/200;
      y2=whly[i]*cy/4/200;
      if ((prx>-1.)&&(pry>-1.)&&(whlx[i]>-1.)&&(whly[i]>-1.))
	 mline(schh+x1,cy-y1,schh+x2,cy-y2,mwi);

      if ((whlx[i]>-1.)&&(whly[i]>-1.))
	{
	  prx=whlx[i];pry=whly[i];
	}
    }

msetcolor(3,mwi);
  prx=-1;pry=-1;
  for(i=whlfr;i<whlto;i++)
    {
      x1=prx*cx/5/200;
      x2=whlx[i]*cx/5/200;
      y1=pry*cy/4/200;
      y2=whly[i]*cy/4/200;
      if ((prx>-1.)&&(pry>-1.)&&(whlx[i]>-1.)&&(whly[i]>-1.))
	 mline(schh+x1,cy-y1,schh+x2,cy-y2,mwi);

      if ((whlx[i]>-1.)&&(whly[i]>-1.))
	{
	  prx=whlx[i];pry=whly[i];
	}
    }

}

draw_detect(int mwi,int sch)
{
  int i,ii;
  char st[100];

  if (rasfl) { draw_raster(mwi,sch); return;}

  for(i=0;i<cx;i++)
    {
      if (dtbuf[i]>0)
	{
	  if (clufl)   msetcolor(palette[dtbuf[i]],mwi);
	  else msetcolor(DETECT_COLOR,mwi);

	  mline(sch+i,12,sch+i,cy,mwi);
	  for(ii=0;ii<tr->nocol;ii++)
	    {
	      if (res>0)
		mline(i+sch+(cx+cgap)*ii,0,i+sch+(cx+cgap)*ii,cy-GRIP,mwi);
	      else   mline(-res*i+sch+(cx+cgap)*ii,0,-res*i+sch+(cx+cgap)*ii,cy-GRIP,mwi);
	      if (clufl)
		{
		  sprintf(st,"%i",dtbuf[i]);
		  if (res>0)
		    mtext(sch+i-5+(cx+cgap)*ii,10,st,mwi);
		  else mtext(sch-i*res-5+(cx+cgap)*ii,10,st,mwi);
		}
	    }


	}
    }
}

init_rapos()
{
int i,rawig;
 ralifl=1;
 rawig=(raend-rastart)/(clon);
 rawi=4*rawig/5;
 raskip=rawig-rawi;
 for(i=1;i<=clon;i++)
     rapos[i]=rastart+(i-1)*(rawi+raskip);
}
init_raster()
{
 int i;
 rastart=cy/2;
 raend=cy-2*GRIP;
 clon=clno-1;

 raon[1]=0;
 raon[0]=0;
 for(i=2;i<=clno;i++)
   {
     raon[i]=1;
     raord[i]=i-1;
   }
 init_rapos();
}
draw_raster(int mwi,int sch)
{
  int i,j;
  char st[100];
  for(j=1;j<=clno;j++)
    if (raon[j])
    {
      msetcolor(palette[raord[j]+1],mwi);
      sprintf(st,"%i",j);
      mtext(sch-10,rapos[raord[j]]+rawi,st,mwi);
      if (ralifl)
	{
	  msetcolor(DETECT_COLOR,mwi);
	  mline(sch-10,rapos[raord[j]]+rawi+raskip/2,cx+sch,rapos[raord[j]]+rawi+raskip/2,mwi);
	}
    }
  for(j=1;j<=clno;j++)
  for(i=0;i<cx;i++)
    {
      if (rasbuf[j][i]>0)
	{
	  msetcolor(palette[raord[j]+1],mwi);
	  mline(sch+i,rapos[raord[j]],sch+i,rapos[raord[j]]+rawi,mwi);
	}
    }
}


custom_text(int mwi,int sch)
{
  int i;

  for(i=0;i<stxno;i++)
    {
      msetcolor(stx[i].col,mwi);
      mtext(sch+stx[i].x,stx[i].y,stx[i].st,mwi);
    }

}

#define YAXISCOLOR 31


pointline(int pos,int sch,int mwi)
{
  int i;
  for(i=0;i<tr->nocol;i++)
    {
      if (res>0)
	mline(pos+sch+(cx+cgap)*i,0,pos+sch+(cx+cgap)*i,cy-GRIP,mwi);
      else   mline(-res*pos+sch+(cx+cgap)*i,0,-res*pos+sch+(cx+cgap)*i,cy-GRIP,mwi);
    }
}


draw_traces(int mwi,int sch)
{
  int i;

  if (!print_only)
    clearwindow(mwi);

  if (detfl)
    draw_detect(mwi,scrhead);

  if (whlfl)
    draw_whl(mwi,scrhead);


  for(i=0;i<(tr->cnon);i++)
    {
      msetcolor((tr->chcol)[i],mwi);
      draw_channel(i,mwi,scrhead);

    }

  if (yaxison)
    {
      msetcolor(YAXISCOLOR,mwi);
      pointline(yaxispos,scrhead,mwi);
    }
  msetcolor(FOREGRCOLOR,mwi);

  if (!prfl)
    scr_texts(mwi,scrhead);

  custom_text(mwi,scrhead);
  msetcolor(FOREGRCOLOR,mwi);
  grid(mwi,scrhead);

}

testskip(char ch)
{
  if ((ch>'9')||(ch<'1')) return;
  skipv=ch-'1';
}


int skipoffset()
{
  int j,i;
  j=1;
  for(i=0;i<skipv/2;i++)
    j=j*10;
  if ((skipv%2)==1) j=j*5;
  return j;
}

add_chan(int ch,int xx,int yy)
{
  int i,j,k;

  for(i=(tr->cnon)-1;i>=-1;i--)
    {
      if (((tr->conpo)[i]>ch)&&(i>=0))
	{
	  (tr->conpo)[i+1]=(tr->conpo)[i];
	  (tr->chcol)[i+1]=(tr->chcol)[i];
	  (tr->chposy)[i+1]=(tr->chposy)[i];
	  (tr->chposx)[i+1]=(tr->chposx)[i];
	  (tr->muly)[i+1]=(tr->muly)[i];
	}else {
	  (tr->conpo)[i+1]=ch;
	  (tr->chcol)[i+1]=FOREGRCOLOR;
	  (tr->chposy)[i+1]=yy;
	  (tr->chposx)[i+1]=((xx-scrhead)/(cx+cgap))*cx;
	  if (i>=0)
	  (tr->muly)[i+1]=(tr->muly)[i];
	  else (tr->muly)[i+1]=(tr->muly)[i+2];
	  break;
	}
    }
  chon[ch]++;
  (tr->cnon)++;
  load_buff();
}





add_text(int x,int y,int mwi,int sch)
{

  if (stxno>=MAX_TEXT) return;

  getstring(ASK_X,ASK_Y,ASK_HEI,"Text ask","Text to display? ",
		       stx[stxno].st,lac,lav,mwi+1);
  if (strlen(stx[stxno].st)>0)
    {
      stx[stxno].x=x-sch;
      stx[stxno].y=y;
      stx[stxno].col=3;
      stxno++;

    }
}



copy_text(int x,int y,int selch,int sch)
{

  if (stxno>=MAX_TEXT) return;
  stx[stxno].x=x-sch;
  stx[stxno].y=y;
  stx[stxno].col=stx[selch].col;
  strcpy(stx[stxno].st,stx[selch].st);
  stxno++;


}


addchan(int xx,int yy,int sch,int mwi,int mkey)
{
  char *selel[MAX_CH];
  char mask[MAX_CH];
  int i,j,k;
  int cx,cy,cdisp,rpos;
  cx=12;
  cdisp=chno+2;
  if ((cdisp%cx)==0) cy=cdisp/cx;
  else cy=cdisp/cx+1;

  if (cdisp<cx) {cx=cdisp;cy=1;}
  for(i=0;i<cx*cy;i++) selel[i]=(char*)malloc(10);
  for(i=0;i<cx*cy;i++)
    if (i<chno) {
      sprintf(selel[i],"%i",i);
      mask[i]=(chon[i]>0)?1:0;
    }
    else {
      selel[i][0]=0;
      mask[i]=0;
    }
  strcpy(selel[cx*cy-2],"Text");
  strcpy(selel[cx*cy-1],"Cancel");
  select_out(mwi+1,cx,cy,mask,selel,"Channels",lac,lav);
  while(1)
    {
      select_wait(mwi+1,&i,&j);
      rpos=j*cx+i;
      if (rpos>=cx*cy-2) break;
      if (rpos>=chno) continue;
      { add_chan(rpos,xx,yy); break;}
    }
  redrfl=1;
  close_window(mwi+1);
  if (rpos==cx*cy-2) add_text(xx,yy,mwi,sch);

}



inc_color(int *a)
{

  int po;
  po=lookup_pale(*a)+1;
  if (po>=MAXCOLOR) po=0;
  *a=pale[po];

}



int marktext(int xx,int yy,int sch, int mkey)
{
  int i,xxr,midx,dx,dy;



  midx=-1;
  xxr=xx-sch;
  dx=(cx*(tr->nocol))/100.;
  dy=cy/100; /* in what distance to select */
  for(i=0;i<stxno;i++)
    {
      if ((stx[i].x+dx<xxr)||(stx[i].x-dx>xxr)) continue;
      if ((stx[i].y+dy<yy)||(stx[i].y-dy>yy)) continue;
      midx=i;break;
    }
  if (midx==-1) return midx;
  if (mkey==1)
    {
      redrfl=1;
      selfl=1;
      selflch=1;
      selch=midx;
      skipv=0;
    } else if (mkey==2)
      {
	inc_color(&(stx[midx].col));
	redrfl=1;
      }
}


markchan(int xx,int yy,int sch,int mwi,int mkey)
{
  int i,dis,mdis,midx,xxre,xxr,cxre,firfl;

  if (xx<sch)
    {
      idchan++;
      if (idchan>=tr->nocol) idchan=0;
      redrfl=1;
      return;
    }
  if (marktext(xx,yy,sch,mkey)!=-1) return; /* character was selected */
  midx=-1;
  xxr=xx-sch;
  cxre=(res<0)?-cx/res:cx;

  for(i=0,firfl=1;i<(tr->cnon);i++)
    {
      if ((xxr<(tr->chposx)[i]*(cgap+cx)/cx)||(xxr>(tr->chposx)[i]*(cgap+cx)/cx+cxre)) continue;
      xxre=xx-sch-(tr->chposx)[i]*(cgap+cx)/cx;
      dis=(maxx[i][xxre]+minx[i][xxre])/2*(tr->muly)[i]+(tr->chposy)[i]-yy;
      if (dis<0) dis=-dis;
      if ((firfl)||(mdis>dis))
	{
	  mdis=dis;
	  midx=i;
	  firfl=0;
	}
    }
  if (midx==-1) return;
  if (mkey==1)
    {
      redrfl=1;
      selfl=1;
      selch=midx;
      skipv=0;
    } else if (mkey==2)
      {
	inc_color(&(tr->chcol)[midx]);
	if ((tr->chcol)[midx]==BACKGRCOLOR)  inc_color(&(tr->chcol)[midx]);
	redrfl=1;
      }
}
off_chan(int sch)
{
  int i;

  if ((chon[(tr->conpo)[sch]]<=0)||((tr->cnon)==1)) return;
  chon[(tr->conpo)[sch]]--;
  for(i=sch;i<(tr->cnon)-1;i++)
    {
      (tr->chposy)[i]=(tr->chposy)[i+1];
      (tr->chposx)[i]=(tr->chposx)[i+1];
      (tr->conpo)[i]=(tr->conpo)[i+1];
      (tr->muly)[i]=(tr->muly)[i+1];
      (tr->chcol)[i]=(tr->chcol)[i+1];
    }
  (tr->cnon)--;
  load_buff();
}

off_text(int sch)
{
  int i;

  if (sch>=stxno) return;
  stxno--;
  for(i=sch;i<stxno;i++)
    {
      stx[i].x=stx[i+1].x;
      stx[i].y=stx[i+1].y;
      stx[i].col=stx[i+1].col;
      strcpy(stx[i].st,stx[i+1].st);
    }
}


float inc_mul(float m)
{
  float m1;
  m1=m;
  if (fabs(m-.5)<0.0001) m1=1.;
  if (fabs(m-.2)<0.0001) m1=.5;
  if (fabs(m-.1)<0.0001) m1=.2;
  if (fabs(m-.05)<0.0001) m1=.1;
  if (fabs(m-.02)<0.0001) m1=.05;
  if (fabs(m-.01)<0.0001) m1=.02;
  if (fabs(m-.005)<0.0001) m1=.01;
  if (fabs(m-.002)<0.0001) m1=.005;
  if (fabs(m-.001)<0.0001) m1=.002;
  return m1;
}

float dec_mul(float m)
{
  float m1;
  m1=m;
  if (fabs(m-1.)<0.0001) m1=.5;
  if (fabs(m-.5)<0.0001) m1=.2;
  if (fabs(m-.2)<0.0001) m1=.1;
  if (fabs(m-.1)<0.0001) m1=.05;
  if (fabs(m-.05)<0.0001) m1=.02;
  if (fabs(m-.02)<0.0001) m1=.01;
  if (fabs(m-.01)<0.0001) m1=.005;
  if (fabs(m-.005)<0.0001) m1=.002;
  if (fabs(m-.002)<0.0001) m1=.001;
  return m1;
}

restoretraces(int pos,int sch,int mwi)
{
  int i,j,k;
  for(i=0;i<(tr->cnon);i++)
    {
      msetcolor((tr->chcol)[i],mwi);
      if (res<0)
	{
	  if (pos+1<cx)
	  mline(-pos*res+sch+(tr->chposx)[i]*(cgap+cx)/cx,
		(tr->chposy)[i]+minx[i][pos]*(tr->muly)[i],
		-(pos+1)*res+sch+(tr->chposx)[i]*(cgap+cx)/cx,
		(tr->chposy)[i]+minx[i][pos+1]*(tr->muly)[i],mwi);
	  if (pos-1>=0)
	  mline(-pos*res+sch+(tr->chposx)[i]*(cgap+cx)/cx,
		(tr->chposy)[i]+minx[i][pos]*(tr->muly)[i],
		-(pos-1)*res+sch+(tr->chposx)[i]*(cgap+cx)/cx,
		(tr->chposy)[i]+minx[i][pos-1]*(tr->muly)[i],mwi);
      }else{
	mline(pos+sch+(tr->chposx)[i]*(cgap+cx)/cx,
	      (tr->chposy)[i]+minx[i][pos]*(tr->muly)[i],
	      pos+sch+(tr->chposx)[i]*(cgap+cx)/cx,
	      (tr->chposy)[i]+maxx[i][pos]*(tr->muly)[i],mwi);
    }
    }
  msetcolor(FOREGRCOLOR,mwi);
  grid(mwi,sch);
}



raster_width(int eve,int mwi,int scrhead)
{
  char ch;
  int che,i,xx,yy,mkey,xxconv,xsize;
  long int k;
  static int pose=0;

  xsize=(res<0)?-(cx/res):cx;
  if ((eve==EXPOSE)||(redrfl))
    {
      clearwindow(mwi);
      for(i=0;i<(tr->cnon);i++)
	{
	  msetcolor((tr->chcol)[i],mwi);
	  draw_channel(i,mwi,scrhead);
	  msetcolor(FOREGRCOLOR,mwi);
	  mline(scrhead,rastart,cx,rastart,mwi);
	  msetcolor(FOREGRCOLOR+1,mwi);
	  mline(scrhead,raend,cx,raend,mwi);
	}
      grid(mwi,scrhead);
      redrfl=0;
    }

  if (eve==MOUPR)
    {
      mousecoord(&xx,&yy,&mkey,mwi);
      if (mkey==1)
	{
	  msetcolor(BACKGRCOLOR,mwi);
	  mline(scrhead,rastart,cx,rastart,mwi);
	  msetcolor(FOREGRCOLOR,mwi);
	  mline(scrhead,yy,cx,yy,mwi);
	  rastart=yy;
	}
      if (mkey==2) goto exit;
      if (mkey==4)
	{
	  msetcolor(BACKGRCOLOR,mwi);
	  mline(scrhead,raend,cx,raend,mwi);
	  msetcolor(FOREGRCOLOR+1,mwi);
	  mline(scrhead,yy,cx,yy,mwi);
	  raend=yy;
	}
    }
  if(eve==KEYPR)
    {
      getkey(&ch);
      che=get_extra_keys();
      if (che==Escape_Key)
	{
	  goto exit;
	}
    }
  return;
exit:
  init_rapos();
  rawifl=0;
  redrfl=1;

}
select_poi(int eve,int mwi,int scrhead)
{
  char ch;
  int che,i,xx,yy,mkey,xxconv,xsize;
  long int k;
  static int pose=0;

  yaxison=0;
  xsize=(res<0)?-(cx/res):cx;
  if ((eve==EXPOSE)||(redrfl))
    {
      clearwindow(mwi);
      for(i=0;i<(tr->cnon);i++)
	{
	  msetcolor((tr->chcol)[i],mwi);
	  draw_channel(i,mwi,scrhead);
	}
      pointline(poipos,scrhead,mwi);
      point_texts(mwi,scrhead,1);
      grid(mwi,scrhead);
      redrfl=0;
      if (pose>xsize) pose=0;
    }

   point_texts(mwi,scrhead,0);
  if (eve==MOUPR)
    {
      mousecoord(&xx,&yy,&mkey,mwi);
      if (xx<scrhead)
	{
	  idchan++;
	  if (idchan>=tr->nocol) idchan=0;
	}else
	  {
	    poipos=xx-scrhead;
	    if (res<0) poipos=-poipos/res;
	    while(poipos>cx) poipos=poipos-cgap-cx;
	  }
    }
  if(eve==KEYPR)
    {
      getkey(&ch);
      che=get_extra_keys();
      testskip(ch);
      if (che==Left_Key)
	{
	  poipos-=(skipv+1);
	}
      if (che==Right_Key)
	{
	  poipos+=skipv+1;
	}
      if (che==Escape_Key)
	{
	  poifl=0;
	  redrfl=1;
	}
      if (ch=='p')
	{
	  if (!pofl)
	    {
	      getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Pointer file?  ",
			   poiflnm,lac,lav,mwi+1);
	      open_poifile(mwi);
	    }
	  if (res>0) k=startpos+poipos*res; else k=startpos+poipos;
	  fprintf(poifp,"%li\n",k);
	   poifl=0;
	  redrfl=1;
	}
      if (ch=='c')
	{
	  if (res>0)
	  buffp+=(poipos-xsize/2)*res;
	  else buffp+=(poipos-xsize/2);
	  poifl=0;
	  load_buff();
	  redrfl=1;
	}
      if (ch=='s')
	{
	  if (res>0)
	  buffp+=(poipos-xsize/2)*res;
	  else buffp+=(poipos-xsize/2);
	  poifl=0;
	  swapchan();
	  load_buff();
	  redrfl=1;
	}
      if (ch=='l')
	{
	  if (res>0)
	  buffp+=(poipos)*res;
	  else buffp+=poipos;
	  poifl=0;
	  load_buff();
	  redrfl=1;
	}
      if (ch=='r')
	{
	  if (res>0)
	  buffp+=-(xsize-poipos)*res;
	  else buffp+=-(xsize-poipos);
	  poifl=0;
	  load_buff();
	  redrfl=1;
	}
      if (ch=='x')
	{
	  yaxison=1;/* leave pointer axis on he screen */
	  poifl=0;
	  yaxispos=poipos;
	  redrfl=1;
	}

    }
  poipos=poipos%xsize;
  if ((poipos<0)) poipos=pose;
  point_texts(mwi,scrhead,1);
  if (poipos!=pose)
	{
	  msetcolor(BACKGRCOLOR,mwi);
	  pointline(pose,scrhead,mwi);
	  restoretraces(pose,scrhead,mwi);
	  msetcolor(FOREGRCOLOR,mwi);
	  pointline(poipos,scrhead,mwi);
	  pose=poipos;
	}

}
select_sel(int eve,int mwi,int scrhead)
{
  char ch;
  int che,i,xx,yy,mkey,xxconv,nchpox;
  if ((eve==EXPOSE)||(redrfl))
    {
      clearwindow(mwi);
      if (detfl)
	draw_detect(mwi,scrhead);
      for(i=0;i<(tr->cnon);i++)
	{
	  if ((i==selch)&&(selflch==0))
	  msetcolor(MARKCOLOR,mwi);
	  else msetcolor((tr->chcol)[i],mwi);
	  draw_channel(i,mwi,scrhead);
	  redrfl=0;
	}

      for(i=0;i<stxno;i++)
	{
	  if ((i==selch)&&(selflch==1))
	  msetcolor(MARKCOLOR,mwi);
	  else  msetcolor(stx[i].col,mwi);
	  mtext(scrhead+stx[i].x,stx[i].y,stx[i].st,mwi);
	}



      msetcolor(FOREGRCOLOR,mwi);
      scr_texts(mwi,scrhead);
      grid(mwi,scrhead);
    }
  if(eve==KEYPR)
    {
      getkey(&ch);
      che=get_extra_keys();
      testskip(ch);
      if (selflch==0)
	{
	  if (ch=='i')
	    {
	      (tr->muly)[selch]=inc_mul((tr->muly)[selch]);
	      redrfl=1;
	    }
	  if (ch=='d')
	    {
	      (tr->muly)[selch]=dec_mul((tr->muly)[selch]);
	      redrfl=1;
	    }
	  if ((ch>='1')&&(ch<='9'))
	    {
	      (tr->chcol)[selch]=pale[(int)(ch)-(int)'1'];
	      redrfl=1;
	      selfl=0;
	    }
	  if (che==Up_Key)
	    {
	      (tr->chposy)[selch]-=(skipv-1);
	      redrfl=1;
	    }

	  if (che==Down_Key)
	    {
	      (tr->chposy)[selch]+=skipv+1;
	      redrfl=1;
	    }
	}else {
	  if ((ch>='1')&&(ch<='9'))
	    {
	      stx[selch].col=pale[(int)(ch)-(int)'1'];
	      redrfl=1;
	      selfl=0;
	      selflch=0;
	    }

	}
      if (che==Escape_Key)
	{
	  selfl=0;
	  selflch=0;
	  redrfl=1;
	}
    }
  if (eve==MOUPR)
    {
      mousecoord(&xx,&yy,&mkey,mwi);
      if (mkey==1)
	{
	  if (selflch==0)
	    {
	      nchpox=((xx-scrhead)/(cx+cgap))*(cx);
	      if (nchpox!=(tr->chposx)[selch])
		(tr->chposx)[selch]=nchpox;
	      else {
		xxconv=xx-scrhead-(tr->chposx)[selch]/cx*(cx+cgap);
		if (xxconv<0) xxconv=0;
		if (xxconv>=cx) xxconv=cx-1;
		(tr->chposy)[selch]=yy-(tr->muly)[selch]*(minx[selch][xxconv]+
							  maxx[selch][xxconv])/2.;
	      }
	    }else {
	      stx[selch].x=xx-scrhead;
	      stx[selch].y=yy;
	      selflch=0;
	    }
	  redrfl=1;
	  selfl=0;
	}

      if (mkey==2) {
	if (selflch==0)
	  {
	    off_chan(selch);
	  }else {
	    off_text(selch);
	    selflch=0;
	  }
	selfl=0;
	redrfl=1;
      }
      if ((mkey==4)&&(selflch)){

	copy_text(xx,yy,selch,scrhead);
	selflch=0;
	selfl=0;
	redrfl=1;
      }
    }
}



open_whlfile(int mwi)
{
  FILE *fpwhl;
  float x,y;
  int i;
  if (strlen(whl_name)!=0)
    {
      fpwhl=fopen(whl_name,"r");
      if (fpwhl!=NULL)
	{
	  if (whlfl)
	    { free(whlx);free(whly);}
	  whlfl=1;
	  whllen=0;
	  while(!feof(fpwhl))
	    {
	      if (fscanf(fpwhl,"%f %f",&x,&y)!=2) break;
	      whllen++;
	    }

	  rewind(fpwhl);
	  whlx=(float*)calloc(whllen,sizeof(float));
	  whly=(float*)calloc(whllen,sizeof(float));

	  for(i=0;i<whllen;i++)
	    if (fscanf(fpwhl,"%f %f",&whlx[i],&whly[i])!=2) break;
	  fclose(fpwhl);
      } else
	{
	  if (xwindow_open)
	  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
		      "File does not exsist! ... press  any key",lac,lav,mwi+1);
	  else {
	    printf("Cannot open whl file\n");
	    exit(1);
	  }
	}
    }
}





open_detfile(int mwi)
{
  if (strlen(det_name)!=0)
    {
      fpdt=fopen(det_name,"r");
      if (fpdt!=NULL)
	{
	  detfl=1;
      } else
	{
	  if (xwindow_open)
	  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
		      "File does not exsist! ... press  any key",lac,lav,mwi+1);
	  else {
	    printf("Cannot open detection file\n");
	    exit(1);
	  }
	}
    }
}

open_clufile(int mwi)
{
  if (strlen(clu_name)!=0)
    {
      fpcl=fopen(clu_name,"r");
      if (fpcl!=NULL)
	{
	  clufl=1;
	  fscanf(fpcl,"%i",&clno);
      } else
	{
	  if (xwindow_open)
	  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
		      "File does not exsist! ... press  any key",lac,lav,mwi+1);
	  else {
	    printf("Cannot open detection file\n");
	    exit(1);
	  }
	}
    }
}

open_bmfile(int mwi)
{
  if (strlen(bmflnm)!=0)
    {
      bmfp=fopen(bmflnm,"r");
      if (bmfp!=NULL)
	{
	  bmfl=1;
      } else
	{
	  if (xwindow_open)
	  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
		      "File does not exsist! ... press  any key",lac,lav,mwi+1);
	  else {
	    printf("Cannot open bookmark file\n");
	    exit(1);
	  }
	}
    }
}

open_poifile(int mwi)
{
  if (strlen(poiflnm)!=0)
    {
      poifp=fopen(poiflnm,"w");
      if (poifp!=NULL)
	{
	  pofl=1;
      } else
	{
	  if (xwindow_open)
	  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
		      "File does not exsist! ... press  any key",lac,lav,mwi+1);
	  else {
	    printf("Cannot open pointer file\n");
	    exit(1);
	  }
	}
    }
}

arr_scale()
{
  float muy,mulx,mulpoi;
  int i;
  if (prfl)
    {
      muy=cy/(float)cyo;
      mulx=(cx+cgap)/(float)(cxo+cgapo);
      mulpoi=(cx)/(float)(cxo);
      scrheado=scrhead;
      scrhead=0;
    }
  else
    {
      muy=cyo/(float)cy;
      mulpoi=(cxo)/(float)(cx);
      mulx=(cxo+cgapo)/(float)(cx+cgap);
      scrhead=scrheado;
    }


  for(i=0;i<(tr->cnon);i++)
    {
      (tr->chposy)[i]=(float)(tr->chposy)[i]*muy;
      (tr->chposx)[i]=(float)(tr->chposx)[i]*mulx;
      (tr->muly)[i]=(float)(tr->muly)[i]*muy;
    }

  for(i=0;i<stxno;i++)
    {
      stx[i].x=(float)(stx[i].x*mulx);
      stx[i].y=(float)(stx[i].y*muy);
    }


  if (yaxison) {
    if (res>0)
    yaxispos=(float)(yaxispos*mulpoi);
    else yaxispos=-(float)(yaxispos*mulpoi)/res;
  }

  for(i=1;i<=clon;i++)
    rapos[i]=(float)rapos[i]*muy;
  rawi=(float)rawi*muy;
  raskip=(float)raskip*muy;
}

addnewcol(){
int i,ocx;
ocx=cx;
cx=(cx*tr->nocol-cgap)/(tr->nocol+1);
for(i=0;i<(tr->cnon);i++)
  {
    (tr->chposx)[i]=(tr->chposx)[i]/ocx*cx;
  }
(tr->nocol)++;;
}



addquarternewcol(){
int i,ocx;
ocx=cx;
cx=(cx*tr->nocol-cgap)/(tr->nocol+1)*3/2;
for(i=0;i<(tr->cnon);i++)
  {
    (tr->chposx)[i]=(tr->chposx)[i]/ocx*cx;
  }
(tr->nocol)++;;
}


swapchan()
{
 int i,no;
 if (expfl)
   {
     cx=(cx-(tr1.nocol-1)*cgap)/tr1.nocol;
     for(i=no=0;i<tr1.cnon;i++)
       {
	 if ((tr1.chposx[i])/cx!=expch)
	   {
	     tr1.chposx[no]=tr1.chposx[i];
	     tr1.chposy[no]=tr1.chposy[i];
	     tr1.muly[no]=tr1.muly[i];
	     tr1.chcol[no]=tr1.chcol[i];
	     tr1.conpo[no]=tr1.conpo[i];
	     no++;
	   }
       }
     for(i=0;i<tr2.cnon;i++)
       {
	     tr1.chposx[no]=expch*cx;
	     tr1.chposy[no]=tr2.chposy[i];
	     tr1.muly[no]=tr2.muly[i];
	     tr1.chcol[no]=tr2.chcol[i];
	     tr1.conpo[no]=tr2.conpo[i];
	     no++;

       }
     tr1.cnon=no;
     tr=&tr1;
     expfl=0;
   }
 else
   {
     expch=idchan;
     for(i=no=0;i<tr1.cnon;i++)
       {
	 if ((tr1.chposx[i])/cx==expch)
	   {
	     tr2.chposx[no]=0;
	     tr2.chposy[no]=tr1.chposy[i];
	     tr2.muly[no]=tr1.muly[i];
	     tr2.chcol[no]=tr1.chcol[i];
	     tr2.conpo[no]=tr1.conpo[i];
	     no++;
	   }
       }
     tr2.nocol=1;
     tr2.cnon=no;
     cx=cx*tr1.nocol+cgap*(tr1.nocol-1);
     tr=&tr2;
     expfl=1;
   }

}
select_main(int eve,int mwi,int scrhead,FILE *fpc)
{
  char ch=0,str[100],scc;
  int che,res1,i;
  int xx,yy,mkey,mulf;
  int cxold,cyold;
  long int newpos;

  if (eve==CHCONF)
    {
      cxold=cx;
      cyold=cy;
      getwinsize(&cx,&cy,mwi);
      cx=(cx-scrhead-(tr->nocol-1)*cgap)/(tr->nocol);
      for(i=0;i<(tr->cnon);i++)
	{
	(tr->chposx)[i]=(tr->chposx)[i]*cx/cxold;
	(tr->chposy)[i]=(tr->chposy)[i]*cy/cyold;
	}

      for(i=0;i<stxno;i++)
	{
	  stx[i].x=stx[i].x*cx/cxold;
	  stx[i].y=stx[i].y*cy/cyold;
	}

      redrfl=1;
      load_buff();
    }
  if ((eve==EXPOSE)||(redrfl))
    {
      draw_traces(mwi,scrhead);
      redrfl=0;
    }
  if(eve==KEYPR)
	{
	  if (print_only)
	    {
	      che=0;
	      ch='P';
	    }else
	      {
		getkey(&ch);
		che=get_extra_keys();
	      }

	 testskip(ch);
	 if ((ch==32)||(che==Right_Key))
	   {
	     buffp+=sclen()*skipoffset();
	     load_buff();
	     redrfl=1;
	   }
	 if (che==Left_Key)
	   {
	     buffp-=sclen()*skipoffset();
	     load_buff();
	     redrfl=1;
	   }
	 if (che==Home_Key)
	   {
	     buffp=0;
	     load_buff();
	     redrfl=1;
	   }
	 if (che==End_Key)
	   {
	     move_end();
	     load_buff();
	     redrfl=1;
	   }

	 if (che==Escape_Key)
	   {
	     scc=choose_win(ASK_X,ASK_Y,ASK_HEI,"message",
			    "Save configuration(y/n/Cancel)?",
			    "ync",lac,lav,mwi+1);
	     if (scc!='c')
	       {
		 if (scc=='y')
		   {
		     fpc=fopen(".regaamc","w");
		     if (fpc!=NULL)
		       save_config(fpc);
		     else
		       message_win(ASK_X,ASK_Y,ASK_HEI,"message",
				   "Unable to open file! ... press any key",
				   lac,lav,mwi+1);
		   }
		 exit(1);
	       }
	   }
	 if (che==F8_Key)
	   {
	     getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Configuration filename to save? ",
		       str,lac,lav,mwi+1);
	     if (strlen(str)!=0)
	       {
		 scc=0;
		 fpc=fopen(str,"r");
		 if (fpc!=NULL)
		      scc=choose_win(ASK_X,ASK_Y,ASK_HEI,"message",
				     "File exist -- Overwrite (y/n)?",
				     "yn",lac,lav,mwi+1);
		 if ((fpc==NULL)||(scc=='y'))
		   {
			fpc=fopen(str,"w");
			if (fpc!=NULL)
			  save_config(fpc);
			else
			  message_win(ASK_X,ASK_Y,ASK_HEI,"message",
				      "Unable to open file! ... press any key",
				      lac,lav,mwi+1);
		   }

	       }
	   }
	 if (che==F9_Key)
	   {
	     getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Configuration filename to load? ",
		       str,lac,lav,mwi+1);
	     if (strlen(str)!=0)
	       {
		 fpc=fopen(str,"r");
		 if (fpc!=NULL)
		   {
		     load_config(fpc,mwi);
		     load_buff();
		     redrfl=1;
		   } else {
		     message_win(ASK_X,ASK_Y,ASK_HEI,"message","File does not exsist! ... press any key",
				 lac,lav,mwi+1);
		   }
	       }
	   }
	 if (ch=='<')
	   {
	     if (res==1) res=2;
	     else if (res==-2) res=1;
	     else if (res<-2) res=res/2;
	     else {
	       res1=res;
	       while(res1>10) res1=res1/10;
	       if ((res1%5)==0) res*=2;
	       else if ((res1%4)==0) res=5*res/4;
	       else if ((res1%2)==0) res*=2;
	     }
	     load_buff();
	     redrfl=1;
	   }

	 if (ch=='>')
	   {
	     if (res==1) res=-2;
	     else if (res<-1) res=res*2;
	     else {
	       res1=res;
	       while(res1>10) res1=res1/10;
	       if ((res1%10)==0) res=res/2;
	       else if ((res1%5)==0) res=4*res/5;
	       else if ((res1%2)==0) res=res/2;
	     }
	     load_buff();
	     redrfl=1;
	   }
	 if ((!expfl)&&(ch=='a'))
	   {
	     addnewcol();
	     redrfl=1;
	   }
	 if ((!expfl)&&(ch=='A'))
	   {
	     addquarternewcol();
	     redrfl=1;
	   }

	 if  (ch=='s')
	   {
	     swapchan();
	     redrfl=1;
	     load_buff();
	   }
	 if  (ch=='g')
	   {
	     if (gridtype==1) gridtype=0;
	     else gridtype=1;
	     redrfl=1;
	   }
	 if (ch=='i')
	   {
	     for(i=0;i<(tr->cnon);i++) (tr->muly)[i]=inc_mul((tr->muly)[i]);
	     redrfl=1;
	   }
	 if (ch=='d')
	   {
	     for(i=0;i<(tr->cnon);i++) (tr->muly)[i]=dec_mul((tr->muly)[i]);
	     redrfl=1;
	   }
	 if (ch=='p')
	   {
	     poifl=1;
	     redrfl=1;
	   }
	 if (ch=='m')
	   {
	     if (poimode==1) poimode=0; else poimode=1;
	     redrfl=1;
	   }
	 if (ch=='T')
	   {
	     getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Spike time file?  ",
		       det_name,lac,lav,mwi+1);
	     open_detfile(mwi);
	     if (detfl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }

	 if (ch=='t')
	   {
	     sprintf(det_name,"%s.res",basenm);
	     open_detfile(mwi);
	     if (detfl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }

	 if (ch=='h')
	   {
	     sprintf(whl_name,"%s.whl",basenm);
	     open_whlfile(mwi);
	     if (whlfl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }

	 if (ch=='H')
	   {
	     getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Whl file?  ",
		       whl_name,lac,lav,mwi+1);
	     open_whlfile(mwi);
	     if (whlfl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }
	 if (ch=='b')
	   {
	     if (!bmfl)
	       {
		 getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Bookmark file?  ",
			   bmflnm,lac,lav,mwi+1);
		 open_bmfile(mwi);
	       }
	     if (bmfl)
	       {
		 if (fscanf(bmfp,"%li",&newpos)!=1) {
		   message_win(ASK_X,ASK_Y,ASK_HEI,"message",
			       "Bookmark file ended! ... press any key",
			       lac,lav,mwi+1);
		   bmfl=0;
		 }
		 else {
		   buffp=newpos-sclen()/2;
		   load_buff();
		   redrfl=1;
		 }
	       }
	   }
	 if (ch=='c')
	   {
	     sprintf(clu_name,"%s.clu",basenm);
	     open_clufile(mwi);
	     if (clufl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }

	 if (ch=='C')
	   {
	     getstring(ASK_X,ASK_Y,ASK_HEI,"File ask","Cluster id file?  ",
		       clu_name,lac,lav,mwi+1);
	     open_clufile(mwi);
	     if (clufl)
	       {
		 redrfl=1;
		 load_buff();
	       }
	   }
	 if (((clufl)&&(ch=='R'))||((rasfl==0)&&(ch=='r'))) {
	   redrfl=1;
	   rasfl=(rasfl)?0:1;
	   if (rasfl) init_raster();
	   load_buff();
	 }
	 if ((rasfl)&&(ch=='l'))
	   {
	     ralifl=(ralifl)?0:1;
	     redrfl=1;
	   }
	 if ((rasfl)&&(ch=='r'))
	   {
	     order_raster(mwi,mkey);
	     redrfl=1;
	   }
	 if (ch=='x')
	   {
	     fpextr=fopen("out","wb");
	     if (fpextr==NULL)
	       {
		 printf("File error\n");
		 exit(1);
	       }
	     extract_fl=1;
	     load_buff();
	     fclose(fpextr);
	     extract_fl=0;
	   }
	 if (ch=='X')
	   {
	     fpextr=fopen("out","w");
	     if (fpextr==NULL)
	       {
		 printf("File error\n");
		 exit(1);
	       }
	     extract_fl=2;
	     load_buff();
	     fclose(fpextr);
	     extract_fl=0;
	   }
	 if (ch=='P')
	   {
	     printf("Print\n");
	     fppr=fopen("out","w");
	     if (fppr==NULL)
	       {
		 printf("File error\n");
		 exit(1);
	       }
	     cyo=cy;
	     cxo=cx;
	     cgapo=cgap;
	     cy=5000;
	     reso=res;
	     if ((res<5)||(res%8==0))
	       {
		 mulf=8;
		 for(i=0;i<3;i++)
		 {
		   if (res>1) res=res/2;
		   else if (res<=-2) res=res*2;
		   else res=-2;

		 }
	       }else {
		 mulf=10;
		 if (res%10==0) {res=res/10;}
		 else { res=-2; /* prev res must be 5 */}
	       }


	     cx=cx*mulf;
	     cgap=cgap*mulf;

	     load_buff();
	     prfl=1;

	     redrfl=1;
	     arr_scale();

	   }
	 if ((detfl)&&(ch=='u'))
	   {
            which_mulres(mwi,mkey);
	    load_buff();
	    redrfl=1;
	   }
	 if ((clufl)&&(rasfl)&&(ch=='w'))
	   {
	     rawifl=1;
	     redrfl=1;
	   }
       }
      if (eve==MOUPR)
	{
	  mousecoord(&xx,&yy,&mkey,mwi);

	  if ((poimode)&&(mkey==1))
	    {
	      poipos=xx-scrhead;
	      if (res<0) poipos=-poipos/res;
	      poifl=1;
	      redrfl=1;
	    }
	  if (((poimode==0)&&(mkey==1))||(mkey==2))
	    {

	      if (cy-yy>GRIP) /* point bellow the axis */
	      markchan(xx,yy,scrhead,mwi,mkey);
	      else
		{
		  selfl=0; /* swith single multi colom mode */
		  expch=(xx-scrhead)/(cx+cgap);
		  if (expch<0) expch=0;
		  if (expch>=tr->nocol) expch=tr->nocol;
		  idchan=expch;
		  swapchan();
		  redrfl=1;
		  load_buff();
		}
	    }
	  if (mkey==4)
	    {
	      addchan(xx,yy,scrhead,mwi,mkey);
	    }
	}
}




main(int argc, char *argv[])
{
  FILE *fpc;
  int slen;
  int mwi,i,j;
  char str[100],ch;
  int eve,evewin,po;
  tr=&tr1;
  lac=argc;lav=argv;



  if (!strcmp(argv[1],"-12"))
    {bitn=2048;po=2;}
  else if (!strcmp(argv[1],"--print"))
    {bitn=0;po=2;print_only=1;}
  else {po=1;bitn=0;}
  fpd=fopen(argv[po++],"rb");
  if (fpd==NULL)
    {
      printf("Error opening data files\n");
      exit(1);
    }

  strcpy(dataflnm,argv[po-1]);

  for(i=0;i<=strlen(dataflnm);i++)
     if (dataflnm[i]=='.')
       {basenm[i]=0;break;}
     else
       basenm[i]=dataflnm[i];


  if (sscanf(argv[po++],"%i",&chnoin)!=1)
    {
      printf("Error reading chno\n");
       exit(1);
    }
  chno=chnoin; /* it will be changed later in loadconfig */
  if (sscanf(argv[po++],"%f",&smrate)!=1)
    {
      printf("Error reading chno\n");
      exit(1);
    }
  if (argc>po)
    {
      fpc=fopen(argv[po++],"r");
      if (argc>po)
	{
	  strcpy(prfilename,argv[po++]);
	  prtypefl=1;
	}

     }
  else {
    fpc=fopen(".regaamc","r");
  }
  if (fpc!=NULL)
     load_config(fpc,mwi);
  else init_config();

  mwi=0;
  scrhead=100;

  if (!print_only)
    {
      setpalette();
      init_screen();
      setbackgr(BACKGRCOLOR,mwi);
      open_window(xs, ys,cx*(tr->nocol)+((tr->nocol)-1)*cgap+scrhead,cy,
		  "regaa","regaa",argc,argv,mwi);
      xwindow_open=1;
    }
  load_buff();


  if (print_only)
    {
      eve=KEYPR;
      select_main(eve,mwi,scrhead,fpc);
      draw_traces(mwi,scrhead);
    }
  else
    {
      while(1)
	{
	  if (!redrfl) eve=next_event(&evewin); else eve=NOEV;
	  if (selfl) select_sel(eve,mwi,scrhead);
	  else if (poifl) select_poi(eve,mwi,scrhead);
	  else if (rawifl) raster_width(eve,mwi,scrhead);
	  else select_main(eve,mwi,scrhead,fpc);
	  if ((!redrfl)&&(prfl)) {
	    fclose(fppr);
	    redrfl=1;
	    prfl=0;
	    arr_scale();
	    cx=cxo;
	    cy=cyo;
	    cgap=cgapo;
	    res=reso;
	    load_buff();
	  }
	}
    }
}
