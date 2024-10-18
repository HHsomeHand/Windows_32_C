#pragma once

#include <Windows.h>
#include <functional>
#pragma comment(lib, "ws2_32.lib")
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// ����ģʽ��ʹ�õĳ��ú���
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

namespace SockWidget {
	// pData�����ݰ�ͷ, dwSize�ǽ��յ��İ�ͷ��С
	// ����ֵΪ TRUE �������������, FALSE ��������𻵵�
	// dwRestLen Ϊʣ����ֽ���, RecvPackage�Ḻ����պ��������
	typedef std::function<BOOL(_In_ char* pHead, _In_ DWORD dwRecvSize, _Out_ DWORD* pRestLen)> RECV_PACKAGE_CALLBACK;

	int WaitForRecvData(SOCKET hSocket, DWORD dwMsec);
	BOOL RecvData(SOCKET hSocket, char* pData, DWORD dwSize);
	BOOL RecvPackage(SOCKET hSocket, char* pData, DWORD dwHeadSize, RECV_PACKAGE_CALLBACK callback);
}
