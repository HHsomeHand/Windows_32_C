#pragma once

#include <Windows.h>
#include <functional>
#pragma comment(lib, "ws2_32.lib")
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// 阻塞模式下使用的常用函数
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

namespace SockWidget {
	// pData是数据包头, dwSize是接收到的包头大小
	// 返回值为 TRUE 代表包是完整的, FALSE 代表包是损坏的
	// dwRestLen 为剩余的字节数, RecvPackage会负责接收后面的数据
	typedef std::function<BOOL(_In_ char* pHead, _In_ DWORD dwRecvSize, _Out_ DWORD* pRestLen)> RECV_PACKAGE_CALLBACK;

	int WaitForRecvData(SOCKET hSocket, DWORD dwMsec);
	BOOL RecvData(SOCKET hSocket, char* pData, DWORD dwSize);
	BOOL RecvPackage(SOCKET hSocket, char* pData, DWORD dwHeadSize, RECV_PACKAGE_CALLBACK callback);
}
