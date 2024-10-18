#pragma once

#include <Windows.h>

void _RegCreateSubKey(PWCHAR strKey, PWCHAR strSubKeyName);
void _RegDelSubKey(PWCHAR strKey, PWCHAR strSubKeyName);
void _RegSetValue(PWCHAR strKey, PWCHAR strValueName, DWORD dwValueType, PBYTE bytesValue, DWORD dwSize);
void _RegDelValue(PWCHAR strKey, PWCHAR strValueName);

void _RegQueryValue(PWCHAR strKey, PWCHAR strValueName, PDWORD lpType, PBYTE lpData, PDWORD lpcbData);
