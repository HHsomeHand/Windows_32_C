#include "Reg.h"
#include <functional>
#include <string>

typedef std::function<void(HKEY hKey)>  REG_HANDLER;

void _RegHandle(DWORD samDesired, const WCHAR* strKey, const  REG_HANDLER& fnCallBack)
{
	HKEY hKey = NULL;

	DWORD result = RegOpenKeyEx(HKEY_CURRENT_USER, strKey, NULL, samDesired, &hKey);


	if (result == ERROR_SUCCESS)
	{
		fnCallBack( hKey);
		RegCloseKey(hKey);
	}

}

void _RegCreateSubKey(PWCHAR strKey, PWCHAR strSubKeyName)
{
	_RegHandle(KEY_CREATE_SUB_KEY, strKey, [strSubKeyName](HKEY hKey) {
		HKEY hSubKey = NULL;
		RegCreateKeyEx(hKey, strSubKeyName, 0, NULL, 0, 0, NULL, &hSubKey, NULL);

		RegCloseKey(hSubKey);
	});
}

void _RegDelSubKey(PWCHAR strKey, PWCHAR strSubKeyName)
{
	_RegHandle(KEY_WRITE, strKey, [strSubKeyName](HKEY hKey) {
		RegDeleteKey(hKey, strSubKeyName);
		});
}

void _RegSetValue(PWCHAR strKey, PWCHAR strValueName, DWORD dwValueType, PBYTE bytesValue, DWORD dwSize)
{
	_RegHandle(KEY_WRITE, strKey, [strValueName, bytesValue, dwValueType, dwSize](HKEY hKey) {
		RegSetValueEx(hKey, strValueName, 0, dwValueType, bytesValue, dwSize);
		});
}

void _RegDelValue(PWCHAR strKey, PWCHAR strValueName)
{
	_RegHandle(KEY_WRITE, strKey, [strValueName](HKEY hKey) {
		RegDeleteValue(hKey, strValueName);
		});
}

void _RegQueryValue(PWCHAR strKey, PWCHAR strValueName, PDWORD lpType, PBYTE lpData, PDWORD lpcbData)
{
	_RegHandle(KEY_QUERY_VALUE, strKey, [strValueName, lpType, lpData, lpcbData](HKEY hKey) {
		RegQueryValueEx(hKey, strValueName, 0, lpType, lpData, lpcbData);
		});
}