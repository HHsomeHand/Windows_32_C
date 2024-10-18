#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))
#define ID_STATUSBAR 0x1
#define IDC_EDIT 0x2
#define ID_TOOLBAR 0x3

int g_arrStatusWidth[] = { 60, 140, 172, -1 };


TBBUTTON g_arrTBButtons[] = 
{
	{
	.iBitmap = STD_CUT,
	.idCommand = IDM_FILE,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_COPY,
	.idCommand = IDM_SAVE,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_PASTE,
	.idCommand = IDM_PAGE,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_UNDO,
	.idCommand = IDM_QUIT,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_REDOW,
	.idCommand = IDM_SEARCH,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_DELETE,
	.idCommand = IDM_REPLACE,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_FILENEW,
	.idCommand = IDM_FONT,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},
{
	.iBitmap = STD_FILEOPEN,
	.idCommand = IDM_COLOR,
	.fsState = TBSTATE_ENABLED,
	.fsStyle = TBSTYLE_BUTTON,
	.bReserved = 0,
	.dwData = 0,
	.iString = (INT_PTR)-1
},

};
HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hStatusBar;

DWORD g_idTimer = 1;

BOOL g_isInsert = TRUE;

HWND g_hToolBar;

void MainDlg_OnClose(HWND hDlg)
{
	KillTimer(hDlg, g_idTimer);
	PostQuitMessage(0);
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
	RECT rcEdit = { 0 };
	GetClientRect(hDlg, &rcEdit);

	RECT rcStatus = { 0 };
	GetClientRect(g_hStatusBar, &rcStatus);

	RECT rcToolBar = { 0 };
	GetClientRect(g_hToolBar, &rcToolBar);

	rcEdit.top += rcToolBar.bottom;
	rcEdit.bottom -= rcStatus.bottom;

	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT);
	MoveWindow(hEdit, 0, rcEdit.top, rcEdit.right, rcEdit.bottom, TRUE);
	MoveWindow(g_hStatusBar, 0, 0, 0, 0, TRUE);
	SendMessageW(g_hToolBar, TB_AUTOSIZE, 0, 0);
}

 BOOL MainDlg_OnCreate(HWND hDlg, LPCREATESTRUCT lpCreateStruct) 
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
	
	g_hToolBar = CreateToolbarEx(hDlg, TBSTYLE_LIST | CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS  | TBSTYLE_FLAT | WS_CHILD | WS_VISIBLE | CCS_TOP, ID_TOOLBAR, 0, HINST_COMMCTRL, IDB_STD_LARGE_COLOR, g_arrTBButtons, _countof(g_arrTBButtons), 0, 0, 0, 0, sizeof(TBBUTTON));
	SendMessage(g_hToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
	DWORD dwStyle2 = (DWORD)SendMessage(g_hToolBar, TB_GETSTYLE, 0, 0);
	SendMessage(g_hToolBar, TB_SETSTYLE, 0, dwStyle2 | TBSTYLE_LIST);
	
	// SendMessage(g_hToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
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

#if _DEBUG
		{
		if (id != IDC_EDIT)
		{
			wchar_t szBuffer[64] = { 0 };
			wsprintfW(szBuffer, L"%d\r\n", id);
			WriteConsoleW(g_hConsole, szBuffer, lstrlenW(szBuffer), NULL, NULL);
		}
		}
#endif	
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


LRESULT CALLBACK DialogProc(
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
		chHANDLE_DLGMSG(hDlg, WM_CREATE, MainDlg_OnCreate);
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
		else if (lpNMHDR->idFrom == ID_TOOLBAR)
		{
			if (lpNMHDR->code == TBN_QUERYDELETE ||
				lpNMHDR->code == TBN_QUERYINSERT)
			{
				return TRUE;
			}
			else if (lpNMHDR->code == TBN_GETBUTTONINFOW)
			{
				TBNOTIFYW* pTB = (TBNOTIFYW*)lpNMHDR;
				if (pTB->iItem < _countof(g_arrTBButtons))
				{
					RtlMoveMemory(&pTB->tbButton, &g_arrTBButtons[pTB->iItem], sizeof(TBBUTTON));

					LoadStringW(g_hInstance, pTB->tbButton.idCommand, pTB->pszText, pTB->cchText);
					
					//if (pTB->iItem == _countof(g_arrTBButtons) - 1)
					//{
					//	return FALSE;
					//}
					return TRUE;
				}
				return FALSE;
			}
		} // else if (lpNMHDR->idFrom == ID_TOOLBAR)
		else if (lpNMHDR->code == TTN_NEEDTEXT)
		{
			TOOLTIPTEXT* lpTipText = (TOOLTIPTEXT*)lParam;
			lpTipText->lpszText = MAKEINTRESOURCE(lpNMHDR->idFrom);
			lpTipText->hinst = g_hInstance;
		}
		break;
	}
	default:
		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	
	}// END of switch (uMsg)
	return FALSE;
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
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };

	wc.lpfnWndProc = DialogProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// Correct.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	return 0;
}