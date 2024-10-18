#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <Richedit.h>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

#define IDC_RICHEDIT 2000
HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hRichEdit;

HMENU g_hMenu;

WCHAR g_strFind[80];

UINT g_uIDFindMsg;

HWND g_hFindDlg;

HWND g_hDlg;

FINDREPLACE  stFind = {
	sizeof FINDREPLACE,
	NULL,
	NULL,
	FR_DOWN,
	g_strFind,
	NULL,
	_countof(g_strFind),
	0, 
	NULL,
	NULL,
	NULL,
};

void MainDlg_OnClose(HWND hDlg)
{
	// EndDialog(hDlg, 0);
	PostQuitMessage(0);
}

void SetWindowCenter(HWND hwnd)
{
	RECT pos;
	GetClientRect(hwnd, &pos);

	RECT rect;
	int width = GetSystemMetrics(SM_CXFULLSCREEN); // dpi(150%): 1280, dpi(125%): 1536, dpi(100%): 1920
	int height = GetSystemMetrics(SM_CYFULLSCREEN); //dpi(150%): 657, dpi(125%): 801, dpi(100%): 1017
	int newLeft = width / 2 - pos.right / 2;
	int newTop = height / 2 - pos.bottom / 2;

	MoveWindow(hwnd, newLeft, newTop, pos.right, pos.bottom, TRUE);
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	{
		DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
		dwStyle |= CS_HREDRAW;
		dwStyle |= CS_VREDRAW;
		SetClassLongW(hDlg, GCL_STYLE, dwStyle);
	}

	LoadLibraryW(L"RichEd20.dll");

	 g_hRichEdit = CreateWindowW(L"RichEdit20W", NULL, ES_NOHIDESEL | WS_HSCROLL | WS_VSCROLL |ES_WANTRETURN | ES_MULTILINE |WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hDlg, (HMENU)IDC_RICHEDIT, g_hInstance, NULL);

	SendMessageW(g_hRichEdit, EM_SETTEXTMODE, TM_PLAINTEXT, 0);

	SetWindowCenter(hDlg);

	g_hMenu = LoadMenuW(g_hInstance, MAKEINTRESOURCEW( IDM_MAIN));

	SetMenu(hDlg, g_hMenu);

	g_uIDFindMsg = RegisterWindowMessage(FINDMSGSTRING);

	return TRUE;
}

void MainDlg_OnPaint(HWND hDlg)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);

	EndPaint(hDlg, &ps);
}

void _FindText()
{
	FINDTEXTEXW stFindText;
	stFindText.lpstrText = stFind.lpstrFindWhat;

	DWORD dwCharPos = Edit_GetSel(g_hRichEdit);
	stFindText.chrg.cpMin = LOWORD(dwCharPos);
	if (stFind.Flags & FR_DOWN)
	{
		stFindText.chrg.cpMin = HIWORD(dwCharPos);

	}
	
	stFindText.chrg.cpMax = -1;

	DWORD dwFlags = stFind.Flags & (FR_DOWN | FR_MATCHCASE | FR_WHOLEWORD);
	int result = SNDMSG(g_hRichEdit, EM_FINDTEXTEXW, dwFlags, (LPARAM)&stFindText);
	if (result == -1)
	{
		MessageBoxW(g_hFindDlg != NULL ? g_hFindDlg : g_hDlg
			, L"’“≤ªµΩ", NULL, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		Edit_SetSel(g_hRichEdit, stFindText.chrgText.cpMin, stFindText.chrgText.cpMax);
		Edit_ScrollCaret(g_hRichEdit);
	}

}

void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDM_OPEN)
	{

	}
	else if (id == IDM_SAVE)
	{

	}
	else if (id == IDM_EXIT)
	{
		SNDMSG(hDlg, WM_CLOSE, 0, 0);
	}
	else if (id == IDM_UNDO)
	{
		Edit_Undo(g_hRichEdit);
	}
	else if (id == IDM_REDO)
	{
		SNDMSG(g_hRichEdit, EM_REDO, 0, 0);
	}
	else if (id == IDM_SELALL)
	{
		SNDMSG(g_hRichEdit, EM_SETSEL, 0, -1);
	}
	else if (id == IDM_COPY)
	{
		SNDMSG(g_hRichEdit, WM_COPY, 0, -1);
	}
	else if (id == IDM_PASTE)
	{
		SNDMSG(g_hRichEdit, WM_PASTE, 0, -1);
	}
	else if (id == IDM_CUT)
	{
		SNDMSG(g_hRichEdit, WM_CUT, 0, -1);
	}
	else if (id == IDM_FIND)
	{
		stFind.Flags &= ~FR_DIALOGTERM;
		g_hFindDlg = FindText(&stFind);
	}
	else if (id == IDM_FINDNEXT)
	{
		stFind.Flags |= FR_DOWN;
		_FindText();
	}
	else if (id == IDM_FINDPREV)
	{
		stFind.Flags &= ~FR_DOWN;
		_FindText();

	}
 }

void MainDlg_OnSize(HWND hDlg, UINT state, int cx, int cy)
{
	RECT rcClient = { 0 };
	GetClientRect(hDlg, &rcClient);

	MoveWindow(g_hRichEdit, 0, 0, rcClient.right, rcClient.bottom, TRUE);
 }

#pragma region ENABLE_MENU_IF_MACRO

#define ENABLE_MENU_IF(express, id_key) if (express)				 \
{																 \
	EnableMenuItem(g_hMenu, IDM_##id_key, MF_ENABLED);			 \
}																 \
else															 \
{																 \
	EnableMenuItem(g_hMenu, IDM_##id_key, MF_DISABLED);			 \
}

#define ENABLE_MENU_IF_CAN(key) ENABLE_MENU_IF(SNDMSG(g_hRichEdit, EM_CAN##key, 0, 0), key)

#pragma endregion

void SetStatus()
{
	DWORD sel = Edit_GetSel(g_hRichEdit);
	int cpBegin = LOWORD(sel);
	int cpEnd = HIWORD(sel);


	ENABLE_MENU_IF(!(cpBegin == cpEnd), COPY);
	ENABLE_MENU_IF(!(cpBegin == cpEnd), CUT);
	ENABLE_MENU_IF(g_strFind[0] != '\0', FINDNEXT);
	ENABLE_MENU_IF(g_strFind[0] != '\0', FINDPREV);
	ENABLE_MENU_IF_CAN(PASTE);
	ENABLE_MENU_IF_CAN(REDO);
	ENABLE_MENU_IF_CAN(UNDO);

}

void  MainDlg_OnInitMenu(HWND hDlg, HMENU hMenu)
{
	SetStatus();
 }

void MainDlg_OnActivate(HWND hDlg, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	if (state == WA_CLICKACTIVE || state == WA_ACTIVE)
	{
		SetWindowTextW(hDlg, L"[focus]");
		SetFocus(g_hRichEdit);
	}
	else
	{
		SetWindowTextW(hDlg, L"[stop]");
	}
 }



INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message  
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	if (uMsg == g_uIDFindMsg)
	{
		if (stFind.Flags & FR_DIALOGTERM)
		{
			g_hFindDlg = NULL;
		}
		else
		{
			_FindText();
		}
	}

	switch (uMsg)
	{
		chHANDLE_DLGMSG(hDlg, WM_ACTIVATE, MainDlg_OnActivate);
		chHANDLE_DLGMSG(hDlg, WM_SIZE, MainDlg_OnSize);
		chHANDLE_DLGMSG(hDlg, WM_INITMENU, MainDlg_OnInitMenu);
		chHANDLE_DLGMSG(hDlg, WM_COMMAND, MainDlg_OnCommand);
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
		g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
#endif
	g_hInstance = hInstance;
	
	HWND hDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	
	g_hDlg = hDlg;

	stFind.hwndOwner = hDlg;
	stFind.hInstance = hInstance;

	ShowWindow(hDlg, SW_SHOW);
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	
	return 0;
}