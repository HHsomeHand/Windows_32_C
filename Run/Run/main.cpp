#include <Windows.h>
#include "Reg.h"

void SetAutoRun(BOOL isInstall)
{
    PCWSTR pszKey = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)";
    PCWSTR pszValueName = L"Test";
    if (isInstall)
    {
        WCHAR strExePath[MAX_PATH] = { 0 };
        int len = GetModuleFileName(NULL, strExePath, MAX_PATH) + 1;
        _RegSetValue(pszKey, pszValueName, REG_SZ, (PBYTE)strExePath, len * sizeof(WCHAR));
    }
    else
    {
        _RegDelValue(pszKey, pszValueName);
    }
}
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    SetAutoRun(TRUE);

    wchar_t szBuffer[64] = L"ÄãºÃ ÊÀ½ç";
        MessageBoxW(NULL, szBuffer, L"DEBUG", MB_OK);

    
	return 0;
}