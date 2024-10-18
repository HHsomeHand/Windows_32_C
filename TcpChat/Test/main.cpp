#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <stdexcept>
#include <string>

#include "..\CommonFiles\Message.h"
#include "..\CommonFiles\SockWidget.h"
#include "MessageQueue.h"

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

typedef struct _SESSION
{
	WCHAR szUserName[Message::MSG_USER_NAME_LEN];
	DWORD dwMessageId;
	ULONGLONG dwLastTime;
} SESSION;

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

#define WM_SHOWTHREADCOUNT (WM_USER+0x100)

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hWinMain;

DWORD g_isEnd = FALSE;

SOCKET g_listenSock = 0;

DWORD g_dwThreadCount = 0;

CRITICAL_SECTION g_cs = { 0 };

BOOL SendMsgQueue(SOCKET hSocket, SESSION* lpSession)
{
	BOOL isNoWrong = TRUE;
	while (!g_isEnd)
	{
		Message::MSG_STRUCT stDownPackage = { 0 };
		stDownPackage.MsgHead.dwCmdId = Message::CMD_MSG_DOWN;

		BOOL bResult = g_MsgQueue.GetMsg(lpSession->dwMessageId, stDownPackage.MsgDown.szSender, stDownPackage.MsgDown.szContent);
		if (bResult == FALSE)
		{
			break;
		}

		lpSession->dwMessageId++;
		stDownPackage.MsgDown.dwLength = lstrlen(stDownPackage.MsgDown.szContent) + 1;
		stDownPackage.MsgHead.dwLength = sizeof(Message::MSG_HEAD) + 
			Message::MSG_USER_NAME_LEN * sizeof(WCHAR) +
			sizeof(stDownPackage.MsgDown.dwLength) + 
			stDownPackage.MsgDown.dwLength * sizeof(WCHAR);

		int nResult = send(hSocket, (const char*)&stDownPackage, stDownPackage.MsgHead.dwLength, 0);
		if (nResult == SOCKET_ERROR)
		{
			isNoWrong = FALSE;
			break;
		}

		lpSession->dwLastTime = GetTickCount64();
		DWORD dwResult = SockWidget::WaitForRecvData(hSocket, 0);
		if (dwResult == SOCKET_ERROR)
		{
			isNoWrong = FALSE;
			break;
		}
		else if (dwResult > 0)
		{
			break;
		}
	} // while (!g_isEnd)

	return isNoWrong;
}

BOOL LinkCheck(SOCKET hSocket, SESSION* lpSession)
{
	ULONGLONG dwCurrTime = GetTickCount64();
	ULONGLONG dwTimeInterval = dwCurrTime - lpSession->dwLastTime;
	if (dwTimeInterval < 30 * 1000)
	{
		return TRUE;
	}

	Message::MSG_HEAD stCheckPackage;
	stCheckPackage.dwCmdId = Message::CMD_CHECK_LINE;
	stCheckPackage.dwLength = sizeof(Message::MSG_HEAD);

	DWORD dwResult = send(hSocket, (const char*)&stCheckPackage, sizeof(Message::MSG_HEAD), 0);

	if (dwResult == SOCKET_ERROR)
	{
		return FALSE;
	}

	return TRUE;
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

DWORD WINAPI ServiceThreadProc(
	LPVOID lpParameter   // thread data
)
{
	SOCKET clientSock = (SOCKET)lpParameter;

	try {
		EnterCriticalSection(&g_cs);
		g_dwThreadCount++;
		LeaveCriticalSection(&g_cs);

		PostMessage(g_hWinMain, WM_SHOWTHREADCOUNT, 0, 0);

		SESSION session = { 0 };
		session.dwMessageId = g_MsgQueue.GetCurrentId();

		Message::MSG_STRUCT msg = { 0 };

		BOOL bResult = RecvPackage(clientSock, (char*)&msg, sizeof(msg));

		DWORD dwResult = 0;

		if (!bResult)
		{
			throw std::runtime_error("RecvPackage: recv Login package Error");

		}
		else if (msg.MsgHead.dwCmdId != Message::CMD_LOGIN)
		{
			throw std::runtime_error("RecvPackage: Login package error");
		}

		lstrcpy(session.szUserName, msg.Login.szUserName);

		// ==============
		// 发送登录回应报文
		// ==============
		msg.MsgHead.dwCmdId = Message::CMD_LOGIN_RESP;
		msg.MsgHead.dwLength = sizeof(Message::MSG_HEAD) + sizeof(Message::MSG_LOGIN_RESP);

		msg.LoginResp.isSuccessLogin = TRUE;

		dwResult = send(clientSock, (char*)&msg, msg.MsgHead.dwLength, 0);

		if (dwResult == SOCKET_ERROR)
		{
			throw std::runtime_error("send:  Login resp package error");
		}

		session.dwLastTime = GetTickCount64();

		// ==============
		// 加入了聊天室
		// ==============
		{
			std::wstring strJoin(session.szUserName);

			strJoin += L" 加入了聊天室";

			g_MsgQueue.InsertMsg(L"系统消息", strJoin.c_str());
		}


		while (!g_isEnd)
		{
			bResult = SendMsgQueue(clientSock, &session);
			if (!bResult)
			{
				throw std::runtime_error("SendMsgQueue:  error");

			}

			bResult = LinkCheck(clientSock, &session);
			if (!bResult)
			{
				throw std::runtime_error("LinkCheck:  error");
			}

			dwResult = SockWidget::WaitForRecvData(clientSock, 200);
			if (dwResult == SOCKET_ERROR)
			{
				break;
			}
			else if (dwResult == 0)
			{
				continue;
			}

			bResult = RecvPackage(clientSock, (char*)&msg, sizeof(msg));
			if (!bResult)
			{
				break;
			}

			session.dwLastTime = GetTickCount64();
			if (msg.MsgHead.dwCmdId == Message::CMD_MSG_UP)
			{
				g_MsgQueue.InsertMsg(session.szUserName, msg.MsgUp.szContent);
			}
		} // while (!g_isEnd)

		// ==============
		// 退出了聊天室
		// ==============
		{
			std::wstring strLeave(session.szUserName);

			strLeave += L" 离开了聊天室";

			g_MsgQueue.InsertMsg(L"系统消息", strLeave.c_str());
		}
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

	closesocket(clientSock);

	EnterCriticalSection(&g_cs);
	g_dwThreadCount--;
	LeaveCriticalSection(&g_cs);

	PostMessage(g_hWinMain, WM_SHOWTHREADCOUNT, 0, 0);
	return 0;
}

DWORD WINAPI ListenThreadProc(
	LPVOID lpParameter   // thread data
)
{

	try {
		g_listenSock = socket(AF_INET, SOCK_STREAM, 0);
		if (g_listenSock == INVALID_SOCKET)
		{
			throw std::runtime_error("ListenThreadProc: socket fail");
		}

		sockaddr_in sockAddr = { 0 };
		sockAddr.sin_addr.s_addr = INADDR_ANY;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(12345);

		if (bind(g_listenSock, (sockaddr*)&sockAddr, sizeof(sockAddr)) != 0)
		{
			throw std::runtime_error("ListenThreadProc: bind fail");
		}

		listen(g_listenSock, 5);
		SOCKET clientSock = INVALID_SOCKET;
		while ((clientSock = accept(g_listenSock, NULL, 0)) != INVALID_SOCKET)
		{
			CloseHandle(chBEGINTHREADEX(NULL, 0, ServiceThreadProc, clientSock, 0, NULL));
		}
	}
	catch (std::runtime_error err)
	{

	}

	closesocket(g_listenSock);
	return 0;
}

void MainDlg_OnClose(HWND hDlg)
{
	g_isEnd = TRUE;
	closesocket(g_listenSock);
	while (g_dwThreadCount != 0)
	{
	}
	DeleteCriticalSection(&g_cs);
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
	
	InitializeCriticalSection(&g_cs);
	CloseHandle(chBEGINTHREADEX(NULL, 0, ListenThreadProc, 0, 0, NULL));
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

}

INT_PTR CALLBACK DialogProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
)
{
	if (uMsg == WM_SHOWTHREADCOUNT)
	{
		EnterCriticalSection(&g_cs);
		SetDlgItemInt(hDlg, IDC_STATIC_CLIENT_NUN, g_dwThreadCount, FALSE);
		LeaveCriticalSection(&g_cs);
	}

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