#include <windows.h>
#if _DEBUG
HANDLE g_hConsole = 0;
#endif

typedef void* (WINAPI* WinApiPtr)(void);


DWORD GetKernelBase()
{
    const int MODULE_ALIGN = 0x10000;
    //#if _DEBUG
//    {
//        AllocConsole();
//        g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//    }
//#endif
    DWORD dwCurrAddr = 0x80000000;

    while (dwCurrAddr > 0x70000000)
    {
        dwCurrAddr -= MODULE_ALIGN;

        if (dwCurrAddr == 0x770B0000)
        {
            int i = 0;
        }

        __try {

            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)dwCurrAddr;
            if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) // 异常 但没被捕获
            {
                continue;
            }

            PIMAGE_NT_HEADERS32 pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)pDosHeader + pDosHeader->e_lfanew);
            if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
            {
                continue;
            }

            PIMAGE_OPTIONAL_HEADER32 pOpHeader = &pNtHeader->OptionalHeader;

            PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)pOpHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
            if (pExport == 0)
            {
                continue;
            }

            pExport = (PIMAGE_EXPORT_DIRECTORY)(dwCurrAddr + (DWORD)pExport);
            PCHAR pDllName = (PCHAR)(dwCurrAddr + pExport->Name);
//#if _DEBUG
//            {
//                char szBuffer[64] = { 0 };
//                wsprintfA(szBuffer, "%s\r\n", pDllName);
//                WriteConsoleA(g_hConsole, szBuffer, lstrlenA(szBuffer), NULL, NULL);
//            }
//#endif
            char szName[64] = { 0 };
            lstrcpyA(szName, pDllName);
            _strlwr_s(szName, 64);
            if (strcmp((char*)szName, "kernel32.dll") == 0)
            {
                return dwCurrAddr;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            {
//#if _DEBUG
//                {
//                    wchar_t szBuffer[64] = { 0 };
//                    //wsprintfW(szBuffer, L"0x%x Error\r\n", dwCurrAddr);
//                    //WriteConsoleW(g_hConsole, szBuffer, lstrlenW(szBuffer), NULL, NULL);
//                }
//#endif
            }
        }
    }
    return 0;
}

WinApiPtr GetApi(DWORD dwModuleBase, const char* pszApiName)
{
    WinApiPtr dwApiAddr = NULL;
    __try {
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)dwModuleBase;

        PIMAGE_NT_HEADERS32 pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)pDosHeader + pDosHeader->e_lfanew);

        PIMAGE_OPTIONAL_HEADER32 pOpHeader = &pNtHeader->OptionalHeader;

        PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)pOpHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

        pExport = (PIMAGE_EXPORT_DIRECTORY)(dwModuleBase + (DWORD)pExport);

        DWORD dwIdx = 0;

        DWORD* pRvaFnNames = (DWORD*)(dwModuleBase + pExport->AddressOfNames);
        WORD* pOrdinals = (WORD*)(dwModuleBase + pExport->AddressOfNameOrdinals);
        DWORD* pFns = (DWORD*)(dwModuleBase + pExport->AddressOfFunctions);
        while (dwIdx < pExport->NumberOfNames)
        {
            char* pFnName = (char*)(dwModuleBase + pRvaFnNames[dwIdx]);
            if (strcmp(pFnName, pszApiName) == 0)
            {
                WORD wOrdinal = pOrdinals[dwIdx];
                dwApiAddr = (WinApiPtr)(dwModuleBase + pFns[wOrdinal]);
                break;
            }

            dwIdx++;
        } // while (dwIdx < pExport->NumberOfNames)


    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        {
            dwApiAddr = NULL;
        }
    }

    return dwApiAddr;
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{

    WinApiPtr fnGetProcAddress = GetApi(GetKernelBase(), "GetProcAddress");
    WinApiPtr fnLoadLibraryW = GetApi(GetKernelBase(), "LoadLibraryW");
    WinApiPtr fnMessageBoxW = fnGetProcAddress(fnLoadLibraryW(L"User32.dll"), "MessageBoxW");
    fnMessageBoxW(0, L"content", L"title", 0);
    return 0;
}
