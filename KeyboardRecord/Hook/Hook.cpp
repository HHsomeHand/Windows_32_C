#include "pch.h"
#include "Hook.h"

HHOOK g_hHook;

extern HMODULE g_hInstance;

HWND g_hWnd;

DWORD g_dwMessage;

#define IsKeyPressed(nVirtKey) ((GetKeyState(nVirtKey) & 0x8000) != 0)

LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam)
{
    CallNextHookEx(g_hHook, code, wParam, lParam);

    BYTE szKeyState[256] = { 0 };
    GetKeyboardState(szKeyState);

    if (IsKeyPressed(VK_SHIFT))
    {
        szKeyState[VK_SHIFT] = 1;
    }

    DWORD dwVirtualCode = wParam;

    char strAscii[4] = { 0 };
    ToAscii(dwVirtualCode, HIWORD(lParam), szKeyState, (LPWORD)strAscii, 0);

    for (int i = 0; strAscii[i] != '\0'; i++)
    {
        SendMessageW(g_hWnd, g_dwMessage, (WPARAM)strAscii[i], 0);

    }

    return 0;
}

HOOK_API HHOOK InstallHook(HWND hwnd, int code)
{
    if (g_hHook == NULL)
    {
        g_hWnd = hwnd;
        g_dwMessage = code;
        g_hHook = SetWindowsHookEx(WH_KEYBOARD, HookProc, g_hInstance, NULL);
        return g_hHook;
    }
    else
    {
        return FALSE;
    }
}

HOOK_API void UninstallHook()
{
    if (g_hHook != NULL){
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}
