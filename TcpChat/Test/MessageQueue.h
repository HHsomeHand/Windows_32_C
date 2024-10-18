#pragma once

#include <deque>
#include <Windows.h>
#include "../CommonFiles/Message.h"

typedef struct _MSG_QUEUE_ITEM {
    DWORD dwMessageId;
    WCHAR szSender[Message::MSG_USER_NAME_LEN];
    WCHAR szContent[Message::MSG_MAX_CONTENT_LEN];
} MSG_QUEUE_ITEM;

constexpr int  QUEUE_SIZE = 100;

class MsgQueue {
public:
    MsgQueue();
    ~MsgQueue();

    void InsertMsg(_In_ const WCHAR* szSender, _In_ const WCHAR* szContent);

    bool GetMsg(_In_ DWORD dwMessageId, _Out_ WCHAR* szSender, _Out_ WCHAR* szContent) ;

    MSG_QUEUE_ITEM& operator[](size_t index);

    DWORD GetCurrentId();
private:
    std::deque<MSG_QUEUE_ITEM> queue_;
    DWORD m_dwCurrentId = 0;
    CRITICAL_SECTION m_cs;
};

extern MsgQueue g_MsgQueue;
