// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

int main()
{
    LPWSTR strCommandLine = GetCommandLine();
    int nArgs = 0;
    LPWSTR* pArgList = CommandLineToArgvW(strCommandLine, &nArgs);

    try {
        if (pArgList == NULL)
        {
            throw new std::runtime_error("can't get commandLine");
        }
        else for (int i = 0; i < nArgs; i++)
            printf("%ws\r\n", pArgList[i]);
    }
    catch (std::runtime_error err)
    {
        std::cout << err.what() << std::endl;
    }
    LocalFree(pArgList);

    getchar();
    getchar();
    getchar();
    getchar();
}

