#include <Windows.h>
#include <stdexcept>

#define SAFE_CLOSE_HANDLE(p) if ((p) != NULL){CloseHandle(p); (p)=NULL;}

#define UPX_BEGIN_ADDR (0x405120)
#define REAL_OEP (0x00401000)
#define PATCH_ADDR (0x401004)
void hzxWriteProcessMemory(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize)
{
    DWORD dwNewProtect = PAGE_READWRITE;
    DWORD dwOldProtect = 0;
    VirtualProtectEx(hProcess, lpBaseAddress, nSize, dwNewProtect, &dwOldProtect);
    
    SIZE_T nBytesToWritten = 0;
    WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &nBytesToWritten);
    if (nBytesToWritten == 0)
    {
        throw std::runtime_error("can't write process");
    }

    VirtualProtectEx(hProcess, lpBaseAddress, nSize, dwOldProtect, &dwNewProtect);
}


int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd

)
{
    int nExitCode = 0;
    PROCESS_INFORMATION pi = { 0 };
    try {
        STARTUPINFO si = { 0 };
        GetStartupInfo(&si);
       
        if (!CreateProcess(L"Test.exe", NULL, NULL, NULL, FALSE, DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
        {
            throw std::runtime_error("can't open test.exe");
        }

        DEBUG_EVENT dbgEvent = { 0 };
        while (TRUE)
        {
            WaitForDebugEvent(&dbgEvent, INFINITE);
            if (dbgEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            {
                break;
            }
            else if (dbgEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
            {
                char asmInt3 = 0xCC;
                hzxWriteProcessMemory(pi.hProcess, (LPVOID)UPX_BEGIN_ADDR, &asmInt3, 1);
            }
            else if (dbgEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
            {
                EXCEPTION_DEBUG_INFO* pEdi = (EXCEPTION_DEBUG_INFO*) & dbgEvent.u;
                if (pEdi->ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
                {
                    CONTEXT context = { 0 };
                    context.ContextFlags = CONTEXT_FULL;
                    GetThreadContext(pi.hThread, &context);
                    if (context.Eip - 1 == UPX_BEGIN_ADDR){
                        context.Eip -= 1;

                        char asmPushad = 0x60;
                        hzxWriteProcessMemory(pi.hProcess, (LPVOID)UPX_BEGIN_ADDR, &asmPushad, 1);

                        context.EFlags |= 0x100;
                    }
                    SetThreadContext(pi.hThread, &context);
                }
                else if (pEdi->ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP)
                {
                    CONTEXT context = { 0 };
                    context.ContextFlags = CONTEXT_FULL;
                    GetThreadContext(pi.hThread, &context);
                    if (context.Eip == REAL_OEP) {
                        WORD asmNopNop = 0x9090;
                        hzxWriteProcessMemory(pi.hProcess, (LPVOID)PATCH_ADDR, &asmNopNop, 2);
                    }
                    else
                    {
                        context.EFlags |= 0x100;
                    }
                    SetThreadContext(pi.hThread, &context);
                }
            } // else if (dbgEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
            ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);
        } // while (TRUE)
    } // try {
    catch (std::runtime_error e)
    {
        MessageBoxA(NULL, e.what(), "error", MB_OK);
        nExitCode = 1;
    }
    SAFE_CLOSE_HANDLE(pi.hProcess);
    SAFE_CLOSE_HANDLE(pi.hThread);
	return nExitCode;
}