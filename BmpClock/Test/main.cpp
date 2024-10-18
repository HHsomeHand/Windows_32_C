#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>

#define  PI 3.1415926

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;
const POINT g_stCenterPos = { 75, 75 }; // Ô²ÐÄ
const int g_nRadius = 75; // °ë¾¶
const int g_nClockSize = 150;
int idTimer;

HANDLE hConsole;

#pragma comment(lib,"msimg32.lib")

void MainDlg_OnTimer(HWND hDlg, UINT id)
{
	InvalidateRect(hDlg, NULL, TRUE);
}

void MainDlg_OnClose(HWND hDlg)
{
	EndDialog(hDlg, 0);
	KillTimer(hDlg, idTimer);
}

void SetWindowCenter(HWND hwnd)
{
	RECT pos;
	GetClientRect(hwnd, &pos);

	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	int height = GetSystemMetrics(SM_CYFULLSCREEN);

	int newLeft = width / 2 - pos.right / 2;
	int newTop = height / 2 - pos.bottom / 2;

	MoveWindow(hwnd, newLeft, newTop, pos.right, pos.bottom, TRUE);
}
HDC g_hDcBack;
int g_idCircle = IDB_CIRCLE1;
int g_idBack = IDB_BACK1;
HCURSOR g_hCursorMove;
HCURSOR g_hCursorGrab;
HMENU g_hPopupMenu;
void RenderBackground(HWND hDlg)
{
	HDC hDcDlg = GetDC(hDlg);
	HBITMAP hBmpCirle = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(g_idCircle));
	HBITMAP hBmpBack = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(g_idBack));
	HBRUSH hBrushBack = CreatePatternBrush(hBmpBack);

	HDC hDcCircle = CreateCompatibleDC(hDcDlg);
	SelectObject(hDcCircle, hBmpCirle);

	RECT rect = { 0 };
	rect.right = g_nClockSize;
	rect.bottom = g_nClockSize;
	FillRect(g_hDcBack, &rect, hBrushBack);

	TransparentBlt(g_hDcBack, 0, 0, g_nClockSize, g_nClockSize, hDcCircle, 0, 0, g_nClockSize, g_nClockSize, 0);

	ReleaseDC(hDlg, hDcDlg);
	DeleteObject(hBmpBack);
	DeleteObject(hBmpCirle);
	DeleteDC(hDcCircle);
	DeleteObject(hBrushBack);
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	g_hPopupMenu = LoadMenuW(g_hInstance, MAKEINTRESOURCEW(IDM_RBUTTON));
	g_hPopupMenu = GetSubMenu(g_hPopupMenu, 0);
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	g_hCursorMove = LoadCursorW(g_hInstance, MAKEINTRESOURCEW(IDC_MOVE));
	g_hCursorGrab = LoadCursorW(g_hInstance, MAKEINTRESOURCEW(IDC_GRAB));
	
	SetClassLongPtr(hDlg, GCLP_HCURSOR, (LONG_PTR)g_hCursorMove);
	SetTimer(hDlg, idTimer, 1000, NULL);

	HDC hdc = GetDC(hDlg);
	g_hDcBack = CreateCompatibleDC(hdc);
	SelectObject(g_hDcBack, CreateCompatibleBitmap(hdc, g_nClockSize, g_nClockSize));
	ReleaseDC(hDlg, hdc);

	SendMessage(hDlg, WM_COMMAND, IDM_BACK1, 0);
	SendMessage(hDlg, WM_COMMAND, IDM_CIRCLE1, 0);

	HRGN hRgn = CreateEllipticRgn(0, 0, g_nClockSize, g_nClockSize);

	SetWindowRgn(hDlg, hRgn, TRUE);

	DeleteObject(hRgn);

	SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	SetWindowCenter(hDlg);
	
	return TRUE;
}

void DrawDotByPos(HDC hdc, int x, int y, int radius)
{
	Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
	
}

void CalcPointPos(LPPOINT lpPoint, int degree, int radius)
{
	degree -= 90;
	double radian = degree * PI / 180;
	lpPoint->x = (int)(cos(radian) * radius) + g_stCenterPos.x;
	lpPoint->y = (int)(sin(radian) * radius) + g_stCenterPos.y;
}

void DrawDotByDegree(HDC hdc, int degree, int radius)
{
	POINT pos;
	CalcPointPos(&pos, degree, g_nRadius - 10);
	DrawDotByPos(hdc, pos.x, pos.y, radius);
}

void DrawDot(HDC hdc, int degreeInc, int radius)
{
	for (int degree = 0; degree <= 360; degree += degreeInc)
	{
		DrawDotByDegree(hdc, degree, radius);
	}
}

void DrawLine(HDC hdc, int degree, int radiusAdjust)
{
	POINT destPos;

	CalcPointPos(&destPos, degree,  g_nRadius - radiusAdjust);

	MoveToEx(hdc, g_stCenterPos.x, g_stCenterPos.y, NULL);
	LineTo(hdc, destPos.x, destPos.y);
}

void ShowTime(HWND hDlg, HDC hdc)
{
	/*HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	DrawDot(hdc, 360 / 12, 3);
	DrawDot(hdc, 360 / 60, 1);
	DeleteObject(SelectObject(hdc, hOldBrush));*/
	

	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);

	time.wHour %= 12;
	HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	DrawLine(hdc, time.wHour * 360 / 12, 35);

	pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	DeleteObject(SelectObject(hdc, pen));
	DrawLine(hdc, time.wMinute * 360 / 60, 20);

	pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	DeleteObject(SelectObject(hdc, pen));
	DrawLine(hdc, time.wSecond * 360 / 60, 15);

	DeleteObject(SelectObject(hdc, oldPen));
}

void MainDlg_OnPaint(HWND hDlg)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);
	int width = ps.rcPaint.right - ps.rcPaint.left;
	int height = ps.rcPaint.bottom - ps.rcPaint.top;
	BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, width, height, g_hDcBack, 0, 0, SRCCOPY);
	ShowTime(hDlg, hdc);
	EndPaint(hDlg, &ps);
}

void MainDlg_OnLButtonDown(
	HWND hDlg,
	BOOL fDoubleClick,
	int x,
	int y,
	UINT keyFlags
)
{
	SetCursor(g_hCursorGrab);
	SendMessageW(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
}

void MainDlg_OnRButtonDown(
	HWND hDlg,
	BOOL fDoubleClick,
	int x,
	int y,
	UINT keyFlags
)
{
	POINT pos = { 0 };
	GetCursorPos(&pos);
	TrackPopupMenu(g_hPopupMenu, TPM_LEFTALIGN, pos.x, pos.y, 0, hDlg, NULL);
}

void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDM_BACK1:
		g_idBack = IDB_BACK1;
		CheckMenuRadioItem(g_hPopupMenu, IDM_BACK1, IDM_BACK2, IDM_BACK1, MF_BYCOMMAND);
		break;
	case IDM_BACK2:
		g_idBack = IDB_BACK2;
		CheckMenuRadioItem(g_hPopupMenu, IDM_BACK1, IDM_BACK2, IDM_BACK2, MF_BYCOMMAND);
		break;
	case IDM_CIRCLE1:
		g_idCircle = IDB_CIRCLE1;
		CheckMenuRadioItem(g_hPopupMenu, IDM_CIRCLE1, IDM_CIRCLE2, IDM_CIRCLE1, MF_BYCOMMAND);
		break;
	case IDM_CIRCLE2:
		g_idCircle = IDB_CIRCLE2;
		CheckMenuRadioItem(g_hPopupMenu, IDM_CIRCLE1, IDM_CIRCLE2, IDM_CIRCLE2, MF_BYCOMMAND);
		break;
	case IDM_EXIT:
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;
	}

	RenderBackground(hDlg);
	InvalidateRect(hDlg, NULL, TRUE);
}

INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message  
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	switch (uMsg)
	{
		chHANDLE_DLGMSG(hDlg, WM_COMMAND, MainDlg_OnCommand);
		chHANDLE_DLGMSG(hDlg, WM_LBUTTONDOWN, MainDlg_OnLButtonDown);
		chHANDLE_DLGMSG(hDlg, WM_RBUTTONDOWN, MainDlg_OnRButtonDown);
		chHANDLE_DLGMSG(hDlg, WM_TIMER, MainDlg_OnTimer);
		chHANDLE_DLGMSG(hDlg, WM_INITDIALOG, MainDlg_OnInitDialog);
		chHANDLE_DLGMSG(hDlg, WM_CLOSE, MainDlg_OnClose);
		chHANDLE_DLGMSG(hDlg, WM_PAINT, MainDlg_OnPaint);
	default:
		return FALSE;
	}
	return TRUE;
}

int WINAPI wWinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPWSTR lpCmdLine,          // command line
	int nCmdShow              // show state
)
{
#if _DEBUG
	{
		AllocConsole();
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
#endif
	g_hInstance = hInstance;
	DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	return 0;
}