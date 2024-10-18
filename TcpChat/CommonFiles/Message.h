#pragma once

#include <Windows.h>
namespace Message {
	// 客户端 -> 服务端 登录
	constexpr auto CMD_LOGIN = 0x00000001 ;
	// 服务端 -> 客户端 登录回应
	constexpr auto CMD_LOGIN_RESP = 0x00000081;
	// 客户端 -> 服务端 聊天语句
	constexpr auto CMD_MSG_UP = 0x00000002;
	// 服务端 -> 客户端 聊天语句
	constexpr auto CMD_MSG_DOWN = 0x00000082;
	// 服务端 -> 客户端 聊天语句
	constexpr auto CMD_CHECK_LINE = 0X00000083;


	constexpr int  MSG_USER_NAME_LEN = 12;
	constexpr int  MSG_PASSWORD_LEN = 12;
	constexpr int  MSG_MAX_CONTENT_LEN = 256;

	typedef struct  _MSG_HEAD {
		DWORD dwCmdId;
		DWORD dwLength; // 整个包的大小
	} MSG_HEAD;

	typedef struct _MSG_LOGIN {
		WCHAR szUserName[MSG_USER_NAME_LEN];
		WCHAR szPassword[MSG_PASSWORD_LEN];
	} MSG_LOGIN;

	typedef struct _MSG_LOGIN_RESP {
		BOOL isSuccessLogin; // TRUE 为成功, FALSE 为失败
	} MSG_LOGIN_RESP;

	typedef struct _MSG_UP {
		DWORD dwLength; // 后面内容的长度
		WCHAR szContent[MSG_MAX_CONTENT_LEN]; // 内容, 不等长, 长度由dwLength指定
	} MSG_UP;

	typedef struct _MSG_DOWN {
		WCHAR szSender[MSG_USER_NAME_LEN];
		DWORD dwLength; // 后面内容的长度
		WCHAR szContent[MSG_MAX_CONTENT_LEN]; // 内容, 不等长, 长度由dwLength指定
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
