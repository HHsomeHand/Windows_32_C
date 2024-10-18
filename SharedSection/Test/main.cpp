#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <exception>
#include <stdexcept>

#define SAFE_CLOSE_FILE(p) if ((p) != INVALID_HANDLE_VALUE){CloseHandle(p); (p)=NULL;}
#define SAFE_CLOSE_HANDLE(p) if ((p) != NULL){CloseHandle(p); (p)=NULL;}
#define SAFE_UNMAP_VIEW(p) if ((p) != NULL){UnmapViewOfFile(p); (p)=NULL;}

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HANDLE g_hFileMap;
wchar_t* g_pFileView;

const int idTimerCheck = 500;

#pragma data_seg("Shared")
wchar_t g_shared_str[512] ;
#pragma data_seg()

#pragma comment(linker,"/section:Shared,rws")


void MainDlg_OnClose(HWND hDlg)
{
	SAFE_CLOSE_HANDLE(g_hFileMap);
	SAFE_UNMAP_VIEW(g_pFileView);

	KillTimer(hDlg, idTimerCheck);

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

void InitMMF()
{
	try {
		const wchar_t* strMapName = L"3AB509F9-1617-4C0F-9186-1B2D7C223B20";
		g_hFileMap = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, strMapName);
		if (g_hFileMap == NULL)
		{
			g_hFileMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, strMapName);
			if (g_hFileMap == NULL)
			{
				throw std::runtime_error("g_hFileMap ´íÎó");
			}
		} // if (g_hFileMap == NULL)

		g_pFileView = (PWCHAR)MapViewOfFile(g_hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if (g_pFileView == NULL)
		{
			throw std::runtime_error("g_hFileView ´íÎó");
		}
	}
	catch (const std::runtime_error& e)
	{
		MessageBoxA(NULL, e.what(), "´íÎó", MB_OK);

		PostQuitMessage(0);
	};
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);

	SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	
	// InitMMF	();

	SetTimer(hDlg, idTimerCheck, 500, NULL);
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
	if (id == IDC_EDIT_MAIN && codeNotify == EN_CHANGE)
	{
		WCHAR strText[2048] = { 0 };
		Edit_GetText(hwndCtl, strText, _countof(strText));
		// lstrcpyW(g_pFileView, strText);
		 lstrcpyW(g_shared_str, strText);
	}
}

void MainDlg_OnTimer(HWND hDlg, UINT id)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_MAIN);
	if (id == idTimerCheck )
	{
			WCHAR strText[2048] = { 0 };
			
			//lstrcpyW(strText, g_pFileView);
			lstrcpyW(strText, g_shared_str);

			SetDlgItemTextW(hDlg, IDC_STATIC_SHOW, strText);
	} // if (id == idTimerCheck)
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