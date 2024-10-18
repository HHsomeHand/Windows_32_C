#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hEditHex;

HWND g_hEditDec;

WNDPROC g_oldHexProc;

BOOL isInProcess = FALSE;

// ÷ª‘ –Ì ‰»Îhex
LRESULT CALLBACK EditHexProc(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{


	if (uMsg == WM_CHAR)
	{
		char input = wParam;
		if (isxdigit(input) || input == '\b')
		{
			return CallWindowProc(g_oldHexProc, hwnd, uMsg, toupper(input), lParam);
		}
	}
	else
	{
		return CallWindowProc(g_oldHexProc, hwnd, uMsg, wParam, lParam);
	}


	return 0;
}

void MainDlg_OnClose(HWND hDlg)
{

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

	SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}




BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);

	g_hEditHex = GetDlgItem(hDlg, IDC_EDIT_HEX);
	g_hEditDec = GetDlgItem(hDlg, IDC_EDIT_DEC);

	Edit_LimitText(g_hEditHex, 8);
	Edit_LimitText(g_hEditDec, 10);

	 g_oldHexProc = (WNDPROC)SetWindowLongPtr(g_hEditHex, GWLP_WNDPROC, (LONG_PTR)&EditHexProc);

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
	WCHAR strEditHex[32] = { 0 };
	WCHAR strEditDec[32] = { 0 };
	WCHAR* pEnd = NULL;
	if (!isInProcess) {
		isInProcess = TRUE;
		if (id == IDC_EDIT_HEX)
		{
			// hex to dec
			Edit_GetText(g_hEditHex, strEditHex, _countof(strEditHex));
			int num = wcstol(strEditHex, &pEnd, 16);
			SetDlgItemInt(hDlg, IDC_EDIT_DEC, num, FALSE);
		}
		else if(id == IDC_EDIT_DEC)
		{
			int num = GetDlgItemInt(hDlg, IDC_EDIT_DEC, NULL, FALSE);
			wsprintfW(strEditHex, L"%X", num);
			Edit_SetText(g_hEditHex, strEditHex);
		}
		isInProcess = FALSE;
	} // if (!isInProcess) {
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
	//DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	
	HWND hDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);

	ShowWindow(hDlg, SW_SHOW);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}