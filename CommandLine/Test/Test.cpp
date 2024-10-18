// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

int argc()
{
    int dwArgc = 0;
    LPWSTR strCommandLine = GetCommandLine();
    int idx = 0;


    while (strCommandLine[idx] != L'\0')
    {
        if (strCommandLine[idx] != L' ')
        {
            dwArgc++;
            while (TRUE) // 遍历完一个参数
            {
                if (strCommandLine[idx] == L'\0')
                {
                    goto EXIT;
                }
                else if (strCommandLine[idx] == L' ')
                {
                    break;
                }
                else if (strCommandLine[idx] == L'"')
                {
                    idx++;
                    while (strCommandLine[idx] != L'"')
                    {
                        if (strCommandLine[idx] == L'\0')
                        {
                            goto EXIT;
                        }
                        idx++;
                    } // while (strCommandLine[idx] != L'"')
                }
                idx++;
            } //  while (TRUE)
        }
        idx++;
    } // while (strCommandLine[idx] != L'\0')


EXIT:
    return dwArgc;
}

void argv(int argIdx, _Out_ LPWSTR strReturn, DWORD dwSize)
{
    int dwArgc = 0;
    LPWSTR strCommandLine = GetCommandLine();
    int idx = 0;

    BOOL isBeginCopy = FALSE;
    int idxReturn = 0;


    while (strCommandLine[idx] != L'\0')
    {
        if (strCommandLine[idx] != L' ')
        {
            dwArgc++;

            if ((dwArgc-1) == argIdx)
            {
                isBeginCopy = TRUE;
            }

            while (TRUE) // 遍历完一个参数
            {
                if (strCommandLine[idx] == L'\0')
                {
                    goto EXIT;
                }
                else if (strCommandLine[idx] == L' ')
                {
                    break;
                }
                else if (strCommandLine[idx] == L'"')
                {
                    idx++;
                    while (strCommandLine[idx] != L'"')
                    {
                        if (strCommandLine[idx] == L'\0')
                        {
                            goto EXIT;
                        }
                        if (isBeginCopy && idxReturn < dwSize)
                        {
                            strReturn[idxReturn] = strCommandLine[idx];
                            idxReturn++;
                        }
                        idx++;
                    } // while (strCommandLine[idx] != L'"')
                }
                else
                {
                    if (isBeginCopy && idxReturn < dwSize)
                    {
                        strReturn[idxReturn] = strCommandLine[idx];
                        idxReturn++;
                    }
                }
                idx++;
            } //  while (TRUE)

            if (isBeginCopy)
            {
                goto EXIT;
            }
        }
        idx++;
    } // while (strCommandLine[idx] != L'\0')

    
EXIT:
    strReturn[idxReturn] = L'\0';
    return;
}

int main()
{
    std::cout << argc() << std::endl;

    int count = argc();
    const int BUFFER_SIZE = 1024;
    wchar_t strBuffer[BUFFER_SIZE] = { 0 };

    for (int i = 0; i < count; i++)
    {
        argv(i, strBuffer, BUFFER_SIZE);
        _putws(strBuffer);
    }

    getchar();
    getchar();
    getchar();
}


