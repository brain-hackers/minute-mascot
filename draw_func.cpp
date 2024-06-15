#ifndef	UNICODE
#define	UNICODE
#endif	/* UNICODE */
#ifndef	_UNICODE
#define	_UNICODE
#endif	/* _UNICODE */
#define	STRICT
#include <windows.h>
#include <commctrl.h>
#include	<commdlg.h>
#include <wingdi.h>
#include "resource.h" 
#include <Winbase.h>
#include "MMascot.h"

BYTE depth[3];
int depth_white;
BYTE add_num;

extern TCHAR Message[256];
int	test;

int	raw_black,raw_white;
int	black,white;

void get_display_depth(PIC_STRUCT *p)
{
	int	backup,c,i,k;

	BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
	backup=get_pixel(p,0,0);

	if (bitspixel==8) {
		raw_black=0x000000;
		for (i=0;i<=0xff;i++) {
			set_pixel(p,0,0,0x010101*i);
			BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
			BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
			c=get_pixel(p,0,0);
			if (c!=raw_black) {
				black=c;
				break;
			}
		}
		raw_white=0xffffff;
		for (i=0xff;i>=0;i--) {
			set_pixel(p,0,0,0x010101*i);
			BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
			BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
			c=get_pixel(p,0,0);
			if (c!=raw_white) {
				white=c;
				break;
			}
		}

		for (i=0;i<256;i++) {
			p->buf[0]=i;
			BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
			BitBlt(baseDC, 0,0,1,1,NullDC, 0,0, SRCCOPY);
		}
	}
	
	if (bitspixel==24) {
		set_pixel(p,0,0,0xffffff);
		BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
		BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
		c=get_pixel(p,0,0);
		raw_white=c;
		depth[0]=(raw_white>>16)&0xff;
		depth[1]=(raw_white>>8 )&0xff;
		depth[2]=(raw_white    )&0xff;
		raw_black=0x000000;
		black=1;
		white=raw_white;
		if (c%256==0xff) black=0x000001;
		if (c%256==0xfe) black=0x000002;
		if (c%256==0xfc) black=0x000004;
		if (c%256==0xf8) black=0x000008;
		if (c%256==0xf0) black=0x000010;
		if (c%256==0xe0) black=0x000020;
		if (c%256==0xc0) black=0x000040;
		if (c&256==0x80) black=0x000080;
		white-=black;
		test=c;
	}

	for (k=0;k<=max_pic;k++) {
		if (pic[k].buf==NULL) continue;
		for (i=0;i<(1<<pic[k].bitspixel);i++) {
			set_pixel(p,0,0,pic[k].palette[i]);
			BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
			BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
			c=get_pixel(p,0,0);
			if (c==raw_black) c=black;
			if (c==raw_white) c=white;
			if (bitspixel==8) {
				int cc;
				set_pixel(p,0,0,c);
				BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
				for (;;) {
					BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
					cc=get_pixel(p,0,0);
					if (cc==c) break;
					c=cc;
					BitBlt(tmp256DC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
					BitBlt(NullDC, 0,0,1,1, tmp256DC, 0,0, SRCCOPY);
				}
			}
			pic[k].palette[i]=c;
		}
	}

	for (k=0;k<=max_string;k++) {
		if (moji[k].fontsize==-1) continue; 
		set_pixel(p,0,0,moji[k].color);
		BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
		BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
		c=get_pixel(p,0,0);
		if (c==raw_black) c=black;
		if (c==raw_white) c=white;
		if (bitspixel==8) {
			int cc;
			set_pixel(p,0,0,c);
			BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);
			for (;;) {
				BitBlt(baseDC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
				cc=get_pixel(p,0,0);
				if (cc==c) break;
				c=cc;
				BitBlt(tmp256DC, 0,0,1,1, NullDC, 0,0, SRCCOPY);
				BitBlt(NullDC, 0,0,1,1, tmp256DC, 0,0, SRCCOPY);
			}
		}
		moji[k].color=c;
	}

	set_pixel(p,0,0,backup);
	BitBlt(NullDC, 0,0,1,1, baseDC, 0,0, SRCCOPY);				
}

int	get_pixel(PIC_STRUCT *pic,int x,int y)
{
	int c;
	if (pic->bitspixel==24) {
		int k=((pic->ysize-1-y)*(pic->xsize)+x)*3;
		BYTE r,g,b;
		b=pic->buf[k  ];
		g=pic->buf[k+1];
		r=pic->buf[k+2];
		return (int)((b<<16) | (g<<8) | r);
	}
	if (pic->bitspixel==1) {
		BYTE shift[8]={7,6,5,4,3,2,1,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+7)/8)+x/8]>>shift[x%8])&1; 
	}
	if (pic->bitspixel==2) {
		BYTE shift[4]={6,4,2,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+3)/4)+x/4]>>shift[x%4])&3; 
	}
	if (pic->bitspixel==4) {
		BYTE shift[2]={4,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+1)/2)+x/2]>>shift[x%2])&15; 
	}
	if (pic->bitspixel==8) {
		c= pic->buf[(pic->ysize-1-y)*pic->xsize+x]; 
	}
	if (pic->palette==NULL)	return c;
	return pic->palette[c];
}

int	get_pixel_raw(PIC_STRUCT *pic,int x,int y)
{
	int c;
	if (pic->bitspixel==24) {
		int k=((pic->ysize-1-y)*(pic->xsize)+x)*3;
		BYTE r,g,b;
		b=pic->buf[k  ];
		g=pic->buf[k+1];
		r=pic->buf[k+2];
		return (int)((b<<16) | (g<<8) | r);
	}
	if (pic->bitspixel==1) {
		BYTE shift[8]={7,6,5,4,3,2,1,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+7)/8)+x/8]>>shift[x%8])&1; 
	}
	if (pic->bitspixel==2) {
		BYTE shift[4]={6,4,2,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+3)/4)+x/4]>>shift[x%4])&3; 
	}
	if (pic->bitspixel==4) {
		BYTE shift[2]={4,0};
		c= (pic->buf[(pic->ysize-1-y)*((pic->xsize+1)/2)+x/2]>>shift[x%2])&15; 
	}
	if (pic->bitspixel==8) {
		c= pic->buf[(pic->ysize-1-y)*pic->xsize+x]; 
	}
	return c;
}

void set_pixel(PIC_STRUCT *pic,int x,int y,int c)
{
	int xs,ys,bp;
	xs=pic->xsize;
	ys=pic->ysize;
	bp=pic->bitspixel;

	if (x<0 || xs<=x || y<0 || ys<=y) return;

	if (bp==1) {
		static BYTE and_mask[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
		static BYTE shift[8]={7,6,5,4,3,2,1,0};
		if (c>=2) return;
		int k=(ys-1-y)*((xs+7)/8)+x/8;
		pic->buf[k]&=and_mask[x%8];
		pic->buf[k]|=c<<shift[x%8];
	}
	if (bp==2) {
		static BYTE and_mask[4]={0x3f,0xcf,0xf3,0xfc};
		static BYTE shift[4]={6,4,2,0};
		if (c>=4) return;
		int k=(ys-1-y)*((xs+3)/4)+x/4;
		pic->buf[k]&=and_mask[x%4];
		pic->buf[k]|=c<<shift[x%4];
	}
	if (bp==4) {
		static BYTE and_mask[2]={0x0f,0xf0};
		static BYTE shift[2]={4,0};
		if (c>=16) return;
		int k=(ys-1-y)*((xs+1)/2)+x/2;
		pic->buf[k]&=and_mask[x%2];
		pic->buf[k]|=c<<shift[x%2];
	}
	if (bp==8) {
		if (c>=256) return;
		int k=(ys-1-y)*xs+x;
		pic->buf[k]=c;
	}
	if (bp==24) {
		int k=((ys-1-y)*xs+x)*3;
		pic->buf[k  ]=(c>>16);
		pic->buf[k+1]=(c>>8 );
		pic->buf[k+2]=(c>>0 );
	}
}
void set_pixel_transparent(PIC_STRUCT *pic,int x,int y,int c,int c2,int tr)
{
	int xs,ys;
	BYTE r1,g1,b1;
	BYTE r2,g2,b2;
	BYTE rr,gg,bb;

	if (tr==TRANS_100) {
		set_pixel(pic,x,y,c);
		return;
	}
	if (tr==TRANS_HALF) {
		if ((x%2)==(y%2))
			set_pixel(pic,x,y,c);
		return;
	}
	
	xs=pic->xsize;
	ys=pic->ysize;

	if (x<0 || xs<=x || y<0 || ys<=y) return;

	int k=((ys-1-y)*xs+x)*3;
	r1=(c>>16)&0xff;
	g1=(c>>8 )&0xff;
	b1=(c    )&0xff;
	r2=(c2>>16)&0xff;
	g2=(c2>>8 )&0xff;
	b2=(c2    )&0xff;
	
	if (tr==TRANS_25) {
		rr=(r1+r2*3)/4;
		gg=(g1+g2*3)/4;
		bb=(b1+b2*3)/4;
	}
	if (tr==TRANS_50) {
		rr=(r1+r2)/2;
		gg=(g1+g2)/2;
		bb=(b1+b2)/2;
	}
	if (tr==TRANS_75) {
		rr=(r1*3+r2)/4;
		gg=(g1*3+g2)/4;
		bb=(b1*3+b2)/4;
	}

	pic->buf[k  ]=rr & depth[0];
	pic->buf[k+1]=gg & depth[1];
	pic->buf[k+2]=bb & depth[2];
}
