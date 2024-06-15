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

BMI bmi ;
BITMAPFILEHEADER	BmpFH ;
BITMAPINFOHEADER	BmpIH ;

TCHAR path[256];
extern TCHAR Message[256];

void load_core(PIC_STRUCT *pic,BOOL mask_flag)
{
	HANDLE	hFile;
	DWORD	readsize ;
	BYTE	buf[1024*3+1];
	int		i,j;
	int		x_file_offset;
	int		xs,ys,bc;
	TCHAR	fn[256];

	wsprintf(fn,TEXT("%s%s"),path,pic->filename);
	hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		wsprintf( Message, TEXT("File Open error %s") ,fn) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return;
	}

	ReadFile( hFile, &BmpFH, sizeof BmpFH, &readsize, NULL ) ;
	if (BmpFH.bfType!=0x4D42) {
		wsprintf( Message, TEXT("File Type error") ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return;
	}

	ReadFile( hFile, &bmi, BmpFH.bfOffBits-sizeof(BmpFH), &readsize, NULL ) ;

	xs=bmi.bmih.biWidth;
	ys=bmi.bmih.biHeight;
	bc=bmi.bmih.biBitCount;

	pic->xsize=xs;
	pic->ysize=ys;
	pic->bitspixel=bc;

	if ((GetFileSize(hFile,NULL)-BmpFH.bfOffBits)-(xs*ys*bc/8)==0x3F0)
		x_file_offset=xs*bc/8;
	else
		x_file_offset=(GetFileSize(hFile,NULL)-BmpFH.bfOffBits)/ys;

	if (bc<=8 && mask_flag==TRUE) {
		int colors=1<<(bc);
		int r,g,b;
		int i;
		pic->palette=(int *)malloc((sizeof(int))*colors);
		for (i=0;i<colors;i++) {
			r=bmi.rgq[i].rgbRed; 
			g=bmi.rgq[i].rgbGreen;
			b=bmi.rgq[i].rgbBlue;
			pic->palette[i]=(b*256+g)*256+r;
		}
	}
	if (mask_flag==FALSE) {
		pic->bitspixel=1;
		pic->palette=(int *)malloc((sizeof(int))*2);
		pic->palette[0]=0;
		pic->palette[1]=1;
	}

	if (mask_flag==TRUE) {
		if (bmi.bmih.biBitCount==1) {
			static int shift[8]={7,6,5,4,3,2,1,0};
			pic->buf=(BYTE *)malloc(((xs+7)/8)*ys);
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ; 
				for (i=0;i<xs;i++) {
					set_pixel(pic,i,j,(buf[i/8]>>shift[i%8])&1);
				}	
			}
		}
		if (bmi.bmih.biBitCount==2) {
			static int shift[4]={6,4,2,0};
			pic->buf=(BYTE *)malloc(((xs+3)/4)*ys);
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					set_pixel(pic,i,j,(buf[i/4]>>shift[i%4])&3);
				}	
			}
		}
		if (bmi.bmih.biBitCount==4) {
			static int shift[2]={4,0};
			pic->buf=(BYTE *)malloc(((xs+1)/2)*ys);
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					set_pixel(pic,i,j,(buf[i/2]>>shift[i%2])&0xf);
				}	
			}
		}
		if (bmi.bmih.biBitCount==8) {
			pic->buf=(BYTE *)malloc(xs*ys);
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ; 
				for (i=0;i<xs;i++) {
					set_pixel(pic,i,j,buf[i]);
				}	
			}  
		}  
		if (bmi.bmih.biBitCount==24) { 
			pic->buf=(BYTE *)malloc(xs*ys*3);
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					int	r,g,b;
					b=buf[i*3];
					g=buf[i*3+1];
					r=buf[i*3+2];
					set_pixel(pic,i,j,(b*256+g)*256+r);
				}	
			}
		}
	} else {
		pic->bitspixel=1;
		pic->buf=(BYTE *)malloc(((xs+7)/8)*ys);
		if (bmi.bmih.biBitCount==1) {
			static int shift[8]={7,6,5,4,3,2,1,0};
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ; 
				for (i=0;i<xs;i++) {
					set_pixel(pic,i,j,(buf[i/8]>>shift[i%8])&1);
				}
			}
		}
		if (bmi.bmih.biBitCount==2) {
			static int shift[4]={6,4,2,0};
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					int c=(buf[i/4]>>shift[i%4])&3;
					set_pixel(pic,i,j,(c==pic->mask_color) ? 1 : 0);
				}	
			}
		}
		if (bmi.bmih.biBitCount==4) {
			static int shift[2]={4,0};
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					int c=(buf[i/2]>>shift[i%2])&0xf;
					set_pixel(pic,i,j,(c==pic->mask_color) ? 1 : 0);
				}	
			}
		}
		if (bmi.bmih.biBitCount==8) {
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ; 
				for (i=0;i<xs;i++) {
					int c=buf[i];
					set_pixel(pic,i,j,(c==pic->mask_color) ? 1 : 0);
				}	
			}  
		}  
		if (bmi.bmih.biBitCount==24) { 
			for (j=ys-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				for (i=0;i<xs;i++) {
					int	r,g,b;
					b=buf[i*3];
					g=buf[i*3+1];
					r=buf[i*3+2];
					int c=(b*256+g)*256+r;
					set_pixel(pic,i,j,(c==pic->mask_color) ? 1 : 0);
				}	
			}
		}
	}
	
	CloseHandle( hFile ) ;
}


void get_path()
{
	int i,j;

	GetModuleFileName(NULL,path,MAX_PATH);
	j=0;
	for (i=0;;i++) {
		if (path[i]=='\\') j=i;
		if (path[i]=='\0') {
			path[j]='\\';
			path[j+1]='\0';
			break;
		} 
	}
}


void save_hwnd(HWND hwnd)
{
	HANDLE	hFile;
	DWORD	writesize ;
	TCHAR	fn[256];
	TCHAR	p[MAX_PATH];
	int i,j;

	GetModuleFileName(NULL,p,MAX_PATH);
	j=0;
	for (i=0;;i++) {
		if (p[i]=='\\') j=i;
		if (p[i]=='\0') {
			p[j]='\\';
			p[j+1]='\0';
			break;
		} 
	}
	wsprintf(fn,TEXT("%s__tmp"),p);
	hFile = CreateFile( fn, GENERIC_WRITE, 0, 0,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		wsprintf( Message, TEXT("File Save error") ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return;
	}
	WriteFile( hFile, &hwnd, sizeof(HWND), &writesize, NULL ) ; 
	CloseHandle(hFile);
}

HWND load_hwnd()
{
	HANDLE	hFile;
	DWORD	readsize ;
	TCHAR	fn[256];
	HWND	hwnd;
	TCHAR	p[MAX_PATH];
	int i,j;

	GetModuleFileName(NULL,p,MAX_PATH);
	j=0;
	for (i=0;;i++) {
		if (p[i]=='\\') j=i;
		if (p[i]=='\0') {
			p[j]='\\';
			p[j+1]='\0';
			break;
		} 
	}
	wsprintf(fn,TEXT("%s__tmp"),p);
	hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
//		wsprintf( Message, TEXT("File Open error %s") ,fn) ;
//		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return (HWND)NULL;
	}

	ReadFile( hFile, &hwnd, sizeof(HWND), &readsize, NULL ) ;
	CloseHandle(hFile);
	return hwnd;
}
