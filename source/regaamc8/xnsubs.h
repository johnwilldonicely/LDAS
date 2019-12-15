void open_window(int xs, int ys,int xw,int yw,char *window_name, char *icon_name,
		 int acc, char **avv, int wipo);
void setcolor(int x, int wipo);
void setbackgr(int x, int wipo);
void putpoint(int x, int y, int wipo);
void close_window(int wipo);
int next_event(int *wipo);
void getwinsize(int *xw,int *yw, int wipo);
void clearwindow(int wipo);
void mousecoord(int *x, int *y,int *mkey, int wipo);
void getkey(char *ch);
void line(int xx1,int yy1,int xx2,int yy2,int wipo);
void init_screen(void);
void close_screen(void);
void outtextxy(int x, int y, char st[], int wipo);
void select_out(int wid,int nx,int ny,char mask[],char *mess[], char winm[],int ac, char *av[]);
void fill_rect(int x1,int y1,int x2,int y2,int wipo);
void select_update(int wid,int nx,int ny,char mask[],char *mess[]);
void select_wait(int wid, int *x, int *y);
void linewidth(int width,int wipo);
int get_extra_keys(void);
void getstring(int x,int y, int hei, char til[],char ask[],char inp[],int ac,char *av[],int win);
void message_win(int x,int y, int hei,char til[],char tell[],int ac,char *av[],int win);
char choose_win(int x,int y, int hei,char til[],char tell[],char sel[],int ac,char *av[],int win);
void back_last_event(void);
int chech_event(void);


#define NOEV   0
#define EXPOSE 1
#define CHCONF 2
#define MOUPR  3
#define KEYPR  4
#define OTHER  5
#define MAX_COLORS 32




#define Left_Key 1
#define Right_Key 2
#define F1_Key 3
#define F2_Key 4
#define F3_Key 5
#define F4_Key 6
#define F5_Key 7
#define F6_Key 8
#define F7_Key 9
#define F8_Key 10
#define F9_Key 11
#define F10_Key 12
#define Escape_Key 13
#define Home_Key 14
#define End_Key 15 
#define Up_Key 16
#define Down_Key 17
#define BackSpace_Key 18

#define BLACK 0
#define WHITE 2
