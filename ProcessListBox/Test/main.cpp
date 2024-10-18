#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <tlhelp32.h>
#include <stdexcept>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hMainWnd;

void MainDlg_OnClose(HWND hDlg)
{
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

	SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


// return FALSE to stop
typedef BOOL(*PENUM_PROCESS_CALLBACK)(PROCESSENTRY32* lpProcessEntry);

BOOL EnumerateProcesses(PENUM_PROCESS_CALLBACK pfnCallback)
{
	try {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe32 = { 0 };
			pe32.dwSize = sizeof(PROCESSENTRY32);
			BOOL bHasNextProcess = Process32First(hSnapshot, &pe32);
			while (bHasNextProcess)
			{
				if(!pfnCallback(&pe32))
					break;
				bHasNextProcess = Process32Next(hSnapshot, &pe32);
			} // while (isEnd)
		}
		else
		{
			throw std::runtime_error("can't enum process");
		}
		CloseHandle(hSnapshot);
	}
	catch (std::runtime_error err)
	{
		char szBuffer[64] = { 0 };
		wsprintfA(szBuffer, "%s", err.what());
		MessageBoxA(NULL, szBuffer, "DEBUG", MB_OK);
		return FALSE;
	}

	return TRUE;
}

void RefreshListBox()
{
	HWND hListBox = GetDlgItem(g_hMainWnd, IDC_LIST_PROCESS);

	ListBox_ResetContent(hListBox);

	EnumerateProcesses([](PROCESSENTRY32* lpProcessEntry) -> BOOL {
		HWND hListBox = GetDlgItem(g_hMainWnd, IDC_LIST_PROCESS);
		int nIdx = ListBox_AddString(hListBox, lpProcessEntry->szExeFile);
		ListBox_SetItemData(hListBox, nIdx, lpProcessEntry->th32ProcessID);
		return TRUE;
		});
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	g_hMainWnd = hDlg;

	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);
    
	// SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	
	RefreshListBox();

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
	if (id == IDC_BTN_REFRESH)
	{
		RefreshListBox();
	}
	else if (id == IDC_BTN_TERMIN)
	{
		HWND hListBox = GetDlgItem(g_hMainWnd, IDC_LIST_PROCESS);

		int nTerminIdx = ListBox_GetCurSel(hListBox);
		DWORD dwProcessId = ListBox_GetItemData(hListBox, nTerminIdx);
		
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
		hProcess = NULL;

		RefreshListBox();
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