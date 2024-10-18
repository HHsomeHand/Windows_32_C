#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <wchar.h>


#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

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

	SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
	SetWindowPos(hwnd, NULL, newLeft, newTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);
	
	return TRUE;
}

void MainDlg_OnPaint(HWND hDlg)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);

	EndPaint(hDlg, &ps);
}

// only for ascii
void FormatText(CHAR* pBuffer, DWORD dwSize, HANDLE hOutFile)
{
	CHAR writeBuffer[1024] = { 0 };
	CHAR* pWriteBuffer = writeBuffer;
	for (int i = 0; i < dwSize; i++)
	{
		if (pBuffer[i] == '\n')
		{
			*pWriteBuffer = '\r';
			pWriteBuffer++;
		}
		*pWriteBuffer = pBuffer[i];
		pWriteBuffer++;

		int buflen = pWriteBuffer - writeBuffer;
		if (buflen >= dwSize - 2)
		{
			WriteFile(hOutFile, writeBuffer, buflen, NULL, NULL);
			pWriteBuffer = writeBuffer;
		}
	}
	int buflen = pWriteBuffer - writeBuffer;
	WriteFile(hOutFile, writeBuffer, buflen, NULL, NULL);
	pWriteBuffer = writeBuffer;
}

#define SAFE_CLOSE_FILE(p) if ((p) != INVALID_HANDLE_VALUE){CloseHandle(p); (p)=NULL;}
#define SAFE_CLOSE_HANDLE(p) if ((p) != NULL){CloseHandle(p); (p)=NULL;}
#define SAFE_UNMAP_VIEW(p) if ((p) != NULL){UnmapViewOfFile(p); (p)=NULL;}

void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BTN_BROWER)
	{
		OPENFILENAME opfn = { 0 };
		opfn.lStructSize = sizeof(OPENFILENAME);
		opfn.hwndOwner = hDlg;
		opfn.lpstrFilter = L"文本文件\0*.txt\0\0";

		WCHAR strFilePath[MAX_PATH] = { 0 };
		opfn.lpstrFile = strFilePath;
		opfn.nMaxFile = MAX_PATH;
		opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		GetOpenFileName(&opfn);
		// Edit_SetText(GetDlgItem(hDlg, IDC_EDIT_FILEPATH), strFilePath);
		SetDlgItemTextW(hDlg, IDC_EDIT_FILEPATH, strFilePath);
	}
	else if (id == IDC_EDIT_FILEPATH)
	{
		int length = Edit_GetTextLength(hwndCtl);
		EnableWindow(GetDlgItem(hDlg, IDC_BTN_START), length > 0);
	}
	else if (id == IDC_BTN_START)
	{
		WCHAR* pDot;
		const WCHAR* pAddName;
		int moveIndex;
		HANDLE hOutFile = NULL;
		HANDLE pInMap = NULL;
		char* pFileView = NULL;
		DWORD dwFileSize = 0;

		WCHAR strFilePath[MAX_PATH] = { 0 };

		GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, strFilePath, MAX_PATH);

		HANDLE hInFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE)
		{
			MessageBoxW(hDlg, L"无法打开文件", L"错误", MB_ICONERROR | MB_OK);
			goto EXIT_PROC;
		}

		pInMap = CreateFileMapping(hInFile, NULL, PAGE_READONLY, 0, 0, NULL);

		if (pInMap == NULL)
		{
			MessageBoxW(hDlg, L"无法打开文件", L"错误", MB_ICONERROR | MB_OK);
			goto EXIT_PROC;
		}

		dwFileSize = GetFileSize(hInFile, NULL);
		
		pFileView = (char*)MapViewOfFile(pInMap, FILE_MAP_READ, 0, 0, dwFileSize);
		
		if (pFileView == NULL)
		{
			MessageBoxW(hDlg, L"无法打开文件", L"错误", MB_ICONERROR | MB_OK);
			goto EXIT_PROC;
		}

		// 给文件名strFilePath添加 pAddName 后缀
		pDot = wcsrchr(strFilePath, L'.');

		pAddName = L"_new";

		moveIndex = pDot - strFilePath;
			
		wmemmove(&strFilePath[moveIndex + wcslen(pAddName)], pDot, wcslen(pDot));
		wmemcpy(&strFilePath[moveIndex], pAddName, wcslen(pAddName));
			
			hOutFile = CreateFile(strFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, NULL, NULL);

		if (hOutFile == INVALID_HANDLE_VALUE)
		{
			wchar_t szMsgBuffer[MAX_PATH] = { 0 };
			wsprintfW(szMsgBuffer, L"can't create result file \r\n the file may exist, please delete it\r\n %s", strFilePath);
			MessageBoxW(NULL, szMsgBuffer, L"错误", MB_ICONERROR | MB_OK);
			goto EXIT_PROC;
		}

		
		while (TRUE)
		{
			CHAR strReadBuffer[512] = { 0 };
			DWORD dwBytesRead = 0;
			ReadFile(hInFile, strReadBuffer, _countof(strReadBuffer), &dwBytesRead, NULL);
			if (dwBytesRead == 0)
			{
				break;
			}
			FormatText(pFileView, dwFileSize, hOutFile);
		}
		MessageBoxW(hDlg, L"成功", L"成功", MB_OK);

	EXIT_PROC:
			SAFE_CLOSE_FILE(hInFile);
			SAFE_CLOSE_FILE(hOutFile);
			SAFE_CLOSE_HANDLE(pInMap);
			SAFE_UNMAP_VIEW(pFileView);
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