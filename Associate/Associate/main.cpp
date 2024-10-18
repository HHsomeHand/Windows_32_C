#include <Windows.h>

// lpExt -> txt
// lpExe -> notepad.exe %1
BOOL RegisterAssociate(LPCWSTR lpExt, LPCWSTR lpExe, BOOL isInstall)
{
    BOOL isSuccess = TRUE; 
    HKEY hKey = NULL;

    HKEY hRootKey = NULL;

    LPWSTR lpExtRegPath = NULL;

    DWORD dwResult = 0;

    DWORD len = lstrlen(lpExt) + 2;
    LPWSTR lpDotExt = (LPWSTR)malloc(len * sizeof(WCHAR));
    if (lpDotExt == NULL)
    {
        isSuccess = FALSE;
        goto RegisterAssociateExit;
    }
    lstrcpy(lpDotExt, L".");
    lstrcat(lpDotExt, lpExt);

    lpExtRegPath = (LPWSTR)malloc(MAX_PATH);
    if (lpExtRegPath == NULL)
    {
        isSuccess = FALSE;
        goto RegisterAssociateExit;
    }
    lstrcpy(lpExtRegPath, L"");
    lstrcat(lpExtRegPath, lpExt);
    lstrcat(lpExtRegPath, LR"(\shell\open\command)");

    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER, LR"(Software\Classes)", 0, KEY_ALL_ACCESS, &hRootKey);
    if (dwResult != ERROR_SUCCESS)
    {
        isSuccess = FALSE;
        goto RegisterAssociateExit;
    }

    if (isInstall)
    {
        
        dwResult = RegCreateKey(hRootKey, lpDotExt, &hKey);
        if (dwResult == ERROR_SUCCESS)
        {
            DWORD entryLen = lstrlenW(lpExt) + 1;
            RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)lpExt, entryLen * sizeof(WCHAR));
            RegCloseKey(hKey);
        }
        else
        {
            isSuccess = FALSE;
            goto RegisterAssociateExit;
        }

        

        dwResult = RegCreateKey(hRootKey, lpExtRegPath, &hKey);
        if (dwResult == ERROR_SUCCESS)
        {
            HKEY hSubKey = NULL;
            RegOpenKeyEx(hRootKey, lpExt, 0, KEY_ALL_ACCESS, &hSubKey);
            LPCWSTR lpDesc = L"filetype";
            DWORD descLen = lstrlenW(lpDesc) + 1;
            RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (PBYTE)lpDesc, descLen * sizeof(WCHAR));

            DWORD exeLen = lstrlenW(lpExe) + 1;
            RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)lpExe, exeLen * sizeof(WCHAR));
            RegCloseKey(hSubKey);
            RegCloseKey(hKey);
        }
        else
        {
            isSuccess = FALSE;
            goto RegisterAssociateExit;
        }
    }
    else
    {
        dwResult = RegDeleteKey(hRootKey, lpDotExt);
        if (dwResult != ERROR_SUCCESS && dwResult != ERROR_FILE_NOT_FOUND)
        {
            isSuccess = FALSE;
            goto RegisterAssociateExit;
        }

        
        dwResult = RegDeleteTreeW(hRootKey, lpExt);
        if (dwResult != ERROR_SUCCESS && dwResult != ERROR_FILE_NOT_FOUND)
        {
            isSuccess = FALSE;
            goto RegisterAssociateExit;
        }
    }

RegisterAssociateExit:
    RegCloseKey(hRootKey);
    free(lpDotExt);
    free(lpExtRegPath);
    return isSuccess;
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    BOOL result = RegisterAssociate(L"test", LR"("C:\Users\hwh\Desktop\widget\WinMain参数测试\WinMain32.exe" "%1")", FALSE);
    
    if (result)
    {
        MessageBoxW(NULL, L"成功", L"DEBUG", MB_OK);
    }
    else
    {
        MessageBoxW(NULL, L"失败", L"DEBUG", MB_OK);
    }
    return 0;
}