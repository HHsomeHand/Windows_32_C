#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))
#define ID_STATUSBAR 0x1
#define IDC_EDIT 0x2

DWORD g_arrStatusWidth[] = { 60, 140, 172, -1 };

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hStatusBar;

DWORD g_idTimer = 1;

BOOL g_isInsert = TRUE;

void MainDlg_OnClose(HWND hDlg)
{
	KillTimer(hDlg, g_idTimer);
	EndDialog(hDlg, 0);
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

void MainDlg_OnTimer(HWND hDlg, UINT id)
{
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	wchar_t szTime[64] = { 0 };
	wsprintfW(szTime, L"%d:%d:%d", time.wHour, time.wMinute, time.wSecond);

	SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)szTime);
}

void MainDlg_OnSize(HWND hDlg, UINT state, int cx, int cy)
{
	RECT rcDlg = { 0 };
	GetClientRect(hDlg, &rcDlg);

	RECT rcStatus = { 0 };
	GetClientRect(g_hStatusBar, &rcStatus);

	rcDlg.bottom -= rcStatus.bottom;

	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT);
	MoveWindow(hEdit, 0, 0, rcDlg.right, rcDlg.bottom, TRUE);
	MoveWindow(g_hStatusBar, 0, 0, 0, 0, TRUE);
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);

	CreateWindowW(L"Edit", NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_WANTRETURN , 0, 0, 0, 0, hDlg, (HMENU)IDC_EDIT, g_hInstance, NULL);

	g_hStatusBar = CreateStatusWindowW(WS_CHILD | WS_VISIBLE |  CCS_BOTTOM |CCS_NOMOVEY, NULL, hDlg, ID_STATUSBAR);
	SendMessage(g_hStatusBar, SB_SETPARTS, 4, (LPARAM)g_arrStatusWidth);
	SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)L"字节数:0");
	SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)L"插入");

	SetTimer(hDlg, g_idTimer, 1000, NULL);
	MainDlg_OnTimer(hDlg, g_idTimer);
	
	RECT rcClient = { 0 };
	GetClientRect(hDlg, &rcClient);
	MainDlg_OnSize(hDlg, 0, rcClient.right, rcClient.bottom);
	return TRUE;
}

void MainDlg_OnPaint(HWND hDlg)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);

	EndPaint(hDlg, &ps);
}

void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_EDIT:
	{
		DWORD dwLen = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_EDIT));
		wchar_t szNote[64] = { 0 };
		wsprintfW(szNote, L"字节数:%d", dwLen);
		SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)szNote);
	}
		break;
	}
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
		chHANDLE_DLGMSG(hDlg, WM_SIZE, MainDlg_OnSize);
		chHANDLE_DLGMSG(hDlg, WM_COMMAND, MainDlg_OnCommand);
		chHANDLE_DLGMSG(hDlg, WM_INITDIALOG, MainDlg_OnInitDialog);
		chHANDLE_DLGMSG(hDlg, WM_CLOSE, MainDlg_OnClose);
		chHANDLE_DLGMSG(hDlg, WM_PAINT, MainDlg_OnPaint);
	case WM_MENUSELECT:
	{
		UINT arrIDMaps[] = { 0, IDM_MENUHELP, 0, 0 };
		MenuHelp(WM_MENUSELECT, wParam, lParam, (HMENU)lParam, g_hInstance, g_hStatusBar, arrIDMaps);
	}
	break;
	case WM_NOTIFY:
	{
		NMHDR* lpNMHDR = (NMHDR*)lParam;
		if (lpNMHDR->idFrom == ID_STATUSBAR)
		{
			if (lpNMHDR->code == NM_CLICK)
			{
				POINT ptCursor = { 0 };
				GetCursorPos(&ptCursor);

				RECT rcDlg = { 0 };
				GetWindowRect(hDlg, &rcDlg);

				ptCursor.x -= rcDlg.left;
				if (ptCursor.x >= g_arrStatusWidth[1] && ptCursor.x <= g_arrStatusWidth[2] + 10)
				{
					g_isInsert = !g_isInsert;
					SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)(g_isInsert ? L"插入" : L"改写"));
				}
			}
		}
		break;
	}
	default:
		return FALSE;
	
	}// END of switch (uMsg)
	return TRUE;
}

int WINAPI wWinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPWSTR lpCmdLine,          // command line
	int nCmdShow              // show state
)
{
	InitCommonControls();
#if _DEBUG
	{
		AllocConsole();
		g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
#endif
	g_hInstance = hInstance;
	DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	return 0;
}