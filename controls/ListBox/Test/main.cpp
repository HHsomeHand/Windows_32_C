#include <Windows.h>
#include "resource.h"

HINSTANCE g_hInstance;

INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message  
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconW(g_hInstance, (LPCWSTR)IDI_MAIN));
		SendDlgItemMessageW(hDlg, IDC_LIST_PROJ, LB_ADDSTRING, 0, (LPARAM)L"项目1");
		SendDlgItemMessageW(hDlg, IDC_LIST_PROJ, LB_ADDSTRING, 0, (LPARAM)L"项目2");
		SendDlgItemMessageW(hDlg, IDC_LIST_PROJ, LB_ADDSTRING, 0, (LPARAM)L"项目3");

		SetCurrentDirectoryW(LR"(C:\Users\hwh\Desktop\TMP\Test)");
		SendDlgItemMessageW(hDlg, IDC_LIST_ITEM, LB_DIR, DDL_ARCHIVE | DDL_DIRECTORY , (LPARAM)L"*.*");
		SetFocus(GetDlgItem(hDlg, IDC_BTN_CHECK));
		SendDlgItemMessageW(hDlg, IDC_LIST_ITEM, LB_SETSEL, TRUE, -1);
		return TRUE;
		break;
	case WM_COMMAND:
	{
		int id = LOWORD(wParam);

		if (id == IDC_BTN_RESET)
		{
			SendDlgItemMessageW(hDlg, IDC_LIST_ITEM, LB_SETSEL, FALSE, -1);
		}
		else if (id == IDC_LIST_PROJ)
		{
			int msgType = HIWORD(wParam);
			if (msgType == LBN_SELCHANGE)
			{
				wchar_t szBuffer[64] = { 0 };
				int pos = SendDlgItemMessageW(hDlg, IDC_LIST_PROJ, LB_GETCURSEL, 0, 0);
				SendDlgItemMessageW(hDlg, IDC_LIST_PROJ, LB_GETTEXT, (WPARAM)pos, (LPARAM)szBuffer);
				SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT_SEL), szBuffer);
			}
			else if (msgType == LBN_DBLCLK)
			{
				wchar_t szSelText[64] = { 0 };
				GetDlgItemTextW(hDlg, IDC_TEXT_SEL, szSelText, sizeof(szSelText) / sizeof(wchar_t));
				wchar_t szMsgContent[64] = { 0 };
				wsprintfW(szMsgContent, L"选择结果: %s", szSelText);
				MessageBoxW(hDlg, szMsgContent, L"您的选择", MB_OK);
			}
		} // END of else if (id == IDC_LIST_PROJ)
		else if (id == IDC_BTN_CHECK)
		{
			wchar_t szMsgContent[512] = L"您选择了以下的项目:\r\n";
			wchar_t szBuffer[64] = { 0 };
			DWORD posAry[32] = { 0 };
			HWND hListItem = GetDlgItem(hDlg, IDC_LIST_ITEM);

			int selCount = SendMessage(hListItem, LB_GETSELCOUNT, 0, 0);
			SendMessage(hListItem, LB_GETSELITEMS, sizeof(posAry) / sizeof(DWORD), (LPARAM)posAry);

			for (int i = 0; i < selCount; i++)
			{
				lstrcatW(szMsgContent, L"\r\n");
				SendMessageW(hListItem, LB_GETTEXT, posAry[i], (WPARAM)szBuffer);
				lstrcatW(szMsgContent, szBuffer);
			}

			MessageBoxW(hDlg, szMsgContent, L"您的选择", MB_OK);
		}
	} // END of case WM_COMMAND:
	break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return FALSE;
}



int WINAPI wWinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPWSTR lpwCmdLine,          // command line
	int nCmdShow              // show state
)
{
	g_hInstance = hInstance;
	DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DialogProc, NULL);
	return 0;
}