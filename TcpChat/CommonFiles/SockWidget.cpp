#include "SockWidget.h"
#include <stdexcept>

namespace SockWidget {
	// ����
	//	hSocket ����Ҫ�ȴ���socket���
	//  dwMsec Ҫ�ȴ��ĺ���(1s = 1000ms)
	// ����ֵ
	//	��ʱ����Ϊ 0
	//	�׽��־������� 1
	//	������SOCKET_ERROR
	int WaitForRecvData(SOCKET hSocket, DWORD dwMsec)
	{
		fd_set stFdSet = { 0 };

		stFdSet.fd_count = 1;
		stFdSet.fd_array[0] = hSocket;

		timeval stTimeval = { 0 };
		stTimeval.tv_sec = 0;
		dwMsec *= 1000;
		stTimeval.tv_usec = dwMsec;

		return select(0, &stFdSet, NULL, NULL, &stTimeval);
	}

	// ���չ涨�ֽڵ����ݣ�����������е����ݲ�����ȴ�
	// ����ֵΪ TRUE Ϊ�ɹ�
	// ���Ϊ FALSE ���ǳ�ʱ���ǳ���
	BOOL RecvData(SOCKET hSocket, char* pData, DWORD dwSize)
	{
		try {

			INT64 qwStartTime = GetTickCount64();
			while (TRUE)
			{
				INT64 qwCurrentTime = GetTickCount64();
				INT64 qwTimeIntervel = qwCurrentTime - qwStartTime;

				if (qwTimeIntervel > 10 * 1000)
				{
					throw std::runtime_error("RecvData: wait time out");
				}

				DWORD result = WaitForRecvData(hSocket, 100);

				if (result == 0)
				{
					continue;
				}
				else if (result == SOCKET_ERROR)
				{
					throw std::runtime_error("RecvData: WaitForRecvData socket error");
				}

				int nRecvLen = recv(hSocket, pData, dwSize, 0);

				if (nRecvLen < dwSize)
				{
					dwSize -= nRecvLen;
					pData += nRecvLen;
				}
				else if (nRecvLen == SOCKET_ERROR || nRecvLen == 0)
				{
					throw std::runtime_error("RecvData: recv socket error");
				}
				else
				{
					break;
				}
			} // while (TRUE)

			return TRUE;
		}
		catch (std::runtime_error err)
		{
//#if _DEBUG
//			{
//				char szBuffer[64] = { 0 };
//				wsprintfA(szBuffer, "%s\r\n", err.what());
//				MessageBoxA(NULL, szBuffer, NULL, NULL);
//			}
//#endif
			return FALSE;
		}
	}


	// ����ֵΪ TRUE Ϊ�ɹ�
	// ���Ϊ FALSE ���ǳ�ʱ���ǳ���
	BOOL RecvPackage(SOCKET hSocket, char* pData, DWORD dwHeadSize, RECV_PACKAGE_CALLBACK callback)
	{
		try {
			BOOL isSuccess = RecvData(hSocket, pData, dwHeadSize);
			if (!isSuccess)
			{
				throw std::runtime_error("RecvPackage: RecvData recv head error");
			}

			DWORD dwBodySize = 0;
			isSuccess = callback(pData, dwHeadSize, &dwBodySize);
			if (!isSuccess)
			{
				throw std::runtime_error("RecvPackage: package check error");
			}

			if (dwBodySize != 0)
			{
				pData += dwHeadSize;
				isSuccess = RecvData(hSocket, pData, dwBodySize);
				if (!isSuccess)
				{
					throw std::runtime_error("RecvPackage: RecvData recv body error");
				}
			}
			
			return TRUE;
		}
		catch (std::runtime_error err)
		{
//#if _DEBUG
//			{
//				char szBuffer[64] = { 0 };
//				wsprintfA(szBuffer, "%s\r\n", err.what());
//				MessageBoxA(NULL, szBuffer,  NULL, NULL);
//			}
//#endif
			return FALSE;
		}
	}
}