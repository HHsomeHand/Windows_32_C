#pragma once

#include <Windows.h>
namespace Message {
	// �ͻ��� -> ����� ��¼
	constexpr auto CMD_LOGIN = 0x00000001 ;
	// ����� -> �ͻ��� ��¼��Ӧ
	constexpr auto CMD_LOGIN_RESP = 0x00000081;
	// �ͻ��� -> ����� �������
	constexpr auto CMD_MSG_UP = 0x00000002;
	// ����� -> �ͻ��� �������
	constexpr auto CMD_MSG_DOWN = 0x00000082;
	// ����� -> �ͻ��� �������
	constexpr auto CMD_CHECK_LINE = 0X00000083;


	constexpr int  MSG_USER_NAME_LEN = 12;
	constexpr int  MSG_PASSWORD_LEN = 12;
	constexpr int  MSG_MAX_CONTENT_LEN = 256;

	typedef struct  _MSG_HEAD {
		DWORD dwCmdId;
		DWORD dwLength; // �������Ĵ�С
	} MSG_HEAD;

	typedef struct _MSG_LOGIN {
		WCHAR szUserName[MSG_USER_NAME_LEN];
		WCHAR szPassword[MSG_PASSWORD_LEN];
	} MSG_LOGIN;

	typedef struct _MSG_LOGIN_RESP {
		BOOL isSuccessLogin; // TRUE Ϊ�ɹ�, FALSE Ϊʧ��
	} MSG_LOGIN_RESP;

	typedef struct _MSG_UP {
		DWORD dwLength; // �������ݵĳ���
		WCHAR szContent[MSG_MAX_CONTENT_LEN]; // ����, ���ȳ�, ������dwLengthָ��
	} MSG_UP;

	typedef struct _MSG_DOWN {
		WCHAR szSender[MSG_USER_NAME_LEN];
		DWORD dwLength; // �������ݵĳ���
		WCHAR szContent[MSG_MAX_CONTENT_LEN]; // ����, ���ȳ�, ������dwLengthָ��
	} MSG_DOWN;

	typedef struct _MSG_STRUCT {
		MSG_HEAD MsgHead;
		union {
			MSG_LOGIN Login;
			MSG_LOGIN_RESP LoginResp;
			MSG_UP MsgUp;
			MSG_DOWN MsgDown;
		};
	} MSG_STRUCT;
};
