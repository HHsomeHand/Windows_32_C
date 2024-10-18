#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <stdexcept>
#include "../CommonFiles/Message.h"
#include "../CommonFiles/SockWidget.h"

#pragma region chBEGINTHREADEX
#include 	<process.h>
typedef unsigned(__stdcall* PTHREAD_START) (void*);

#define chBEGINTHREADEX(psa, cbStackSize, pfnStartAddr, \
   pvParam, dwCreateFlags, pdwThreadId)                 \
      ((HANDLE)_beginthreadex(                          \
         (void *)        (psa),                         \
         (unsigned)      (cbStackSize),                 \
         (PTHREAD_START) (pfnStartAddr),                \
         (void *)        (pvParam),                     \
         (unsigned)      (dwCreateFlags),               \
         (unsigned *)    (pdwThreadId)))

#pragma endregion

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

constexpr int IP_LEN = 3 * 4 + 3 + 1;

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hWinMain;

char g_szIp[IP_LEN];

WCHAR g_szUserName[Message::MSG_USER_NAME_LEN];

WCHAR g_szPassword[Message::MSG_PASSWORD_LEN];

SOCKET hSocket;

ULONGLONG g_dwLastTIme;

void DlgEditLimitText(int nIdDlgItem, int cchMax)
{
	Edit_LimitText(GetDlgItem(g_hWinMain, nIdDlgItem), cchMax);
}

BOOL EnableDlgItem(int nIdDlgItem, BOOL bEnable)
{
	HWND hCtrl = GetDlgItem(g_hWinMain, nIdDlgItem);
	return EnableWindow(hCtrl, bEnable);
}

BOOL RecvPackage(SOCKET hSocket, char* pBuffer, DWORD dwBufferSize)
{
	return SockWidget::RecvPackage(hSocket, pBuffer, sizeof(Message::MSG_HEAD),
		[dwBufferSize]
		(_In_ char* pHead, _In_ DWORD dwRecvSize, _Out_ DWORD* pRestLen) -> BOOL {
			Message::MSG_HEAD* pMsgHead = (Message::MSG_HEAD*)pHead;
			*pRestLen = pMsgHead->dwLength - sizeof(Message::_MSG_HEAD);

			BOOL isSuccess = TRUE;
			if (pMsgHead->dwLength < sizeof(Message::MSG_HEAD))
			{
				isSuccess = FALSE;
			}
			else if (pMsgHead->dwLength > dwBufferSize)
			{
				isSuccess = FALSE;
			}

			return isSuccess;
		});
}

DWORD WINAPI WorkThreadProc(
	LPVOID lpParameter   // thread data
)
{
	EnableDlgItem(IDC_EDIT_IP, FALSE);
	EnableDlgItem(IDC_EDIT_USERNAME, FALSE);
	EnableDlgItem(IDC_EDIT_PASSWORD, FALSE);
	EnableDlgItem(IDC_BTN_LOGIN, FALSE);

	try {
		sockaddr_in stSin = { 0 };

		stSin.sin_addr.s_addr = inet_addr(g_szIp);
		if (stSin.sin_addr.s_addr == INADDR_NONE)
		{
			MessageBoxW(g_hWinMain, L"IPµØÖ·ÊäÈë´íÎó", L"´íÎó", MB_OK);
			throw std::runtime_error("inet_addr error");
		}

		stSin.sin_family = AF_INET;
		stSin.sin_port = htons(12345);
		
		hSocket = socket(AF_INET, SOCK_STREAM, 0);

		int nResult = connect(hSocket, (sockaddr*) & stSin, sizeof(stSin));
		if (nResult == SOCKET_ERROR)
		{
			MessageBoxW(g_hWinMain, L"ÎÞ·¨Á´½Ó·þÎñÆ÷", L"´íÎó", MB_OK);
			throw std::runtime_error("connect error");
		}

		Message::MSG_STRUCT stMsg;

		stMsg.MsgHead.dwCmdId = Message::CMD_LOGIN;

		stMsg.MsgHead.dwLength = sizeof(Message::MSG_HEAD) + sizeof(Message::MSG_LOGIN);

		lstrcpy(stMsg.Login.szPassword, g_szPassword);

		lstrcpy(stMsg.Login.szUserName, g_szUserName);

		nResult = send(hSocket, (char*)&stMsg, stMsg.MsgHead.dwLength, 0);
		
		if (nResult == SOCKET_ERROR)
		{
			throw std::runtime_error("login send error");
		}

		BOOL bResult = RecvPackage(hSocket, (char*)&stMsg, sizeof(stMsg));
		if (bResult == FALSE)
		{
			throw std::runtime_error("RecvPackage: recv MSG_LOGIN_RESP error");
		}

		if (stMsg.MsgHead.dwCmdId != Message::CMD_LOGIN_RESP
			)
		{
			throw std::runtime_error("RecvPackage: package MSG_LOGIN_RESP error");
		}
		else if (!stMsg.LoginResp.isSuccessLogin)
		{
			MessageBoxW(g_hWinMain, L"ÕËºÅÃÜÂë´íÎó", L"´íÎó", MB_OK);
			throw std::runtime_error("RecvPackage: username or password error");
		}
		EnableDlgItem(IDC_EDIT_SEND, TRUE);
		EnableDlgItem(IDC_BTN_SEND, TRUE);
		EnableDlgItem(IDC_BTN_LOGOUT, TRUE);

		 g_dwLastTIme = GetTickCount64();
		
		while (TRUE)
		{
			ULONGLONG dwCurrTime = GetTickCount64();
			ULONGLONG dwTimeInterval = dwCurrTime - g_dwLastTIme;

			if (dwTimeInterval > 60 * 1000)
			{
				throw std::runtime_error("recv loop timeout");
			}

			nResult =  SockWidget::WaitForRecvData(hSocket, 200);
			if (nResult == SOCKET_ERROR)
			{
				throw std::runtime_error("recv loop WaitForRecvData error");
			}
			else if (nResult == 0)
			{
				continue;
			}

			bResult = RecvPackage(hSocket, (char*)&stMsg, sizeof(stMsg));
			if (!bResult)
			{
				throw std::runtime_error("recv loop RecvPackage error");
			}

			if (stMsg.MsgHead.dwCmdId == Message::CMD_MSG_DOWN)
			{
				std::wstring strMsg(stMsg.MsgDown.szSender);
				strMsg += L" : ";
				strMsg += stMsg.MsgDown.szContent;

 				ListBox_InsertString(GetDlgItem(g_hWinMain, IDC_LIST_MSG), -1, strMsg.c_str());
			}

			g_dwLastTIme = GetTickCount64();
		} // while (TRUE)
	}
	catch (std::runtime_error err)
	{
#if _DEBUG
		{
			char szBuffer[64] = { 0 };
			wsprintfA(szBuffer, "%s\r\n", err.what());
			WriteConsoleA(g_hConsole, szBuffer, lstrlenA(szBuffer), NULL, NULL);
		}
#endif
	}

	closesocket(hSocket);
	EnableDlgItem(IDC_BTN_SEND, FALSE);

	EnableDlgItem(IDC_EDIT_SEND, FALSE);
	EnableDlgItem(IDC_BTN_LOGOUT, FALSE);
	EnableDlgItem(IDC_EDIT_IP, TRUE);
	EnableDlgItem(IDC_EDIT_USERNAME, TRUE);
	EnableDlgItem(IDC_EDIT_PASSWORD, TRUE);
	EnableDlgItem(IDC_BTN_LOGIN, TRUE);
	return 0;
}



void MainDlg_OnClose(HWND hDlg)
{
	WSACleanup();
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
    
	WSADATA wsaData = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		throw std::runtime_error("MainDlg_OnInitDialog: WSAStartup: can't init winsock");
	}

	// SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)));
	EnableDlgItem(IDC_BTN_LOGIN, FALSE);
	EnableDlgItem(IDC_BTN_LOGOUT, FALSE);
	EnableDlgItem(IDC_BTN_SEND, FALSE);
	EnableDlgItem(IDC_EDIT_SEND, FALSE);

	DlgEditLimitText(IDC_EDIT_IP, IP_LEN-1);
	DlgEditLimitText(IDC_EDIT_USERNAME, Message::MSG_USER_NAME_LEN-1);
	DlgEditLimitText(IDC_EDIT_PASSWORD, Message::MSG_PASSWORD_LEN-1);

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
	if (id == IDC_EDIT_IP ||
		id == IDC_EDIT_USERNAME ||
		id == IDC_EDIT_PASSWORD)
	{
		GetDlgItemTextA(hDlg, IDC_EDIT_IP, g_szIp, IP_LEN);
		GetDlgItemTextW(hDlg, IDC_EDIT_USERNAME, g_szUserName, Message::MSG_USER_NAME_LEN);
		GetDlgItemTextW(hDlg, IDC_EDIT_PASSWORD, g_szPassword, Message::MSG_PASSWORD_LEN);

		if (g_szIp[0] != '\0' && g_szUserName[0] != L'\0' && g_szPassword[0] != L'\0')
		{
			EnableDlgItem(IDC_BTN_LOGIN, TRUE);
		}
	}
	else if (id == IDC_BTN_LOGIN)
	{
		CloseHandle(chBEGINTHREADEX(NULL, 0, WorkThreadProc, 0, 0, NULL));
	}
	else if (id == IDC_BTN_LOGOUT)
	{
		closesocket(hSocket);
	}
	else if (id == IDC_BTN_SEND)
	{
		Message::MSG_STRUCT stMsg = { 0 };
		stMsg.MsgHead.dwCmdId = Message::CMD_MSG_UP;
		GetDlgItemTextW(hDlg, IDC_EDIT_SEND, stMsg.MsgUp.szContent, Message::MSG_MAX_CONTENT_LEN);
		stMsg.MsgUp.dwLength = lstrlen(stMsg.MsgUp.szContent) + 1;
		stMsg.MsgHead.dwLength = sizeof(Message::MSG_HEAD) +
			sizeof(stMsg.MsgUp.dwLength) +
			stMsg.MsgUp.dwLength * sizeof(WCHAR);

		int nResult = send(hSocket, (const char*)&stMsg, stMsg.MsgHead.dwLength, 0);
		if (nResult == SOCKET_ERROR)
		{
			closesocket(hSocket);
		}

		SetDlgItemText(hDlg, IDC_EDIT_SEND, NULL);
		g_dwLastTIme += GetTickCount64();
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