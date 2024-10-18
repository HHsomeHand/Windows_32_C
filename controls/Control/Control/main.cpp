#include <Windows.h>
#include "resource.h"
#include <windowsx.h>

HINSTANCE g_hInstance;

INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message  
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	static HBITMAP hBitmap1 = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_BITMAP1));
	static HBITMAP hBitmap2 = LoadBitmapW(g_hInstance, MAKEINTRESOURCEW(IDB_BITMAP2));
	DWORD dwNotifyCode = HIWORD(wParam);
	if (uMsg == WM_CLOSE)
	{
		EndDialog(hDlg, 0);
	}
	else if (uMsg == WM_INITDIALOG)
	{
		HICON hIcon = LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN));
		SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		CheckDlgButton(hDlg, IDC_CHECK_ALLOW, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_SHOW, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_RADIO_THICK, BST_CHECKED);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CAPTION), FALSE);

		HWND hComboBox = GetDlgItem(hDlg, IDC_CB_CAPTION);
		SendMessageW(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Hello, World!");
		SendMessageW(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"嘿, 你看到标题栏变了吗?");
		SendMessageW(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"自定义");
		SendMessageW(hComboBox, CB_SETCURSEL, -1, 0);

		SendDlgItemMessage(hDlg, IDC_SCROLL, SBM_SETRANGE, 0, 100);
	}
	else if (uMsg == WM_COMMAND)
	{
		int id = LOWORD(wParam);
		if (id == IDC_BTN_CHANGE)
		{
			HBITMAP hTmp = hBitmap1;
			hBitmap1 = hBitmap2;
			hBitmap2 = hTmp;

			HWND hBitmap = GetDlgItem(hDlg, IDC_BITMAP);
			SendMessageW(hBitmap, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap1);
		}
		else if (id == IDC_BTN_EXIT)
		{
			SendMessageW(hDlg, WM_CLOSE, 0, 0);
		}
		else if (id == IDC_EDIT_CAPTION)
		{
			wchar_t szBuffer[64] = { 0 };
			GetDlgItemTextW(hDlg, IDC_EDIT_CAPTION, szBuffer, _countof(szBuffer));
			SetWindowTextW(hDlg, szBuffer);
		}
		else if (id == IDC_CHECK_SHOW)
		{
			HWND hBitmap = GetDlgItem(hDlg, IDC_BITMAP);
			if (IsWindowVisible(hBitmap))
			{
				ShowWindow(hBitmap, SW_HIDE);
			}
			else
			{
				ShowWindow(hBitmap, SW_SHOW);
			}
		}
		else if (id == IDC_CHECK_ALLOW)
		{
			HWND hBtnChange = GetDlgItem(hDlg, IDC_BTN_CHANGE);

			if (IsDlgButtonChecked(hDlg, IDC_CHECK_ALLOW))
			{
				EnableWindow(hBtnChange, TRUE);
			}
			else
			{
				EnableWindow(hBtnChange, FALSE);

			}
		}
		else if (id == IDC_RADIO_MODAL)
		{
			DWORD dwStyle = GetWindowLong(hDlg, GWL_STYLE);
			dwStyle &= ~WS_THICKFRAME;
			SetWindowLong(hDlg, GWL_STYLE, dwStyle);
		}
		else if (id == IDC_RADIO_THICK)
		{
			DWORD dwStyle = GetWindowLong(hDlg, GWL_STYLE);
			dwStyle |= WS_THICKFRAME;
			SetWindowLong(hDlg, GWL_STYLE, dwStyle);
		}
		else if (id == IDC_CB_CAPTION && dwNotifyCode == CBN_SELENDOK)
		{
			wchar_t szCaption[64] = { 0 };
			HWND hCB = (HWND)lParam;
			ComboBox_GetText(hCB, szCaption, _countof(szCaption));
			if (lstrcmpW(szCaption, L"自定义") == 0)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CAPTION), TRUE);
			}
			else
			{
				SetWindowTextW(hDlg, szCaption);
			}
		}
	} // else if (uMsg == WM_COMMAND)
	else if (uMsg == WM_HSCROLL)
	{
		HWND hWnd = (HWND)lParam;
		DWORD dwMsgCode = LOWORD(wParam);
		DWORD dwPos = SendDlgItemMessageW(hDlg, IDC_SCROLL, SBM_GETPOS, 0, 0);
		if (dwMsgCode == SB_LINELEFT)
		{
			dwPos--;
		}
		else if (dwMsgCode == SB_LINERIGHT)
		{
			dwPos++;
		}
		else if (dwMsgCode == SB_PAGELEFT)
		{
			dwPos -= 10;
		}
		else if (dwMsgCode == SB_PAGERIGHT)
		{
			dwPos += 10;
		}
		else if (dwMsgCode == SB_THUMBPOSITION || 
			dwMsgCode == SB_THUMBTRACK)
		{
			dwPos = HIWORD(wParam);
		}

		if (dwPos < 0)
		{
			dwPos = 0;
		}
		else if (dwPos > 100)
		{
			dwPos = 100;
		}
		SendDlgItemMessageW(hDlg, IDC_SCROLL, SBM_SETPOS, dwPos, TRUE);
		SetDlgItemInt(hDlg, IDC_VALUE_SCROLL, dwPos, FALSE);

		return TRUE;
	}  // else if (uMsg == WM_HSCROLL)

	return FALSE;
}

int WINAPI wWinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPWSTR lpCmdLine,          // command line
	int nCmdShow              // show state
)
{
	g_hInstance = hInstance;
	DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(IDD_MAIN), 
		NULL, DialogProc, NULL);
	return 0;
}