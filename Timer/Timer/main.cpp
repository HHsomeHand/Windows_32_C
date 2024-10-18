#include <Windows.h>
#include <windowsx.h>
#include "resource.h"

#define ID_TIMER_CHAGE 1
#define ID_TIMER_BEEP 2

UINT_PTR idTimer;
HWND g_hDlg;
HINSTANCE g_hInstance;

VOID CALLBACK TimerProc(
	HWND hDlg,         // handle to window
	UINT uMsg,         // WM_TIMER message 
	UINT_PTR idEvent,  // timer identifier
	DWORD dwTime       // current system time
)
{
	int time = GetDlgItemInt(g_hDlg, IDC_VALUE_TIME, NULL, TRUE);
	time++;
	SetDlgItemInt(g_hDlg, IDC_VALUE_TIME, time, TRUE);
}

INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message  
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	static HICON hICon1 = LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_ICON1));
	static HICON hICon2 = LoadIconW(g_hInstance, MAKEINTRESOURCEW( IDI_ICON2));
	if (uMsg == WM_TIMER)
	{
		if (wParam == ID_TIMER_CHAGE)
		{
			HICON hTmp = hICon1;
			hICon1 = hICon2;
			hICon2 = hTmp;
			Static_SetIcon(GetDlgItem(hDlg, IDC_ICON), hICon1);
		}
	}
	else if (uMsg == WM_CLOSE)
	{
		KillTimer(hDlg, ID_TIMER_CHAGE);
		KillTimer(hDlg, ID_TIMER_BEEP);
		KillTimer(NULL, idTimer);

		EndDialog(hDlg, 0);
	}
	else if (uMsg == WM_INITDIALOG)
	{
		SetTimer(hDlg, ID_TIMER_CHAGE, 250, NULL);
		SetTimer(hDlg, ID_TIMER_BEEP, 1000, NULL);
		idTimer = SetTimer(hDlg, NULL, 1000, TimerProc);
		Static_SetIcon(GetDlgItem(hDlg, IDC_ICON), hICon1);
		SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hICon1);
		g_hDlg = hDlg;
	}
	else
	{
		
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
	g_hInstance = hInstance;
	DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);

	return 0;
}