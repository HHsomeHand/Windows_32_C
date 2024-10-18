#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <stdexcept>
#include "Reg.h"

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hWinMain;

#define SAFE_FREE(p) if ((p) != NULL){free(p); (p)=NULL;}

void EnumKey(LPWSTR lpKey)
{
	SetDlgItemText(g_hWinMain, IDC_EDIT_KEYLIST, NULL);

	HWND hEditKeyList = GetDlgItem(g_hWinMain, IDC_EDIT_KEYLIST);

	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, lpKey, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		return;
	}

	DWORD dwIndex = 0;
	DWORD cbMaxSubKeyLen = 0;
	DWORD cbMaxValueNameLen = 0;
	DWORD cbMaxValueLen = 0;
	RegQueryInfoKey(hKey, NULL, NULL, 0, NULL, &cbMaxSubKeyLen, NULL, NULL, &cbMaxValueNameLen, &cbMaxValueLen, NULL, NULL);
	cbMaxSubKeyLen = (cbMaxSubKeyLen + 1) * sizeof(WCHAR);
	cbMaxValueNameLen = (cbMaxValueNameLen + 1)*sizeof(WCHAR);

	PWCHAR strSubKeyName = NULL;
	PWCHAR strValueName = NULL;
	PBYTE bytesValue = NULL;
	strSubKeyName = (PWCHAR)malloc(cbMaxSubKeyLen);
	while (TRUE)
	{
		DWORD cchSubKeyName = cbMaxSubKeyLen / sizeof(WCHAR);
		if (RegEnumKeyEx(hKey, dwIndex, strSubKeyName, &cchSubKeyName, 0, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
		{
			break;
		}
		Edit_ReplaceSel(hEditKeyList, L"[子键]");
		Edit_ReplaceSel(hEditKeyList, strSubKeyName);
		Edit_ReplaceSel(hEditKeyList, L"\r\n");

		dwIndex++;
	} // while (TRUE)

	strValueName = (PWCHAR)malloc(cbMaxValueNameLen+ 999);
	bytesValue = (PBYTE)malloc(cbMaxValueLen + 999);
	dwIndex = 0;
	while (TRUE)
	{
		DWORD cchValueName = cbMaxValueNameLen;
		DWORD cbValue = cbMaxValueLen;
		DWORD dwType = 0;

		if (RegEnumValue(hKey, dwIndex, strValueName, &cchValueName, 0, &dwType, bytesValue, &cbValue) == ERROR_NO_MORE_ITEMS)
		{
			break;
		}

		Edit_ReplaceSel(hEditKeyList, L"[键值]");
		Edit_ReplaceSel(hEditKeyList, strValueName);
		Edit_ReplaceSel(hEditKeyList, L"=");
		
		if (dwType == REG_SZ)
		{
			Edit_ReplaceSel(hEditKeyList, bytesValue);
			Edit_ReplaceSel(hEditKeyList, L" REG_SZ类型");

		}
		else if (dwType == REG_DWORD)
		{
			WCHAR strBuffer[32] = { 0 };
			wsprintf(strBuffer, L"%08X", *(PDWORD)bytesValue);
			Edit_ReplaceSel(hEditKeyList, strBuffer);

			Edit_ReplaceSel(hEditKeyList, L" REG_DWORD类型");
		}
		else
		{
			Edit_ReplaceSel(hEditKeyList, L" 其他类型");

		}
		Edit_ReplaceSel(hEditKeyList, L"\r\n");

		dwIndex++;
	}
	RegCloseKey(hKey);
	SAFE_FREE(strSubKeyName);
	SAFE_FREE(strValueName);
	SAFE_FREE(bytesValue);
}

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

BOOL MainDlg_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    g_hWinMain = hDlg;
    
	DWORD dwStyle = GetClassLongW(hDlg, GCL_STYLE);
	dwStyle |= CS_HREDRAW;
	dwStyle |= CS_VREDRAW;
	SetClassLongW(hDlg, GCL_STYLE, dwStyle);

	SetWindowCenter(hDlg);
    
	// SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	HWND hComboBox = GetDlgItem(hDlg, IDC_CB_TYPE);
	ComboBox_AddString(hComboBox, L"REG_SZ");
	ComboBox_AddString(hComboBox, L"REG_DWORD");
	
	ComboBox_SetCurSel(hComboBox, 0);

	EnumKey(NULL);
	return TRUE;
}

void MainDlg_OnPaint(HWND hDlg)
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hDlg, &ps);

	EndPaint(hDlg, &ps);
}

PWCHAR AllocAndInitString(HWND hEdit)
{
	PWCHAR str = NULL;
	DWORD len = Edit_GetTextLength(hEdit)+1;

	if (len != 0)
	{
		str = (PWCHAR)malloc(len * sizeof(WCHAR));
		Edit_GetText(hEdit, str, len);
		return str;
	}
	return NULL;
}

void MainDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BTN_CREATESUBKEY ||
		id == IDC_BTN_DELKEY ||
		id == IDC_BTN_READVAL ||
		id == IDC_BTN_SAVEVAL ||
		id == IDC_BTN_DELSUBKEY
		)
	{
		HWND hEditKey = GetDlgItem(hDlg, IDC_EDIT_KEY);
		HWND hEditValueName = GetDlgItem(hDlg, IDC_EDIT_VALUENAME);
		HWND hEditValue = GetDlgItem(hDlg, IDC_EDIT_VALUE);
		HWND hEditSubKey = GetDlgItem(hDlg, IDC_EDIT_SUBKEY);

		PWCHAR strKey = AllocAndInitString(hEditKey);
		PWCHAR strValueName = AllocAndInitString(hEditValueName);
		PWCHAR strValue = AllocAndInitString(hEditValue);
		PWCHAR strSubKey = AllocAndInitString(hEditSubKey);

		if (id == IDC_BTN_CREATESUBKEY)
		{
			_RegCreateSubKey(strKey, strSubKey);
		}
		else if (id == IDC_BTN_DELSUBKEY)
		{
			_RegDelSubKey(strKey, strSubKey);
		}
		else if (id == IDC_BTN_SAVEVAL)
		{
			HWND hCB = GetDlgItem(hDlg, IDC_CB_TYPE);
			INT len = GetWindowTextLength(hCB)+1;
			PWCHAR pszType = (PWCHAR)malloc(len * sizeof(WCHAR));
			GetWindowText(hCB, pszType, len);
			std::wstring strType(pszType);

			if (strType == L"REG_SZ")
			{
				_RegSetValue(strKey, strValueName, REG_SZ, (PBYTE)strValue, (lstrlen(strValue)+1)* sizeof(WCHAR));
			}
			else
			{
				DWORD dwValue = 0;
				dwValue = GetDlgItemInt(hDlg, IDC_EDIT_VALUE, NULL, FALSE);
				_RegSetValue(strKey, strValueName, REG_DWORD, (PBYTE)&dwValue, 4);

			}

			free(pszType);
		}
		else if (id == IDC_BTN_DELVALUE)
		{
			_RegDelValue(strKey, strValueName);
		}
		else if (id == IDC_BTN_READVAL)
		{
			DWORD dwType = 0;
			const DWORD DATA_LEN = 128;
			DWORD cbData = DATA_LEN;
			BYTE bytesData[DATA_LEN] = { 0 };
			_RegQueryValue(strKey, strValueName, &dwType, bytesData, &cbData);
			if (dwType == REG_SZ)
			{
				SetDlgItemText(hDlg, IDC_EDIT_VALUE, (LPWSTR)bytesData);
			}
			else
			{
				SetDlgItemInt(hDlg, IDC_EDIT_VALUE, *(PDWORD)bytesData, FALSE);
			}
		}

		EnumKey(strKey);

		SAFE_FREE(strKey);
		SAFE_FREE(strValueName);
		SAFE_FREE(strValue);
		SAFE_FREE(strSubKey);
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