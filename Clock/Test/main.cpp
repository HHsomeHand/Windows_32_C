#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>

#define  PI 3.1415926

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;
POINT g_stCenterPos; // Ô²ÐÄ
int g_nRadius; // °ë¾¶
int idTimer;

HANDLE hConsole;


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

	RECT rect;
	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	int height = GetSystemMetrics(SM_CYFULLSCREEN);

	int newLeft = width / 2 - pos.right / 2;
	int newTop = height / 2 - pos.bottom / 2;

	MoveWindow(hwnd, newLeft, newTop, pos.right, pos.bottom, TRUE);
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);

	SetTimer(hDlg, idTimer, 1000, NULL);
	
	return TRUE;
}

void CalcClockPos(HWND hDlg)
{
	RECT pos;
	GetClientRect(hDlg, &pos);
	
	int width = pos.right;
	int height = pos.bottom;

	RtlZeroMemory(&g_stCenterPos, sizeof(g_stCenterPos));
	
	if (width > height)
	{
		g_nRadius = height / 2;
		g_stCenterPos.x = (width - height) / 2;
	}
	else
	{
		g_nRadius = width / 2;
		g_stCenterPos.y = (height - width) / 2;
	}

	g_stCenterPos.x += g_nRadius;
	g_stCenterPos.y += g_nRadius;

	g_nRadius -= 5;
}

void DrawDotByPos(HDC hdc, int x, int y, int radius)
{
	Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
	
}

void CalcPointPos(LPPOINT lpPoint, int degree)
{
	degree -= 90;
	double radian = degree * PI / 180;
	lpPoint->x = (int)(cos(radian) * g_nRadius) + g_stCenterPos.x;
	lpPoint->y = (int)(sin(radian) * g_nRadius) + g_stCenterPos.y;
}

void DrawDotByDegree(HDC hdc, int degree, int radius)
{
	POINT pos;
	CalcPointPos(&pos, degree);
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

	g_nRadius -= radiusAdjust;
	CalcPointPos(&destPos, degree);
	g_nRadius += radiusAdjust;

	MoveToEx(hdc, g_stCenterPos.x, g_stCenterPos.y, NULL);
	LineTo(hdc, destPos.x, destPos.y);
}

void ShowTime(HWND hDlg, HDC hdc)
{
	CalcClockPos(hDlg);
	HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	DrawDot(hdc, 360 / 12, 3);
	DrawDot(hdc, 360 / 60, 1);
	DeleteObject(SelectObject(hdc, hOldBrush));

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
	ShowTime(hDlg, hdc);
	EndPaint(hDlg, &ps);
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