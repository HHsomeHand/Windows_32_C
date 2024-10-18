#include <Windows.h>
#include <stdexcept>

int WINAPI wWinMain(
	HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPWSTR lpCmdLine,          // command line
	int nCmdShow              // show state
)
{
    try {
        STARTUPINFO si = { 0 };
        GetStartupInfo(&si);
        PROCESS_INFORMATION pi = { 0 };
        if (!CreateProcess(
            L"Test.exe",
            NULL,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
            NULL,
            NULL,
            &si,
            &pi))
        {
            throw std::runtime_error("can't open process");
        }
        DWORD oldProtect = 0;
        VirtualProtectEx(pi.hProcess, (LPVOID)0x401004, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
       
        BYTE byteOld[2] = { 0x74, 0x15 };
        BYTE byteBuff[2] = { 0 };
        SIZE_T nRead = 0;
        ReadProcessMemory(pi.hProcess, (LPCVOID)0x401004, byteBuff, 2, &nRead);
        if (nRead == 0)
        {
            throw std::runtime_error("can't read process");
        }

        if (memcmp(byteOld, byteBuff, 2) != 0)
        {
            throw std::runtime_error("test.exe version error");
        }

        BYTE bytePatch[2] = { 0x90, 0x90 };
        SIZE_T nWrite = 0;
        WriteProcessMemory(pi.hProcess, (LPVOID)0x401004, bytePatch, 2, &nWrite);
        if (nWrite == 0)
        {
            throw std::runtime_error("can't write process");
        }

        DWORD oldProtect2 = 0;
        VirtualProtectEx(pi.hProcess, (LPVOID)0x401004, 2, oldProtect, &oldProtect2);

        ResumeThread(pi.hThread);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    catch (std::runtime_error err)
    {
        MessageBoxA(NULL, err.what(), "²¹¶¡", MB_OK);
    }
    return 0;
}