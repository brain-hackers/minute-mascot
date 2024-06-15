extern HINSTANCE ghInst;
extern HWND      ghWnd;
extern HWND      ghWndCB;

#define	PIC_WIDTH_MAX	800
#define PIC_HEIGHT_MAX	600

extern int bitspixel;

typedef struct { 
	BITMAPINFOHEADER	bmih ;
	RGBQUAD	rgq[ 256 ] ;
} BMI ;
extern BMI	bmi;
extern BITMAPFILEHEADER	BmpFH ;
extern BITMAPINFOHEADER	BmpIH ;

extern int	winWidth,winHeight;
extern int bmp_width,bmp_height,bmp_pixel;

extern TCHAR path[256];

typedef struct {
	BYTE *buf;
	TCHAR	filename[64];
	int x,y;
	int	xsize,ysize;
	int bitspixel;
	int *palette;
	int	mask_color;
	BOOL	mask_flag;
} PIC_STRUCT;

typedef struct {
	TCHAR str[128];
	int x,y;
	int fontsize;
	int color;
} MOJI_STRUCT;

typedef struct {
	int pic[10],mask[10];
	int str_num[10];
	int str[10][10];
	int interval1,interval2;
} GROUP_STRUCT;

typedef struct {
	int	group;
	BYTE week[7];
	BYTE month[12+1];
	BYTE day[31+1];
	BYTE hour[24];
	BYTE minute[60];
	BYTE second[60];
} SCHEDULE_STRUCT;

extern void get_path(void);
extern void load_core(PIC_STRUCT *,BOOL);
extern void	set_pixel(PIC_STRUCT *,int,int,int);
extern void	set_pixel_transparent(PIC_STRUCT *,int,int,int,int,int);
extern int	get_pixel(PIC_STRUCT *,int,int); 
extern int	get_pixel_raw(PIC_STRUCT *,int,int); 
extern void get_display_depth(PIC_STRUCT *);

extern int raw_black,raw_white,black,white;

extern void save_hwnd(HWND);
extern HWND load_hwnd(void);

extern BYTE *baseBuf;
extern HDC		baseDC;		// オフスクリーン
extern HDC		tmp256DC;		// オフスクリーン
extern HBITMAP	tmp256Bmp;
extern BYTE *tmp256Buf;
extern HDC		NullDC;

extern PIC_STRUCT *pic,*mask;
extern MOJI_STRUCT *moji;

enum {
	TRANS_100,TRANS_75,TRANS_50,TRANS_25,TRANS_HALF
};

extern int max_pic,max_mask,max_string,*max_group,max_schedule;

