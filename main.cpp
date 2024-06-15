#ifndef	UNICODE
#define	UNICODE
#endif	/* UNICODE */
#ifndef	_UNICODE
#define	_UNICODE
#endif	/* _UNICODE */
#define	STRICT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include	<commdlg.h>
#include <wingdi.h> 
#include <Winuser.h>
#include "resource.h" 
#include <Winbase.h> 
#include "MMascot.h"
DWORD	gle=0;

/* =====================================================================
	関数のプロトタイプ宣言
===================================================================== */
// WinMainから呼ばれる関数
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);

#define		WM_NOTIFYICON	(WM_USER+2)

// コールバック関数
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
// ダイアログ関数
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
// メッセージ処理関数
// ファイル・メニュー関数
void OnFileExit(HWND);			// 終了
// ヘルプ・メニュー関数
void OnHelpAbout(HWND);			// バージョン情報

void OnCreate(HWND, WPARAM, LPARAM);
void OnDestroy(HWND, WPARAM, LPARAM);
void OnPaint(HWND, WPARAM, LPARAM);
void OnHelp(HWND, WPARAM, LPARAM);
void mouse_down(HWND,WPARAM, LPARAM);
void mouse_move(HWND,WPARAM, LPARAM);
void mouse_up(HWND,WPARAM, LPARAM);

HICON		hIcon = 0 ;


/* =====================================================================
	グローバル変数
===================================================================== */
TCHAR gszAppName[]	= TEXT("MMascot");	// クラスの名前
TCHAR gszAppTitle[]	= TEXT("MMascot");	// ウィンドウ・タイトル

HINSTANCE ghInst	= NULL;		// 現在のインスタンス
HWND      ghWnd		= NULL;		// メインウィンドウのハンドル
HWND      ghWndCB	= NULL;		// コマンドバーのハンドル
HACCEL	  hAccel    = NULL;

HDC		baseDC	= NULL;		// オフスクリーン
HBITMAP	baseBmp	= NULL;
BYTE *baseBuf;

HDC		tmp256DC	= NULL;		// オフスクリーン
HBITMAP	tmp256Bmp	= NULL;
BYTE *tmp256Buf;

HDC NullDC=NULL;

BYTE *picBuf[3],*pic2Buf,*picbBuf,*tmpBuf;
BYTE *maskBuf[3];

int	bitspixel;
int gpx,gpy;
int gpsx,gpsy;
BOOL	bg_flag=FALSE;

int	winWidth,winHeight;
BOOL	pspc_flag;

LOGFONT	lf;

DWORD timer_step;
DWORD next_tick=0;

PIC_STRUCT *pic,*mask;
MOJI_STRUCT *moji;
GROUP_STRUCT **group;
SCHEDULE_STRUCT *schedule;
BOOL	*loop_flag;

PIC_STRUCT base_pic,moji_pic,back_pic,tmp_pic,draw_pic;
BYTE *work;

int	counter;

BOOL load_skin(HWND,TCHAR *);
TCHAR	fontname[16];

HANDLE mutex;
BOOL	control_flag=FALSE;
BOOL	move_flag=FALSE;
BOOL	hide_flag=FALSE;

TCHAR *skin_fn[32],*skin_txt[32];
int	skin_num,select_skin_num;
						
int transparent=TRANS_100;
int transparent_backup;

TCHAR	Message[256];

int max_pic,max_mask,max_string,*max_group,max_schedule;
int	sch_no;
SYSTEMTIME	before_st;
BOOL	get_background_flag;

/* =====================================================================
	Windowsプログラムのメイン関数
===================================================================== */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					 LPTSTR lpszCmdParm, int nCmdShow)
{
	MSG    msg;

    // ウィンドウの検索
/*
    HWND hWnd = FindWindow(gszAppName, gszAppTitle);
    if (hWnd) {
        // 既に起動しているウィンドウを最前面に移動して終了
        SetForegroundWindow(hWnd);
        return 0;
    }
*/
	ghInst = hInstance;

	if (!hPrevInstance) {					// ほかのインスタンス実行中 ?
		if (!InitApplication(hInstance))	// 共通の初期化処理
			return FALSE;					// 初期化に失敗した場合は終了
	}

	/* アイコンを読み込む */
	hIcon = (HICON)LoadImage( ghInst, MAKEINTRESOURCE(IDI_MAIN_ICON),
					   IMAGE_ICON, 16, 16, 0 ) ;
		
	if (!InitInstance(hInstance, nCmdShow))	// インスタンス毎の初期化処理
		return FALSE;						// 失敗した場合は終了

//	hAccel=LoadAccelerators(ghInst,MAKEINTRESOURCE(IDR_ACCEL));
	while (GetMessage(&msg, NULL, 0, 0)) {	// メッセージの取得とディスパッチ
		if (!TranslateAccelerator(msg.hwnd,hAccel,&msg)) {
			TranslateMessage(&msg);				// 仮想キーコードの変換
			DispatchMessage(&msg);				// メッセージのディスパッチ
		}
	}
	return msg.wParam;		// PostQuitMessage()関数の戻り値を返す
}

/* =====================================================================
	ウィンドウ・クラスの登録
===================================================================== */
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style		 = CS_HREDRAW|CS_VREDRAW;					// クラス・スタイル
	wc.lpfnWndProc	 = MainWndProc;			// ウィンドウ・プロシージャ
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;			// インスタンス・ハンドル
	wc.hIcon		 = hIcon;
	wc.hCursor		 = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	 = NULL;				// メニューの名前
	wc.lpszClassName = gszAppName;			// ウィンドウ・クラスの名前

	return RegisterClass(&wc);	// ウィンドウ・クラスの登録
}


static	BOOL
AddTaskBarIcon( HWND hWnd, HICON hIcon, UINT uID, LPCTSTR lpszTip )
{
	BOOL			res ;
	NOTIFYICONDATA	tnid ;

	tnid.cbSize = sizeof(NOTIFYICONDATA) ;
	tnid.hWnd = hWnd ;
	tnid.uID = uID ;
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
	tnid.uCallbackMessage = WM_NOTIFYICON ;
	tnid.hIcon = hIcon ;
	if ( lpszTip ) {
		_tcscpy( tnid.szTip, lpszTip ) ;
	} else {
		tnid.szTip[0] = '\0' ;
	}
	res = Shell_NotifyIcon( NIM_ADD, &tnid ) ;
	return res ;
}

static	BOOL
DeleteTaskBarIcon( HWND hWnd, UINT uID )
{
	NOTIFYICONDATA	tnid ;

	tnid.cbSize = sizeof (NOTIFYICONDATA) ;
	tnid.hWnd = hWnd ;
	tnid.uID = uID ;
	return Shell_NotifyIcon( NIM_DELETE, &tnid ) ;
}

/* =====================================================================
	ウィンドウの作成と表示
===================================================================== */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	INT	rc;
	get_path();
	mutex=CreateMutex(NULL,TRUE,TEXT("MMascot"));
	rc=GetLastError();
	if (rc==ERROR_ALREADY_EXISTS) {
		HWND hwnd;
		hwnd=load_hwnd();
		if (hwnd!=NULL) {
			SendMessage(hwnd, WM_USER+2, 0, 0x203);
		}
		ReleaseMutex(mutex);
		return FALSE;
	}
	
	ghWnd = CreateWindow(
 		gszAppName,				// 登録されたウィンドウ・クラスの名前
		gszAppTitle,			// タイトル・バーに表示するテキスト
		0,				// ウィンドウ・スタイル
		0,			// ウィンドウの表示位置 (水平)
		0,			//                      (垂直)
		CW_USEDEFAULT,			// ウィンドウの大きさ   (幅)
		CW_USEDEFAULT,			//                      (高さ)
		NULL,					// 親ウィンドウのハンドル
		NULL,					// ウィンドウ・クラスのメニューを使用
		hInstance,				// アプリケーション・インスタンスのハンドル
		NULL					// ウィンドウ作成データのアドレス
	);
	if (!ghWnd)
		return FALSE;			// ウィンドウの作成に失敗
	
//	ShowWindow(ghWnd, nCmdShow);	// ウィンドウ表示状態の設定
	UpdateWindow(ghWnd);			// クライアント領域の更新

	AddTaskBarIcon( ghWnd, hIcon, IDI_MAIN_ICON, TEXT("JZCLOCK") ) ;

	save_hwnd(ghWnd);
	return TRUE;
}

/* =====================================================================
	ウィンドウ・プロシージャ
===================================================================== */

void set_font(BYTE fontsize)
{
	HFONT	hFont;

	lf.lfHeight=fontsize;;
	wsprintf(lf.lfFaceName,TEXT("%s"),fontname);
	hFont = CreateFontIndirect( &lf );
	SelectObject(baseDC, hFont);
    DeleteObject(hFont);
}

void put_moji(TCHAR *str,int fontsize,int x,int y,int color)
{
	RECT	rc;
	int		i,j;
	int		c1,c2;
	c1=c2=-1;
	set_font(fontsize);
	rc.top=0;
	rc.bottom=base_pic.ysize-1;
	rc.left=0;
	rc.right=base_pic.xsize-1;
	DrawText(baseDC, str, -1,&rc, DT_CALCRECT);
	rc.right+=7;
	if (rc.bottom>=base_pic.ysize) rc.bottom=base_pic.ysize-1;
	if (rc.right>=base_pic.xsize) rc.right=base_pic.xsize-1;
	for (j=rc.top;j<=rc.bottom;j++)
		for (i=rc.left;i<=rc.right;i++) 
			set_pixel(&base_pic,i,j,0xffffff);
	DrawText(baseDC, str, -1,&rc, DT_LEFT | DT_TOP );
	for (j=rc.top;j<=rc.bottom;j++)
		for (i=rc.left;i<=rc.right;i++) {
			if (get_pixel(&base_pic,i,j)==0) {
				set_pixel(&tmp_pic,x+i,y+j,color);
				work[(y+j)*gpsx+(x+i)]=1;
			}

		}
}

void get_initial_background()
{
	int i,j;
	
	BitBlt(baseDC, 0,0,gpsx,gpsy, NullDC, gpx,gpy, SRCCOPY);

	for (j=0;j<gpsy;j++)
		for (i=0;i<gpsx;i++) {
			set_pixel(&back_pic,i,j,get_pixel(&base_pic,i,j));
		}
	bg_flag=FALSE;
}
void set_initial_background()
{
	int i,j;
	
	for (j=0;j<gpsy;j++) 
		for (i=0;i<gpsx;i++)
			set_pixel(&base_pic,i,j,get_pixel(&back_pic,i,j));

	BitBlt(NullDC, gpx,gpy,gpsx,gpsy, baseDC, 0,0, SRCCOPY);
}

int random(int n1,int n2)
{
	if (n1==n2) return n1;
	return n1+(Random()%(n2-n1+1));
}

BOOL check_time(int s,SYSTEMTIME *current_time,SYSTEMTIME *before_time)
{
	int day_of_month[12+1]={-1,31,28,31,30,31,30,31,31,30,31,30,31};
	int week  =before_time->wDayOfWeek;
	int month =before_time->wMonth;
	int day   =before_time->wDay;
	int hour  =before_time->wHour;
	int minute=before_time->wMinute;
	int second=before_time->wSecond;

	for (;;) {
		if (schedule[s].week[week]    ==1 && 
			schedule[s].month[month]  ==1 && 
			schedule[s].day[day]      ==1 && 
			schedule[s].hour[hour]    ==1 && 
			schedule[s].second[second]==1 && 
			schedule[s].minute[minute]==1) {
			return TRUE;
		}
		if (current_time->wDayOfWeek==week   &&
			current_time->wMonth    ==month  &&
			current_time->wDay      ==day    &&
			current_time->wHour     ==hour   &&
			current_time->wMinute   ==minute &&
			current_time->wSecond   ==second) {
			return FALSE;
		}

		second++;
		if (second==60) {
			second=0;
			minute++;
		}
		if (minute==60) {
			minute=0;
			hour++;
		}
		if (hour==24) {
			hour=0;
			day++;
		}
		if (day>day_of_month[month]) {
			day=1;
			month++;
			if (month==13) month=1;
			week=(week+1+7)%7;
		}
	}
	return FALSE;
}

int check_next_time(int s,SYSTEMTIME *current_time,int ts)
{
	int day_of_month[12+1]={-1,31,28,31,30,31,30,31,31,30,31,30,31};
	int week  =current_time->wDayOfWeek;
	int month =current_time->wMonth;
	int day   =current_time->wDay;
	int hour  =current_time->wHour;
	int minute=current_time->wMinute;
	int second=current_time->wSecond;
	int msecond=current_time->wMilliseconds;
	int	t;

	for (t=0;t<ts/1000+1;t++) {
		if (schedule[s].week[week]    ==1 && 
			schedule[s].month[month]  ==1 && 
			schedule[s].day[day]      ==1 && 
			schedule[s].hour[hour]    ==1 && 
			schedule[s].second[second]==1 && 
			schedule[s].minute[minute]==1) {
			return t*1000-msecond+500;
		}
		second++;
		if (second==60) {
			second=0;
			minute++;
		}
		if (minute==60) {
			minute=0;
			hour++;
		}
		if (hour==24) {
			hour=0;
			day++;
		}
		if (day>day_of_month[month]) {
			day=1;
			month++;
			if (month==13) month=1;
			week=(week+1+7)%7;
		}
	}
	return -1;
}

BOOL transrate_mojiretsu(TCHAR *t_str,TCHAR *str)
{
	int i,j;
	SYSTEMTIME	st;
	BOOL ret=FALSE;
	
	GetLocalTime(&st);
	j=0;
	for (i=0;;) {
		if (str[i]=='%') {
			if (str[i+1]=='Y') {
				t_str[j++]='0'+((st.wYear/10))%10;
				t_str[j++]='0'+(st.wYear%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='M') {
				t_str[j++]='0'+(st.wMonth/10);
				t_str[j++]='0'+(st.wMonth%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='D') {
				t_str[j++]='0'+(st.wDay/10);
				t_str[j++]='0'+(st.wDay%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='h') {
				t_str[j++]='0'+(st.wHour/10);
				t_str[j++]='0'+(st.wHour%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='m') {
				t_str[j++]='0'+(st.wMinute/10);
				t_str[j++]='0'+(st.wMinute%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='s') {
				t_str[j++]='0'+(st.wSecond/10);
				t_str[j++]='0'+(st.wSecond%10);
				i+=2;
				ret=TRUE;
				continue;
			}
			if (str[i+1]=='%') {
				t_str[j++]='%';
				i+=2;
				continue;
			}
		} else {
			t_str[j++]=str[i++];
		}
		if (str[i-1]=='\0') break;
	}
	return ret;
}

void jzp_timer(HWND hWnd)
{
	int i,j,k;
	int	c;
	int	sch;
	DWORD t;
	DWORD ts;
	SYSTEMTIME st;

	if (hide_flag==TRUE) return;
	t=GetTickCount();

	KillTimer(hWnd,900);
	{	// タイマーイベントが溜まらないようにする
		MSG msg;
		while(::PeekMessage(&msg, hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
	}

	sch=schedule[sch_no].group;
	ts=random(group[sch][counter].interval1,group[sch][counter].interval2);

	for (j=0;j<gpsy;j++) 
		for (i=0;i<gpsx;i++) 
			work[j*gpsx+i]=0;

	for (k=0;;k++) {
		int p,m;
		p=group[sch][counter].pic[k];
		m=group[sch][counter].mask[k];
		if (p==-1 && m==-1) break;
		if (p==-1) {
			TCHAR t_moji[256];
			i=random(0,group[sch][counter].str_num[k]);
			j=group[sch][counter].str[k][i];
			if (transrate_mojiretsu(t_moji,moji[j].str)==TRUE) {
			}
			put_moji(t_moji,moji[j].fontsize,moji[j].x,moji[j].y,moji[j].color);
		} else {
			for (j=0;j<pic[p].ysize;j++)
				for (i=0;i<pic[p].xsize;i++) {
					int c=get_pixel(&(pic[p]),i,j);
					if (pic[p].mask_flag==FALSE) {
						if (get_pixel(&(mask[m]),i,j)==0) {
							set_pixel(&tmp_pic,pic[p].x+i,pic[p].y+j,c);
							work[(pic[p].y+j)*gpsx+(pic[p].x+i)]=1;
						}
					} else {
						if (bitspixel==24) {
							if (c!=pic[p].palette[pic[p].mask_color]) {
								set_pixel(&tmp_pic,pic[p].x+i,pic[p].y+j,c);
								work[(pic[p].y+j)*gpsx+(pic[p].x+i)]=1;
							}
						}
						if (bitspixel==8) {
							if (get_pixel_raw(&(pic[p]),i,j)!=pic[p].mask_color) {
								set_pixel(&tmp_pic,pic[p].x+i,pic[p].y+j,c);
								work[(pic[p].y+j)*gpsx+(pic[p].x+i)]=1;
							}
						}
					}
				}
		}
	}

	if (get_background_flag==TRUE) {
		get_initial_background();
		get_background_flag=FALSE;
	}

	BitBlt(baseDC, 0,0,gpsx,gpsy, NullDC, gpx,gpy, SRCCOPY);

	for (j=0;j<gpsy;j++) {
		for (i=0;i<gpsx;i++) {
			if (bg_flag==TRUE) {
				c=get_pixel(&base_pic,i,j);
				if (get_pixel(&draw_pic,i,j)!=c) 
					set_pixel(&back_pic,i,j,c);
			}
			if (work[j*gpsx+i]==1) {
				set_pixel_transparent(&base_pic,i,j,get_pixel(&tmp_pic,i,j),get_pixel(&back_pic,i,j),transparent);
			} else {
				set_pixel(&base_pic,i,j,get_pixel(&back_pic,i,j));
			}
		}
	}

	BitBlt(NullDC, gpx,gpy,gpsx,gpsy, baseDC, 0,0, SRCCOPY);
	for (j=0;j<gpsy;j++) 
		for (i=0;i<gpsx;i++)
			set_pixel(&draw_pic,i,j,get_pixel(&base_pic,i,j));

	bg_flag=TRUE;
//	InvalidateRect(hWnd,NULL,FALSE);

	GetLocalTime(&st);
	counter++;

	if (group[sch][counter].pic[0]==-1 && group[sch][counter].mask[0]==-1) {
		sch_no=0;
		for (i=0;;i++) {
			if (i>max_schedule) break;
			if (loop_flag[schedule[i].group]==FALSE) continue;
			if (check_time(i,&st,&st)==TRUE) {
				sch_no=i;
			}
		}
		counter=0;	
	}

	if (before_st.wMonth!=0) {
		for (i=0;;i++) {
			if (i>max_schedule) break;
			if (loop_flag[schedule[i].group]==TRUE) continue;
			if (i<=sch_no) continue;
			if (check_time(i,&st,&before_st)==TRUE) {
				sch_no=i;
				counter=0;
			}
		}
		for (i=0;;i++) {
			int next_time;
			if (i>max_schedule) break;
			if (loop_flag[schedule[i].group]==TRUE) continue;
			if (i<=sch_no) continue;
			next_time=check_next_time(i,&st,ts);
			if (next_time>=0 && next_time<=(int)ts) {
				ts=next_time;
				sch_no=i;
				counter=0;
			}
		}
	}
	before_st=st;

	if (next_tick==0)	next_tick=t;
	timer_step=ts+(next_tick-t);
	if (timer_step<10) timer_step=10;
	if (timer_step>10000000) timer_step=100;
	next_tick=t+timer_step;

	SetTimer(hWnd, 900,timer_step,NULL); 
}

void pic_move(int dx,int dy)
{
	if (move_flag==TRUE) return;
	move_flag=TRUE;
	set_initial_background();
	gpx+=dx;
	gpy+=dy;
	get_initial_background();
	jzp_timer(ghWnd);
	move_flag=FALSE;
}

void pic_hide()
{
	KillTimer(ghWnd,900);
	hide_flag=TRUE;
	set_initial_background();
}

void pic_show()
{
	int	i;
	SYSTEMTIME st;
	
	get_initial_background();
	hide_flag=FALSE;

	GetLocalTime(&st);
	sch_no=0;
	for (i=0;;i++) {
		if (i>max_schedule) break;
		if (loop_flag[schedule[i].group]==FALSE) continue;
		if (check_time(i,&st,&st)==TRUE) {
			sch_no=i;
		}
	}
	counter=0;
	timer_step=200;
	next_tick=0;
	get_background_flag=TRUE;
	SetTimer(ghWnd, 900,1,NULL); 
}

BOOL CALLBACK ControlDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	int k,t;

	switch (uMessage) {
	  case WM_INITDIALOG:			// ダイアログ・ボックスの初期化
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_RESETCONTENT, 0, 0 );
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, WM_SETREDRAW, FALSE, 0L );
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_ADDSTRING,0,(LPARAM)TEXT("100%"));
		if (bitspixel==24) {
			SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_ADDSTRING,0,(LPARAM)TEXT("75%"));
			SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_ADDSTRING,0,(LPARAM)TEXT("50%"));
			SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_ADDSTRING,0,(LPARAM)TEXT("25%"));
		}
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_ADDSTRING,0,(LPARAM)TEXT("Half"));
		transparent_backup=transparent;
		if (bitspixel==24) {
			if (transparent==TRANS_100) k=0;
			if (transparent==TRANS_75) k=1;
			if (transparent==TRANS_50) k=2;
			if (transparent==TRANS_25) k=3;
			if (transparent==TRANS_HALF) k=4;
			transparent=TRANS_25;
		} else {
			if (transparent==TRANS_100) k=0;
			if (transparent==TRANS_HALF) k=1;
			transparent=TRANS_HALF;
		}
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, CB_SETCURSEL, k,0);
		SendDlgItemMessage(hDlg,IDC_TRANSPARENT, WM_SETREDRAW, TRUE, 0L );		
		SetFocus(GetDlgItem(hDlg, IDOK));	// OKボタンにフォーカスを設定
		move_flag=FALSE;
		pic_show();
		return FALSE;				// フォーカスを設定した時はFALSEを返す

	  case WM_COMMAND:				// コマンドを受け取った
		switch (wParam) {	
		case IDC_MOVE_UP:		pic_move( 0,-1); break;
		case IDC_MOVE_DOWN:		pic_move( 0,+1); break;
		case IDC_MOVE_LEFT:		pic_move(-1, 0); break;
		case IDC_MOVE_RIGHT:	pic_move(+1, 0); break;
		case IDC_MOVE_UP_10:	pic_move( 0,-10); break;
		case IDC_MOVE_DOWN_10:	pic_move( 0,+10); break;
		case IDC_MOVE_LEFT_10:	pic_move(-10, 0); break;
		case IDC_MOVE_RIGHT_10:	pic_move(+10, 0); break;
		  case IDC_ABOUT:		OnHelp(ghWnd, wParam, lParam);	break;
		  case IDC_QUIT:		OnDestroy(ghWnd, wParam, lParam);	break;
		  case IDC_HIDE:		pic_hide();
		  case IDOK:				// [OK]ボタンが押された
		  case IDCANCEL:			// [閉じる]が選択された
			k=SendDlgItemMessage( hDlg, IDC_TRANSPARENT, CB_GETCURSEL, 0, 0);
			if (bitspixel==24) {
				if (k==0) t=TRANS_100;
				if (k==1) t=TRANS_75;
				if (k==2) t=TRANS_50;
				if (k==3) t=TRANS_25;
				if (k==4) t=TRANS_HALF;
			} else {
				if (k==0) t=TRANS_100;
				if (k==1) t=TRANS_HALF;
			}
			transparent=transparent_backup;
			if (transparent_backup!=t) {
				pic_hide();
				pic_show();
				transparent=t;
			}
			EndDialog(hDlg, TRUE);	// タイアログ・ボックスを閉じる
			control_flag=FALSE;
			return TRUE;
		}
		break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}

void control_clock(HWND hWnd)
{
	HWND h;

	h = CreateWindow(
		TEXT("JZC"),				// 登録されたウィンドウ・クラスの名前
		TEXT("JZC"),			// タイトル・バーに表示するテキスト
		0,				// ウィンドウ・スタイル
		gpx,			// ウィンドウの表示位置 (水平)
		gpy,			//                      (垂直)
		gpsx,			// ウィンドウの大きさ   (幅)
		gpsy,			//                      (高さ)
		ghWnd,		// 親ウィンドウのハンドル
		NULL,					// ウィンドウ・クラスのメニューを使用
		ghInst,				// アプリケーション・インスタンスのハンドル
		NULL					// ウィンドウ作成データのアドレス
	);
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_CONTROL_DIALOG),h, ControlDlgProc);
	if (move_flag==TRUE) {

	}
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMessage,
						 WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
		case WM_TIMER:		jzp_timer(hWnd);		break;

		case WM_CREATE:		OnCreate(hWnd, wParam, lParam);		break;
		case WM_DESTROY:	OnDestroy(hWnd, wParam, lParam);	break;
		case WM_PAINT:		OnPaint(hWnd, wParam, lParam);		break; 
		case WM_HELP:		OnHelp(hWnd, wParam, lParam);		break;
		case WM_NOTIFYICON:	
			if (lParam==0x201) {
				if (hide_flag==TRUE) {
					pic_show();
				} else {
					pic_hide();
				}
			}
			if (lParam==0x203) {
				if (hide_flag==FALSE) {
					pic_hide();
				}
				if (control_flag==FALSE) {
					control_flag=TRUE;
					control_clock(hWnd);
				}
			}
			break;
	}
	return 0;
}

/* =====================================================================
	ウィンドウ作成時の処理
===================================================================== */

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	TCHAR	Message[256];

	GetWindowRect(hWnd, &rc);
	winWidth  = rc.right - rc.left;
	winHeight = rc.bottom - rc.top;
	if (winWidth<=240)	pspc_flag=TRUE;
	else				pspc_flag=FALSE;
	
	sch_no=0;
	counter=0;
	before_st.wMonth=0;
	NullDC=GetDC(NULL);

	bitspixel=GetDeviceCaps(NullDC,BITSPIXEL);
	if (bitspixel==16) bitspixel=24;

	if (load_skin(hWnd,TEXT("skin.txt"))==FALSE) {
		OnDestroy(hWnd, wParam, lParam);
		return;
	}

	tmp_pic.xsize=gpsx;
	tmp_pic.ysize=gpsy;
	tmp_pic.bitspixel=24;
	tmp_pic.buf=(BYTE *)malloc(gpsx*gpsy*3);

	work=(BYTE *)malloc(gpsx*gpsy);

	back_pic.xsize=gpsx;
	back_pic.ysize=gpsy;
	back_pic.bitspixel=24;
	back_pic.buf=(BYTE *)malloc(gpsx*gpsy*3);

	draw_pic.xsize=gpsx;
	draw_pic.ysize=gpsy;
	draw_pic.bitspixel=24;
	draw_pic.buf=(BYTE *)malloc(gpsx*gpsy*3);

// オフスクリーンの作成
	bmi.bmih.biWidth = gpsx;
	bmi.bmih.biHeight = gpsy ;
	bmi.bmih.biSize = sizeof(bmi.bmih) ;
	bmi.bmih.biPlanes = 1 ;
	bmi.bmih.biBitCount = 24; 
	bmi.bmih.biCompression = BI_RGB ;
	bmi.bmih.biSizeImage = 0 ;
	bmi.bmih.biXPelsPerMeter = 0 ; 
	bmi.bmih.biYPelsPerMeter = 0 ;
	bmi.bmih.biClrUsed = 0 ;
	bmi.bmih.biClrImportant = 0 ;
	
	baseBmp = CreateDIBSection( NullDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
							 (void **)&baseBuf, NULL, 0 ) ;
	if ( !baseBmp ) {
		wsprintf( Message, TEXT("baseBmp error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	baseDC = CreateCompatibleDC( NullDC ) ;
	if ( !baseDC ) {
		wsprintf( Message, TEXT("baseDC error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		DeleteObject( baseBmp);
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	SelectObject( baseDC, baseBmp ) ; 

	base_pic.bitspixel=24;
	base_pic.buf=baseBuf;
	base_pic.xsize=gpsx;
	base_pic.ysize=gpsy;

	if (bitspixel==8) {
		bmi.bmih.biWidth = 1;
		bmi.bmih.biHeight = 1 ;
		bmi.bmih.biBitCount = 8; 
	
		tmp256Bmp = CreateDIBSection( NullDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
								 (void **)&tmp256Buf, NULL, 0 ) ;
		if ( !tmp256Bmp ) {
			wsprintf( Message, TEXT("tmp256Bmp error (%d)"),GetLastError() ) ;
			MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
			MessageBeep( MB_ICONASTERISK ) ;
			return  ;
		}
		tmp256DC = CreateCompatibleDC( NullDC ) ;
		if ( !tmp256DC ) {
			wsprintf( Message, TEXT("baseDC error (%d)"),GetLastError() ) ;
			MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
			DeleteObject( tmp256Bmp);
			MessageBeep( MB_ICONASTERISK ) ;
			return  ;
		}
		SelectObject( tmp256DC, baseBmp ) ;		
	}
	get_display_depth(&base_pic);

	get_background_flag=TRUE;
	hide_flag=FALSE;
	next_tick=0;
//	jzp_timer(ghWnd);
	SetTimer(hWnd, 900,1,NULL); 
}

/* =====================================================================
	ウィンドウ破棄時の処理
===================================================================== */

void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	ReleaseMutex(mutex);
	ReleaseMutex(mutex);

	DeleteTaskBarIcon( ghWnd, IDI_MAIN_ICON ) ;
	set_initial_background();
	PostQuitMessage(0); 
} 

/* =====================================================================
	画面の描画
===================================================================== */ 
void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC         hDC;

	SetFocus(hWnd);
	hDC = BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
}
void mouse_down(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}

void mouse_move(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}

void mouse_up(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}
/* =====================================================================
	ヘルプボタンが押された
===================================================================== */
void OnHelp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	OnHelpAbout(hWnd); 
}

/* =====================================================================
	ファイル・メニュー - 終了
===================================================================== */
void OnFileExit(HWND hWnd) 
{
	SendMessage(hWnd, WM_CLOSE, 0, 0L);
}

/* =====================================================================
	ヘルプ・メニュー - バージョン情報の表示
===================================================================== */
void OnHelpAbout(HWND hWnd)
{
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
		hWnd, AboutDlgProc);
}


/* =====================================================================
    バージョン情報の表示ダイアログ
===================================================================== */
BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
	  case WM_INITDIALOG:			// ダイアログ・ボックスの初期化
		SetFocus(GetDlgItem(hDlg, IDOK));	// OKボタンにフォーカスを設定
		return FALSE;				// フォーカスを設定した時はFALSEを返す

	  case WM_COMMAND:				// コマンドを受け取った
		switch (wParam) {
		  case IDOK:				// [OK]ボタンが押された
		  case IDCANCEL:			// [閉じる]が選択された
			EndDialog(hDlg, TRUE);	// タイアログ・ボックスを閉じる
			return TRUE;
		}
		break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}

int get_num(TCHAR *str,int *offset)
{
	int	ret=0;
	int	i=*offset;
	BOOL	flag=FALSE;

	if (i<0) return -1;

	for (;;) {
		if (str[i]!=' ' || str[i]!='\t') break;
		i++;
	}
	for (;;) {
		if ('0'<=str[i] && str[i]<='9') {
			ret=ret*10+(str[i]-'0');
			flag=TRUE;
		} else {
			if (flag==FALSE) ret=-1;
			if (str[i]=='\0') i=-1;
			else	i++;
			break;
		}
		i++;
	}
	*offset=i;
	return ret;
}

BOOL get_file_line(HANDLE h,TCHAR *buf)
{
	DWORD	readsize;
	int		i;
	BYTE	str[1024],c;
	for (i=0;;i++) {
		ReadFile( h, &c, 1, &readsize, NULL ) ;
		if (readsize==0) return FALSE;
		if (c==0x0d && i==0) {
			ReadFile( h, &c, 1, &readsize, NULL ) ;
			if (readsize==0) return FALSE;
			i=0;
			continue;
		}
		if (c==0x0d || readsize==0) {
			ReadFile( h, &c, 1, &readsize, NULL ) ;
			if (readsize==0) return FALSE;
			str[i]='\0';
			MultiByteToWideChar(CP_ACP, 0, (const char *)str, -1, buf, 1000);
			return TRUE;
		} else {
			str[i]=c;
		}
	}
}

BOOL CALLBACK select_skinDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	int		i;

	switch (uMessage) {
		case WM_INITDIALOG:
			for (i=0;i<skin_num;i++) 
				ListBox_AddString(GetDlgItem(hDlg,IDC_SELECT_SKIN), skin_txt[i]);
			ListBox_SetCurSel(GetDlgItem(hDlg,IDC_SELECT_SKIN),0);
			SetFocus(GetDlgItem(hDlg, IDC_SELECT_SKIN));
			return FALSE;
		case WM_COMMAND:		
			switch (LOWORD(wParam)) {
				case IDC_SELECT_SKIN:
					if (HIWORD(wParam)==LBN_DBLCLK) {
						select_skin_num=ListBox_GetCurSel(GetDlgItem(hDlg,IDC_SELECT_SKIN));
						EndDialog(hDlg, TRUE);
						InvalidateRect(GetParent(hDlg),NULL,FALSE);
						return TRUE;
					}
					return FALSE;
				case IDOK:
					select_skin_num=ListBox_GetCurSel(GetDlgItem(hDlg,IDC_SELECT_SKIN));
				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					InvalidateRect(GetParent(hDlg),NULL,FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}

BOOL load_skin(HWND hWnd,TCHAR *filename)
{
	TCHAR fn[256];
	HANDLE hFile;
	int	*a;
	int i,j,k;
	int sch_num;
	int	group_max_num;
	WIN32_FIND_DATA data;

	skin_num=0;
	wsprintf(fn,TEXT("%s*"),path);
	hFile=FindFirstFile(fn,&data);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox( NULL, TEXT("File Error (1)."), TEXT("Error"), MB_OK ) ;
		return FALSE;
	}
	for (;;) {
		TCHAR	fn2[MAX_PATH];
		HANDLE	hFile2;
		if (data.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY) {
			wsprintf(fn2,TEXT("%s%s\\skin.txt"),path,data.cFileName);
			hFile2 = CreateFile( fn2, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
			if (hFile2 != INVALID_HANDLE_VALUE) {
				TCHAR str[2048];
				skin_fn[skin_num]=(TCHAR *)malloc(MAX_PATH);
				skin_txt[skin_num]=(TCHAR *)malloc(sizeof(TCHAR)*80);
				wsprintf(skin_txt[skin_num],TEXT("%s"),fn2);
				for (;;) {
					if (get_file_line(hFile2,str)==FALSE) break;
//					if (str[0]=='Z' || str[0]=='z') break;
					if (str[0]=='T' || str[0]=='t') {
						for (i=0;i<80;i++) {
							if (str[i+2]=='\0') break;
							skin_txt[skin_num][i]=str[i+2];
						}
						skin_txt[skin_num][i]='\0';
					}
				}
				wsprintf(skin_fn[skin_num],TEXT("%s"),fn2);
				wsprintf(str,TEXT("%d : %s (%s)"),skin_num,skin_fn[skin_num],skin_txt[skin_num]);
				skin_num++;
				CloseHandle(hFile2);
			}
		}
		if (FindNextFile(hFile,&data)==NULL) break;  
	}
	FindClose(hFile);
	
	if (skin_num==0) {
		MessageBox( NULL, TEXT("No skin.txt is found."), TEXT("Error"), MB_OK ) ;
		return FALSE;
	}
	if (skin_num==1) {
		select_skin_num=0;
	} else {
		DialogBox(ghInst, MAKEINTRESOURCE(IDD_SELECT_SKIN), ghWnd, select_skinDlgProc);
	}
	wsprintf(path,TEXT("%s"),skin_fn[select_skin_num]);
	j=0;
	for (i=0;;i++) {
		if (path[i]=='\\') j=i;
		if (path[i]=='\0') {
			path[j]='\\';
			path[j+1]='\0';
			break;
		} 
	}
	wsprintf(fn,TEXT("%s"),skin_fn[select_skin_num]);

	for (i=0;i<skin_num;i++) {
		free(skin_fn[i]);
		free(skin_txt[i]);
	}
	
	hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		TCHAR		Message[ 256 ] ;
		wsprintf( Message, TEXT("File Open error %s") ,fn) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return FALSE;
	}
	group_max_num=0;

	for (;;) {
		int num;
		int	sp;
		TCHAR str[2048];
		
		if (get_file_line(hFile,str)==FALSE) break;;
		if (str[0]=='#') {
			sp=1;
			num=get_num(str,&sp);;
			if (group_max_num<num) group_max_num=num;;
		}
	}
	CloseHandle(hFile);

	group=(GROUP_STRUCT **)malloc(sizeof(GROUP_STRUCT *)*(group_max_num+1));
	max_group=(int *)malloc(sizeof(int)*(group_max_num+1));
	loop_flag=(BOOL *)malloc(sizeof(BOOL)*(group_max_num+1));	
	a=(int *)malloc(sizeof(int)*(group_max_num+1));	

	hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		TCHAR		Message[ 256 ] ;
		wsprintf( Message, TEXT("File Open error %s") ,fn) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return FALSE;
	}

	max_pic=0;
	max_mask=0;
	max_string=0;
	for (i=0;i<=group_max_num;i++)
		max_group[i]=0;
	max_schedule=0;

	for (;;) {
		int num;
		int	sp;
		TCHAR str[2048];
		
		if (get_file_line(hFile,str)==FALSE) break;;
		
		if ('a'<=str[0] && str[0]<='z') str[0]-='a'-'A';	
		if (str[0]=='G') {
			sp=1;
			num=get_num(str,&sp);;
			if (max_pic<num) max_pic=num;
		}
		if (str[0]=='M') {
			sp=1;
			num=get_num(str,&sp);;
			if (max_mask<num) max_mask=num;
		}
		if (str[0]=='S') {
			sp=1;
			num=get_num(str,&sp);;
			if (max_string<num) max_string=num;
		}
		if (str[0]=='#') {
			sp=1;
			num=get_num(str,&sp);;
			max_group[num]++;
		}
		if (str[0]=='$') {
			max_schedule++;
		}
	}
	CloseHandle(hFile);

	pic=(PIC_STRUCT *)malloc(sizeof(PIC_STRUCT)*(max_pic+1));	
	mask=(PIC_STRUCT *)malloc(sizeof(PIC_STRUCT)*(max_mask+1));	
	moji=(MOJI_STRUCT *)malloc(sizeof(MOJI_STRUCT)*(max_string+1));	
	for (i=0;i<=group_max_num;i++) {
//		if (max_group[k]==0) continue;
		group[i]=(GROUP_STRUCT *)malloc(sizeof(GROUP_STRUCT)*(max_group[i]+1));	
	}
	schedule=(SCHEDULE_STRUCT *)malloc(sizeof(SCHEDULE_STRUCT)*(max_schedule+1));	

	for (k=0;k<=max_pic;k++) {
		pic[k].buf=NULL;
	}
	for (k=0;k<=max_mask;k++) {
		mask[k].buf=NULL;
	}
	for (k=0;k<=max_string;k++) {
		moji[k].fontsize=-1;
	}
	for (k=0;k<=group_max_num;k++) {
		loop_flag[k]=TRUE;
		a[k]=0;
		if (max_group[k]==0) continue;
		for (j=0;j<=max_group[k];j++) {
			group[k][j].interval1=-1;
			group[k][j].interval2=-1;
			for (i=0;i<10;i++) {
				group[k][j].pic[i]=-1;
				group[k][j].mask[i]=-1;
			}
		}
	}
	schedule[0].group=0;
	for (i=0;i<7;i++)
		schedule[0].week[i]=1;
	for (i=1;i<=12;i++)
		schedule[0].month[i]=1;
	for (i=1;i<=31;i++)
		schedule[0].day[i]=1;
	for (i=0;i<24;i++)
		schedule[0].hour[i]=1;
	for (i=0;i<60;i++)
		schedule[0].minute[i]=1;
	for (i=0;i<60;i++)
		schedule[0].second[i]=1;
	if (max_schedule>0) {
		for (k=1;k<max_schedule;k++) {
			schedule[k].group=-1;
			for (i=0;i<7;i++)
				schedule[k].week[i]=0;
			for (i=1;i<=12;i++)
				schedule[k].month[i]=0;
			for (i=1;i<=31;i++)
				schedule[k].day[i]=0;
			for (i=0;i<24;i++)
				schedule[k].hour[i]=0;
			for (i=0;i<60;i++)
				schedule[k].minute[i]=0;
			for (i=0;i<60;i++)
				schedule[k].second[i]=0;
		}
	}
	
	hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		TCHAR		Message[ 256 ] ;
		wsprintf( Message, TEXT("File Open error %s") ,fn) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		return FALSE;
	}

	sch_num=1;
	for (;;) {
		TCHAR str[2048];
		
		if (get_file_line(hFile,str)==FALSE) break;;
		
		if ('a'<=str[0] && str[0]<='z') str[0]-='a'-'A';	
//		if (str[0]=='Z') break;

		if (str[0]=='D') {
			int sp=1;
			gpsx=get_num(str,&sp);
			gpsy=get_num(str,&sp);
			gpx=winWidth-gpsx-get_num(str,&sp);	
			gpy=winHeight-gpsy-get_num(str,&sp);
		}
		if (str[0]=='F') {
			int sp,i;
			sp=2;
			for (i=0;;i++) {
				if (str[sp]==',') break;
				fontname[i]=str[sp];
				sp++;
			}
			fontname[i]='\0';
		}
		if (str[0]=='G') {
			int num;
			int sp,i;
			sp=1;
			num=get_num(str,&sp);
			for (i=0;;i++) {
				if (str[sp]==',') break;
				pic[num].filename[i]=str[sp];
				sp++;
			}
			pic[num].filename[i]='\0';
			sp++;
			pic[num].x=get_num(str,&sp);
			pic[num].y=get_num(str,&sp);
			pic[num].mask_color=get_num(str,&sp);
			pic[num].mask_flag=(pic[num].mask_color==-1) ? FALSE : TRUE;
			load_core(&pic[num],TRUE);
		}
		if (str[0]=='M') {
			int num;
			int sp,i;
			sp=1;
			num=get_num(str,&sp);
			for (i=0;;i++) {
				if (str[sp]==',') break;
				mask[num].filename[i]=str[sp];
				sp++;
			}
			mask[num].filename[i]='\0';
			sp++;
			mask[num].x=get_num(str,&sp);
			mask[num].y=get_num(str,&sp);
			mask[num].mask_color=get_num(str,&sp);
			if (mask[num].mask_color==-1) mask[num].mask_color=0;
			load_core(&mask[num],FALSE);	
		}
		if (str[0]=='S') {
			int num;
			int sp,i;
			int r,g,b;
			sp=1;
			num=get_num(str,&sp);
			moji[num].fontsize=get_num(str,&sp);
			r=get_num(str,&sp);
			g=get_num(str,&sp);
			b=get_num(str,&sp);
			moji[num].color=(b*256+g)*256+r;
			moji[num].x=get_num(str,&sp);
			moji[num].y=get_num(str,&sp);

			for (i=0;;i++) {
				if (str[sp]=='\0') break;
				moji[num].str[i]=str[sp];
				sp++;
			}
			moji[num].str[i]='\0';
		}
		if (str[0]=='#') {
			int num;
			int sp;
			int i;
			sp=1;
			num=get_num(str,&sp);
			if (str[sp]=='N' || str[sp]=='n') {
				loop_flag[num]=FALSE;
			} else {
				group[num][a[num]].interval1=get_num(str,&sp);
				group[num][a[num]].interval2=group[num][a[num]].interval1;
				if (str[sp-1]==':') {
					group[num][a[num]].interval2=get_num(str,&sp);
				}
				for (i=0;;i++) {
					if (str[sp]=='s' || str[sp]=='S') {
						sp++;
						group[num][a[num]].pic[i]=-1;
						group[num][a[num]].mask[i]=0;
						group[num][a[num]].str_num[i]=0;
						for (;;) {
							group[num][a[num]].str[i][group[num][a[num]].str_num[i]]=get_num(str,&sp);
							if (str[sp-1]!=':') break;
							group[num][a[num]].str_num[i]++;
						}
					} else {
						group[num][a[num]].pic[i]=get_num(str,&sp);
						if (str[sp-1]==':') {
							group[num][a[num]].mask[i]=get_num(str,&sp);
						}
						if (group[num][a[num]].pic[i]==-1) break;
					}
				}
				a[num]++;
			}
		}
		if (str[0]=='$') {
			int sp;
			int i,n1,n2;
			sp=1;
			schedule[sch_num].group=get_num(str,&sp);
			for (;;) {
				// week
				if (str[sp]=='*') {
					n1=0;
					n2=6;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].week[i]=1;
				if (str[sp-1]==' ') break;
			}
			for (;;) {
				// month
				if (str[sp]=='*') {
					n1=1;
					n2=12;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].month[i]=1;
				if (str[sp-1]==' ') break;
			}
			for (;;) {
				// day
				if (str[sp]=='*') {
					n1=1;
					n2=31;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].day[i]=1;
				if (str[sp-1]==' ') break;
			}
			for (;;) {
				// hour
				if (str[sp]=='*') {
					n1=0;
					n2=23;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].hour[i]=1;
				if (str[sp-1]==' ') break;
			}
			for (;;) {
				// minute
				if (str[sp]=='*') {
					n1=0;
					n2=59;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].minute[i]=1;
				if (str[sp-1]==' ') break;
			}
			for (;;) {
				// second
				if (str[sp]=='*') {
					n1=0;
					n2=59;
					sp+=2;
				} else {
					n1=get_num(str,&sp);
					if (n1==-1) break;
					n2=n1;
					if (str[sp-1]=='-') {
						n2=get_num(str,&sp);
					}
				}
				for (i=n1;i<=n2;i++)
					schedule[sch_num].second[i]=1;
			}
			sch_num++;
		}
	}
	CloseHandle(hFile);
/*
	for (i=0;i<max_schedule;i++) {
		TCHAR m[5000];
		wsprintf(m,TEXT("max: %d %d"),max_schedule,(loop_flag[7]==TRUE) ? 0:1);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
		wsprintf(m,TEXT("week: "));
		for (i=0;i<7;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].week[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
		wsprintf(m,TEXT("month: "));
		for (i=1;i<=12;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].month[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
		wsprintf(m,TEXT("day: "));
		for (i=1;i<=31;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].day[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
		wsprintf(m,TEXT("hour: "));
		for (i=0;i<24;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].hour[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
				wsprintf(m,TEXT("minute: "));
		for (i=0;i<60;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].minute[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
			wsprintf(m,TEXT("second: "));
		for (i=0;i<60;i++)
			wsprintf(m,TEXT("%s %d"),m,schedule[7].second[i]);
		MessageBox( NULL, m, TEXT("Error"), MB_OK ) ;
	}
*/
	  return TRUE;
}