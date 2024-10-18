#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <process.h>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

BOOL g_isStop = TRUE;
DWORD g_dwCount = 0;
HANDLE g_hEvent;

void MainDlg_OnClose(HWND hDlg)
{
	PostQuitMessage(0);

	CloseHandle(g_hEvent);
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
    
	// SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	EnableWindow(GetDlgItem(hDlg, IDC_BTN_PAUSE), FALSE);
	
	g_hEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
	return TRUE;
}

void MainDlg_OnPaint(HWND hDlg)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);

	EndPaint(hDlg, &ps);
}

void Count(HWND hDlg, int idDlgItem)
{
	DWORD dwCount = 0;
	while (!g_isStop)
	{
		WaitForSingleObject(g_hEvent, INFINITE);
			dwCount++;
			SetDlgItemInt(hDlg, idDlgItem, dwCount, FALSE);
	} // while (!g_isStop)
}


DWORD WINAPI ThreadProc (
	LPVOID lpThreadParameter
	)
{
	Count((HWND)lpThreadParameter, IDC_EDIT_MAIN);

	return 0;
}


void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BTN_COUNT)
	{
		g_isStop = !g_isStop;

		if (g_isStop)
		{
			SetWindowText(hwndCtl, L"计数");
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_PAUSE), FALSE);
		}
		else
		{
			SetEvent(g_hEvent);

			SetWindowText(hwndCtl, L"停止计数");
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_PAUSE), TRUE);

			_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ThreadProc, hDlg, 0, NULL);
		} // if (g_isStop)
	}
	else if (id == IDC_BTN_PAUSE)
	{ 
		if (WaitForSingleObject(g_hEvent, 0) == WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
		else
		{
			SetEvent(g_hEvent);
		}

	} // else if (id == IDC_BTN_PAUSE)
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