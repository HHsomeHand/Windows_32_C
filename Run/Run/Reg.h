#pragma once

#include <Windows.h>

void _RegCreateSubKey(PCWCHAR strKey, PCWCHAR strSubKeyName);
void _RegDelSubKey(PCWCHAR strKey, PCWCHAR strSubKeyName);
void _RegSetValue(PCWCHAR strKey, PCWCHAR strValueName, DWORD dwValueType, PBYTE bytesValue, DWORD dwSize);
void _RegDelValue(PCWCHAR strKey, PCWCHAR strValueName);

void _RegQueryValue(PCWCHAR strKey, PCWCHAR strValueName, PDWORD lpType, PBYTE lpData, PDWORD lpcbData);
