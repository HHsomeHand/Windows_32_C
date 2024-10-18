#pragma once

#ifdef HOOK_EXPORTS
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif

#include <Windows.h>

extern "C" HOOK_API HHOOK InstallHook(HWND hwnd, int code);
extern "C" HOOK_API void UninstallHook();
