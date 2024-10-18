#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>
#include <stdexcept>
#include <winsock.h>

#define chHANDLE_DLGMSG(hwnd, message, fn) case (message): return (SetDlgMsgResult(hwnd, uMsg, HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

#pragma	comment(lib, "ws2_32.lib")

HINSTANCE g_hInstance;

HANDLE g_hConsole;

HWND g_hWinMain;

DWORD g_dwThreadCount = 0;

CRITICAL_SECTION g_cs = { 0 };

DWORD g_isEnd = FALSE;

SOCKET g_listenSock = 0;


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

DWORD WINAPI ServiceThreadProc(
	LPVOID lpParameter   // thread data
)
{

	SOCKET clientSock = (SOCKET)lpParameter;

	EnterCriticalSection(&g_cs);
	g_dwThreadCount++;
	SetDlgItemInt(g_hWinMain, IDC_STATIC_CONNECTNUM, g_dwThreadCount, FALSE);
	LeaveCriticalSection(&g_cs);



	while (!g_isEnd)
	{
		fd_set stFdSet = { 0 };
		stFdSet.fd_count = 1;
		stFdSet.fd_array[0] = clientSock;

		timeval stTimeval = { 0 };
		stTimeval.tv_sec = 0;
		stTimeval.tv_usec = 200 * 1000;
		DWORD dwResult = select(0, &stFdSet, NULL, NULL, &stTimeval);


		if (dwResult == SOCKET_ERROR)
		{
			break;
		}
		else if (dwResult > 0)
		{
			CHAR buffer[64] = { 0 };
			DWORD len = recv(clientSock, buffer, _countof(buffer), 0);
			if (len == SOCKET_ERROR || len == 0)
			{
				break;
			}
			DWORD dwResult = send(clientSock, buffer, len, 0);
			if (dwResult == SOCKET_ERROR)
			{
				break;
			}
		} // else if(dwResult > 0)

	}


	EnterCriticalSection(&g_cs);
	g_dwThreadCount--;
	LeaveCriticalSection(&g_cs);	
	
	SetDlgItemInt(g_hWinMain, IDC_STATIC_CONNECTNUM, g_dwThreadCount, FALSE);

	return 0;
}

DWORD WINAPI ListenThreadProc(
	LPVOID lpParameter   // thread data
)
{
	
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

	closesocket(g_listenSock);
	return 0;
}

void MainDlg_OnClose(HWND hDlg)
{
	g_isEnd = TRUE;
	closesocket(g_listenSock);
	while (g_dwThreadCount != 0)
	{ }
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
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
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