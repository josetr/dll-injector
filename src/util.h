#pragma once

#include "framework.h"

#include <format>
#include <string>

inline void ShowError(HWND hwnd, const std::wstring& error)
{
    MessageBox(hwnd, error.c_str(), L"Error", MB_ICONERROR);
}

inline void ShowWindowsError(HWND hwnd, const std::wstring& functionName = L"")
{
    const auto errorCode = GetLastError();

    LPWSTR winMessage = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (WCHAR*)&winMessage, 0, nullptr);
    const auto error = !functionName.empty()
        ? std::format(L"{} failed with error {}: {}", functionName, errorCode, winMessage)
        : std::format(L"Error {}: {}", errorCode, winMessage);

    if (winMessage != nullptr)
        LocalFree(winMessage);

    ShowError(hwnd, error.c_str());
}

inline bool SetDebugPrivileges()
{
    TOKEN_PRIVILEGES tokenPrivileges{};
    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tokenPrivileges.Privileges[0].Luid))
        return false;

    HANDLE tokenHandle = INVALID_HANDLE_VALUE;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &tokenHandle) || tokenHandle == INVALID_HANDLE_VALUE)
        return false;

    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tokenPrivileges.PrivilegeCount = 1;

    const auto result = AdjustTokenPrivileges(tokenHandle, false, &tokenPrivileges, sizeof(tokenPrivileges), nullptr, nullptr);
    CloseHandle(tokenHandle);
    return result;
}
