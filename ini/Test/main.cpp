#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <stdexcept>
#include <strsafe.h>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hWinMain;

WCHAR g_strProfilePath[1024] = { 0 };

CONST WCHAR* g_strWndPosSecName = L"WindowPos";

void SetWindowCenter(HWND hwnd)
{
	RECT pos;
	GetClientRect(hwnd, &pos);

	int width = GetSystemMetrics(SM_CXFULLSCREEN);
	int height = GetSystemMetrics(SM_CYFULLSCREEN);

	int newLeft = width / 2 - pos.right / 2;
	int newTop = height / 2 - pos.bottom / 2;

	SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


void SetProfileWindowPos()
{
	int nX = GetPrivateProfileInt(g_strWndPosSecName, L"X", -1, g_strProfilePath);
	int nY = GetPrivateProfileInt(g_strWndPosSecName, L"Y", -1, g_strProfilePath);

	if (nX == -1 || nY == -1)
	{
		SetWindowCenter(g_hWinMain);
	}
	else
	{
		SetWindowPos(g_hWinMain, NULL, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

void SavaProfileWindowPos()
{
	RECT pos;
	GetWindowRect(g_hWinMain, &pos);

	WCHAR strBuffer[64] = { 0 };

	wsprintf(strBuffer, L"%d", pos.left);
	WritePrivateProfileString(g_strWndPosSecName, L"X", strBuffer, g_strProfilePath);

	wsprintf(strBuffer, L"%d", pos.top);
	WritePrivateProfileString(g_strWndPosSecName, L"Y", strBuffer, g_strProfilePath);
}

void EnumProfile()
{
	HANDLE hFile = CreateFile(g_strProfilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("ReadProfile: can't open profile");
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);

	PBYTE pContext = (PBYTE)malloc(dwFileSize);

	DWORD dwNumOfBytesRead = 0;

	ReadFile(hFile, pContext, dwFileSize, &dwNumOfBytesRead, NULL);

	if (dwNumOfBytesRead != dwFileSize)
	{
		throw std::runtime_error("ReadProfile: can't read profile");

	}

	SetDlgItemTextA(g_hWinMain, IDC_EDIT_INI, (LPCSTR)pContext);

	free(pContext);
}

//void EnumProfile()
//{
//	SetDlgItemText(g_hWinMain, IDC_EDIT_INI, NULL);
//
//	
//	WCHAR arrSecName[8 * 1024] = { 0 };
//	GetPrivateProfileSectionNames(arrSecName, _countof(arrSecName), g_strProfilePath);
//
//	PWCHAR pSecName = arrSecName;
//	while (*pSecName != L'\0')
//	{
//		WCHAR strBuffer[128] = { 0 };
//		wsprintf(strBuffer, L"[%s]\r\n", pSecName);
//		SendDlgItemMessage(g_hWinMain, IDC_EDIT_INI, EM_REPLACESEL, FALSE, (LPARAM)strBuffer);
//
//		WCHAR arrSecStr[8 * 1024] = { 0 };
//		GetPrivateProfileSection(pSecName, arrSecStr, _countof(arrSecStr), g_strProfilePath);
//		
//		PWCHAR pSecStr = arrSecStr;
//		while (*pSecStr != L'\0')
//		{
//			wsprintf(strBuffer, L"%s\r\n", pSecStr);
//			SendDlgItemMessage(g_hWinMain, IDC_EDIT_INI, EM_REPLACESEL, FALSE, (LPARAM)strBuffer);
//
//			pSecStr += lstrlen(pSecStr) + 1;
//		} // while (*pSecStr != L'\0')
//
//		pSecName += lstrlen(pSecName) + 1;
//	} // while (*pSecName != L'\0')
//}

void MainDlg_OnClose(HWND hDlg)
{
	SavaProfileWindowPos();

	PostQuitMessage(0);
}

void SetProfilePath()
{
	DWORD dwCurrDirLen = GetCurrentDirectory(_countof(g_strProfilePath), g_strProfilePath);

	if (dwCurrDirLen == 0)
	{
		throw std::runtime_error("g_strProfilePath is too small \r\nGetCurrentDirectory return 0");
	}

	if (g_strProfilePath[dwCurrDirLen - 1] != L'\\')
	{
		StringCchCat(g_strProfilePath, _countof(g_strProfilePath), L"\\");
	}

	StringCchCat(g_strProfilePath, _countof(g_strProfilePath), L"Option.ini");
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	g_hWinMain = hDlg;

	SetProfilePath();

	SetProfileWindowPos();

	EnumProfile();

	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	// SetWindowCenter(hDlg);
    
	// SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	
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
	if (id >= IDC_BTN_DELSEC && id <= IDC_BTN_GETKEY)
	{
		WCHAR strSec[128] = { 0 };
		WCHAR strKey[128] = { 0 };
		WCHAR strVal[128] = { 0 };

		GetDlgItemText(hDlg, IDC_EDIT_SECTION, strSec, _countof(strSec));
		GetDlgItemText(hDlg, IDC_EDIT_KEY, strKey, _countof(strKey));
		GetDlgItemText(hDlg, IDC_EDIT_VALUE, strVal, _countof(strVal));

		if (id == IDC_BTN_DELSEC)
		{
			WritePrivateProfileString(strSec, NULL, NULL, g_strProfilePath);
		}
		else if (id == IDC_BTN_DELKEY)
		{
			WritePrivateProfileString(strSec, strKey, NULL, g_strProfilePath);
		}
		else if (id == IDC_BTN_SETKEY)
		{
			WritePrivateProfileString(strSec, strKey, strVal, g_strProfilePath);
		}
		else if (id == IDC_BTN_GETKEY)
		{

			GetPrivateProfileString(strSec, strKey, NULL, strVal, sizeof(strVal), g_strProfilePath);
			SetDlgItemText(hDlg, IDC_EDIT_VALUE, strVal);
		}

		EnumProfile();
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
	try {
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
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "error", MB_OK);
	}
	return 0;
}