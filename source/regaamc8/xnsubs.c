#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include "xnsubs.h"
#include "icon_bitmap"
#define BITMAPDEPTH 1
#define MW 10

Display *display;
int screen_num;
Screen *screen_ptr;
extern int colors[];


int win_no;
unsigned long foreground_pixel, background_pixel, border_pixel;

/* values for window_size in main, is window big enough to be useful? */
#define SMALL 1
#define OK 0
XFontStruct *font_info;
XSizeHints size_hints[MW];
XEvent report;


KeySym keysy;


char *progname;
Window win[MW];
GC gc[MW];
int ac;
char **av;
unsigned int width[MW], height[MW],xb,yb;     /* window size and position */


void line(int xx1,int yy1,int xx2,int yy2, int wipo)
{
  XDrawLine(display,win[wipo],gc[wipo],xx1,yy1,xx2,yy2);
}

void clearwindow(int wipo)
{
  XClearWindow(display,win[wipo]);
}


void mousecoord(int *x, int *y, int *mkey, int wipo)
{
  Window jn1,jn2;
  int r1,r2;
  unsigned int mask;
  XQueryPointer(display,win[wipo],&jn1,&jn2,&r1,&r2,x,y,&mask);
  *mkey=mask>>8;

}

void getkey(char *ch)
{
  XKeyEvent *keyev;
  char chh[2];
  chh[0]=0;
  keyev=(XKeyEvent*)&report;
  XLookupString(keyev,chh,1,&keysy,NULL);
  *ch=chh[0];
}


int get_extra_keys(void)
{
  if (keysy== XK_Left) return Left_Key;
  if (keysy== XK_Right) return Right_Key;
  if (keysy== XK_F1) return F1_Key;
  if (keysy== XK_F2) return F2_Key;
  if (keysy== XK_F3) return F3_Key;
  if (keysy== XK_F4) return F4_Key;
  if (keysy== XK_F5) return F5_Key;
  if (keysy== XK_F6) return F6_Key;
  if (keysy== XK_F7) return F7_Key;
  if (keysy== XK_F8) return F8_Key;
  if (keysy== XK_F9) return F9_Key;
  if (keysy== XK_F10) return F10_Key;
  if (keysy== XK_Escape) return Escape_Key;
  if (keysy== XK_Home) return Home_Key;
  if (keysy== XK_End) return End_Key;
  if (keysy== XK_Up) return Up_Key;
  if (keysy== XK_Down) return Down_Key;
  if (keysy== XK_BackSpace) return BackSpace_Key;

  return 0;

}



void outtextxy(int x, int y, char st[], int wipo)
{
  XDrawString(display,win[wipo],gc[wipo],x,y,st,strlen(st));
}

void getwinsize(int *xw,int *yw, int wipo)
{
 *yw=height[wipo];
 *xw=width[wipo];

}

void close_screen(void)
{
  XUnloadFont(display, font_info->fid);
  XCloseDisplay(display);

}

void close_window(int wipo)
{
  XDestroyWindow(display,win[wipo]);
  XFreeGC(display, gc[wipo]);
  win_no--;
}

void setcolor(int x, int wipo)
{
 int y;
 y=x%MAX_COLORS;
 XSetForeground(display, gc[wipo], colors[y]);
}

void setbackgr(int x, int wipo)
{
 int y;
 y=x%MAX_COLORS;
 background_pixel=colors[y];
}

void linewidth(int width,int wipo)
{
  XSetLineAttributes(display,gc[wipo],width,LineSolid,CapRound,JoinRound);
}

void putpoint(int x, int y, int wipo)
{
  XDrawPoint(display,win[wipo],gc[wipo],x,y);
}


int find_win(Window ww)
{
  int i,poo;
  for(i=0;i<win_no;i++)
    {
      if (win[i]==ww){  poo=i; break;}
    }

  if (i==win_no)
    {
      printf("Panic could not find window winno= %i\n",win_no);
/*      exit(1);*/
    }

 return poo;
}


Bool dummy(void)
{
return True;
}

void back_last_event(void)
{
XPutBackEvent(display,&report);
}

int chech_event(void)
{
 if ( XCheckIfEvent(display,&report,dummy,"")==False) return 0;
 back_last_event();
 return 1;

}


int next_event(int *wipo)
{
  

	int window_size = 0;   
	int evetype,i;
	XNextEvent(display, &report);
	switch  (report.type) {
	case Expose:

	 *wipo= find_win(report.xexpose.window);
	  /* get all other Expose events on the queue */
	  while (XCheckTypedEvent(display, Expose, &report));
	  if (window_size == SMALL)
	    TooSmall(win[*wipo], gc[*wipo], font_info);
	  else {
	    evetype=EXPOSE;
	  }
	  break;
	case ConfigureNotify:
	  /* window has been resized, change width and
	   * height to send to place_text and place_graphics
	   * in next Expose */
	  evetype=CHCONF;
	 *wipo= find_win(report.xconfigure.window);
	  width[*wipo] = report.xconfigure.width;
	  height[*wipo] = report.xconfigure.height;
	  if ((width[*wipo] < size_hints[*wipo].min_width) || 
	      (height[*wipo] < size_hints[*wipo].min_height))
	    window_size = SMALL;
	  else
	    window_size = OK;
	  break;
	case ButtonPress:
	  /* trickle down into KeyPress (no break) */
	  evetype=MOUPR;
	 *wipo= find_win(report.xbutton.window);
	  break;
	case KeyPress:
	  evetype=KEYPR;
	  break;
	default:
	  evetype=OTHER;
	  break;
	} /* end switch */
return evetype;

}

void init_screen(void)
{


	unsigned int display_width, display_height;
	char *display_name = NULL;

	/* connect to X server */
	if ( (display=XOpenDisplay(display_name)) == NULL )
	{
		(void) fprintf( stderr, 
				"basicwin: cannot connect to X server %s\\n",
				XDisplayName(display_name));
		exit( -1 );
	}

	/* get screen_num size from display structure macro */
	screen_num = DefaultScreen(display);
	screen_ptr = DefaultScreenOfDisplay(display);

	get_colors();
	load_font(&font_info);
	win_no=0;
}

void open_window(int xs, int ys,int xw,int yw,char *window_name, char *icon_name,
		 int acc, char **avv, int wipo)
{


	unsigned int borderwidth = 4;	      /* four pixels */
	unsigned int icon_width, icon_height;
	Pixmap icon_pixmap;

	ac=acc;av=avv;


	xb = xs, yb = ys;
	width[wipo] = xw, height[wipo] = yw;


	/* create opaque window */
	win[wipo] = XCreateSimpleWindow(display, RootWindow(display,screen_num), xb, yb, 
			width[wipo], height[wipo], borderwidth, border_pixel,
	    		background_pixel);

	/* Create pixmap of depth 1 (bitmap) for icon */
	icon_pixmap = XCreateBitmapFromData(display, win[wipo], icon_bitmap_bits, 
			icon_bitmap_width, icon_bitmap_height);

	/* Set resize hints */
	size_hints[wipo].flags = PPosition | PSize | PMinSize;
	size_hints[wipo].x = xb;
	size_hints[wipo].y = yb;
	size_hints[wipo].width = width[wipo];
	size_hints[wipo].height = height[wipo];
	size_hints[wipo].min_width = 10;
	size_hints[wipo].min_height = 10;

	/* set Properties for window manager (always before mapping) */
	XSetStandardProperties(display, win[wipo], window_name, icon_name, 
	    icon_pixmap, av, ac, &size_hints[wipo]);

	/* Select event types wanted */
	XSelectInput(display, win[wipo], ExposureMask | KeyPressMask | 
			ButtonPressMask | StructureNotifyMask);


	/* create GC for text and drawing */
	get_GC(win[wipo], &gc[wipo], font_info);

	/* Display window */
	XMapWindow(display, win[wipo]);
	win_no++;
      }




get_GC(Window win, GC *gc, XFontStruct *font_info)
{
	unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
	XGCValues values;
	unsigned int line_width = 1;
	int line_style = LineSolid;
	int cap_style = CapRound;
	int join_style = JoinRound;
	int dash_offset = 0;
	static char dash_list[] = {
		12, 24	};
	int list_length = 2;

	/* Create default Graphics Context */
	*gc = XCreateGC(display, win, valuemask, &values);

	/* specify font */
	XSetFont(display, *gc, font_info->fid);

	/* specify black foreground since default may be white on white */
	XSetForeground(display, *gc, foreground_pixel);

	/* set line attributes */
	XSetLineAttributes(display, *gc, line_width, line_style, cap_style, 
			join_style);

	/* set dashes to be line_width in length */
	XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}

load_font(XFontStruct **font_info)
{
	char *fontname = "9x15";

	/* Access font */
	if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
	{
		(void) fprintf( stderr, "Basic: Cannot open 9x15 font\\n");
		exit( -1 );
	}
}


TooSmall(Window win, GC gc, XFontStruct *font_info)
{
	char *string1 = "Too Small";
	int y_offset, x_offset;

	y_offset = font_info->max_bounds.ascent + 2;
	x_offset = 2;

}






int colors[MAX_COLORS];
static char *visual_class[] = {
"StaticGray",
"GrayScale",
"StaticColor",
"PseudoColor",
"TrueColor",
"DirectColor"
};

get_colors() { int default_depth; Visual *default_visual; static char
*name[] = {"Black","green","White","MidnightBlue","SeaGreen","red","magenta2","chocolate",
"sky blue","pale green","wheat",
"purple","cyan","peru","DarkTurquoise","PowderBlue","aquamarine","magenta",
"grey","yellow",
"navy blue","dark grey","pink","SpringGreen1","SlateBlue",
"CadetBlue","DarkKhaki","coral","OrangeRed","orange","maroon1","lightgrey"}; XColor

exact_def; Colormap default_cmap; int ncolors = 0;

	int i = 5;
	XVisualInfo visual_info;
	
	/* Try to allocate colors for PseudoColor, TrueColor, 
	 * DirectColor, and StaticColor.  Use black and white
	 * for StaticGray and GrayScale */

	default_depth = DefaultDepth(display, screen_num);
        default_visual = DefaultVisual(display, screen_num);
	default_cmap   = DefaultColormap(display, screen_num);
	if (default_depth == 1) {
		/* must be StaticGray, use black and white */
		border_pixel = BlackPixel(display, screen_num);
		background_pixel = WhitePixel(display, screen_num);
		foreground_pixel = BlackPixel(display, screen_num);
		return(0);
	}

	while (!XMatchVisualInfo(display, screen_num, default_depth, /* visual class */i--, &visual_info))
		;
	
	
	if (i < 2) {
		/* No color visual available at default_depth.
		 * Some applications might call XMatchVisualInfo
		 * here to try for a GrayScale visual 
		 * if they can use gray to advantage, before 
		 * giving up and using black and white.
		 */
		border_pixel = BlackPixel(display, screen_num);
		background_pixel = WhitePixel(display, screen_num);
		foreground_pixel = BlackPixel(display, screen_num);
		return(0);
	}

	/* otherwise, got a color visual at default_depth */

	/* The visual we found is not necessarily the 
	 * default visual, and therefore it is not necessarily
	 * the one we used to create our window.  However,
	 * we now know for sure that color is supported, so the
	 * following code will work (or fail in a controlled way).
	 * Let's check just out of curiosity: */

	for (i = 0; i < MAX_COLORS; i++) {
		if (!XParseColor (display, default_cmap, name[i], &exact_def)) {
			fprintf(stderr, "color name %s not in database", name[i]);
			exit(0);
		}

   		if (!XAllocColor(display, default_cmap, &exact_def)) {
			fprintf(stderr, "Can't allocate color: all colorcells allocated and no matching cell found.\n");
		exit(0);
		}
		
		colors[i] = exact_def.pixel;
		ncolors++;
	}


	border_pixel = colors[1];
	background_pixel = colors[0];
	foreground_pixel = colors[2];
	return(1);
}


void fill_rect(int x1,int y1,int x2,int y2,int wipo)
{

 XDrawRectangle(display,win[wipo],gc[wipo],x1,y1,x2,y2);
 XFillRectangle(display,win[wipo],gc[wipo],x1,y1,x2,y2);
}



#define SBX 50
#define SBY 50

void select_update(int wid,int nx,int ny,char mask[],char *mess[])
{
  int i,j;

  
  for(i=0;i<ny;i++)
  for(j=0;j<nx;j++)
    {
      setcolor(3,wid);
      fill_rect(j*SBX,i*SBY,SBX-1,SBY-1,wid);

      if (mask[i*nx+j])
	{
	  setcolor(1,wid);
	  fill_rect(j*SBX,i*SBY,SBX-4,SBY-4,wid);
      }else 
	{
	  setcolor(0,wid);
	  fill_rect(j*SBX,i*SBY,SBX-4,SBY-4,wid);
	}
     
    }

  setcolor(2,wid);
  for(i=0;i<ny;i++)
    for(j=0;j<nx;j++) {
      outtextxy(SBX/4+SBX*j,SBY*i+SBY/2,mess[i*nx+j],wid); }
}


void select_out(int wid,int nx,int ny,char mask[],char *mess[], char winm[],int ac, char *av[])
{
  int i,eve,evewin;
  
  open_window(100,200,SBX*nx,SBY*ny,winm,winm,ac,av,wid);
  
 while(1)
   {
     
     eve=next_event(&evewin);
     if ((eve==EXPOSE)&&(evewin==wid))
	{
	  select_update(wid,nx,ny,mask,mess);
	  break;
	}
   }


}






void select_wait(int wid, int *x, int *y)
{
 int xx,yy,mkey,eve,evewin,rx,ry;


 while(1)
   {
     
     eve=next_event(&evewin);
     if ((eve==MOUPR)&&(evewin=wid))
	 {
	   mousecoord(&xx,&yy,&mkey,wid);
	   if (mkey==1)
	     {
	       rx=xx/SBX;
	       ry=yy/SBY;
	       break;
	     }
	 }
   }
 *x=rx;
 *y=ry;
}



void getstring(int x,int y, int hei,char til[],char ask[],char inp[],int ac,char *av[],int win)
{

  int len=500,i,che,eve,evewin;
  char ch,disp[100];
  int xp=10,yp=30;

  setbackgr(0,win);
  open_window(x, y,len,hei,til,til,ac,av,win);
  i=0;
  strcpy(disp,ask);
  while(1)
    {
       eve=next_event(&evewin);
       
       if (eve==EXPOSE)
	 {
	   setcolor(2,win);
	   outtextxy(xp,yp,disp,win);
	 }
       if (eve==KEYPR)
	 {
	   getkey(&ch);
	   che=get_extra_keys();
	   if (ch==0xd) break;
	   if (che==Escape_Key) {i=0;break;}
	   if (che==BackSpace_Key) 
	     {
	       if (i!=0) i--;
	       inp[i]=0;
	     } else if (ch!=0)
	       {
		 inp[i++]=ch;
		 if (i>40) i=40;
		 inp[i]=0;
	       }
	   setcolor(0,win);
	   outtextxy(xp,yp,disp,win);

	   strcpy(disp,ask);
	   strcat(disp,inp);
	   setcolor(2,win);
	   outtextxy(xp,yp,disp,win);
	 }
       
    }
    inp[i]=0;
    close_window(win);


}




void message_win(int x,int y, int hei,char til[],char tell[],int ac,char *av[],int win)
{

  int len,eve,evewin;
  char ch,disp[100];
  int xp=10,yp=30;
  len=XTextWidth(font_info,tell,strlen(tell))+2*xp;
  setbackgr(0,win);
  open_window(x, y,len,hei,til,til,ac,av,win);
  while(1)
    {
       eve=next_event(&evewin);
       
       if (eve==EXPOSE)
	 {
	   setcolor(2,win);
	   outtextxy(xp,yp,tell,win);
	 }
       if (eve==KEYPR)
	 {
	   getkey(&ch);
	   break;
	 }
       if (eve==MOUPR)
	 {
	   break;
	 }
       
    }

    close_window(win);

}



char choose_win(int x,int y, int hei,char til[],char tell[],char sel[],int ac,char *av[],int win)
{

  int len,i,eve,evewin;
  char ch;
  int xp=10,yp=30;

  setbackgr(0,win);
  len=XTextWidth(font_info,tell,strlen(tell))+2*xp;
  open_window(x, y,len,hei,til,til,ac,av,win);
  while(1)
    {
       eve=next_event(&evewin);
       if (eve==EXPOSE)
	 {
	   setcolor(2,win);
	   outtextxy(xp,yp,tell,win);
	 }
       if (eve==KEYPR)
	 {
	   getkey(&ch);
	   
	   for(i=0;i<strlen(sel);i++)
	       if (ch==sel[i]) goto ki;
	 }
      
    }

    
  ki:  close_window(win);
    return ch;
}


