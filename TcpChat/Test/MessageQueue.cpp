#include "MessageQueue.h"
#include <strsafe.h>

MsgQueue::MsgQueue()
{
    InitializeCriticalSection(&m_cs);
}

MsgQueue::~MsgQueue()
{
    DeleteCriticalSection(&m_cs);
}


void MsgQueue::InsertMsg(_In_ const WCHAR* szSender, _In_ const  WCHAR* szContent) {
    if (queue_.size() == QUEUE_SIZE) {
        queue_.pop_front();  // 队列已满，移除最早的元素
    }
    MSG_QUEUE_ITEM item;

    EnterCriticalSection(&m_cs);
    item.dwMessageId = m_dwCurrentId;
    m_dwCurrentId++;
    LeaveCriticalSection(&m_cs);

    StringCchCopy(item.szSender, Message::MSG_USER_NAME_LEN, szSender);
    StringCchCopy(item.szContent, Message::MSG_MAX_CONTENT_LEN, szContent);

    EnterCriticalSection(&m_cs);
    queue_.push_back(item);
    LeaveCriticalSection(&m_cs);
}

bool MsgQueue::GetMsg(_In_ DWORD dwMessageId, _Out_ WCHAR* szSender, _Out_ WCHAR* szContent)  {
    EnterCriticalSection(&m_cs);

    auto target_iterator = std::find_if(queue_.begin(), queue_.end(),
        [dwMessageId]
        (const MSG_QUEUE_ITEM& value) -> bool
        {
            return value.dwMessageId == dwMessageId;
        });

    LeaveCriticalSection(&m_cs);

    if (target_iterator == queue_.end())
    {
        return false;
    }

    StringCchCopy(szSender, Message::MSG_USER_NAME_LEN, target_iterator->szSender);
    StringCchCopy(szContent, Message::MSG_MAX_CONTENT_LEN, target_iterator->szContent);
    
    return true;
}

MSG_QUEUE_ITEM& MsgQueue::operator[](size_t index) {
    return queue_[index];
}

DWORD MsgQueue::GetCurrentId()
{
    DWORD dwCurrentId = 0;

    EnterCriticalSection(&m_cs);
    dwCurrentId =  m_dwCurrentId;
    LeaveCriticalSection(&m_cs);

    return dwCurrentId;
}

MsgQueue g_MsgQueue;

