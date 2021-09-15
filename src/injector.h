#pragma once

#include "framework.h"
#include "util.h"

#include <string>
#include <chrono>

inline bool InjectDLL(HANDLE processHandle, const std::wstring& dllPath, std::chrono::milliseconds timeout = std::chrono::seconds(5))
{
    if (!SetDebugPrivileges())
        return false;

    const auto kernel32 = GetModuleHandle(L"Kernel32");
    if (kernel32 == nullptr)
        return false;

    const auto loadLibraryAddress = GetProcAddress(kernel32, "LoadLibraryW");
    const auto remoteMemory = VirtualAllocEx(processHandle, nullptr, dllPath.size(), MEM_COMMIT, PAGE_READWRITE);
    if (remoteMemory == nullptr)
        return false;

    SIZE_T numberOfBytesWritten;
    WriteProcessMemory(processHandle, remoteMemory, dllPath.c_str(), dllPath.size() * sizeof(WCHAR), &numberOfBytesWritten);

    const auto remoteThreadHandle = CreateRemoteThread(processHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, remoteMemory, 0, nullptr);
    if (remoteThreadHandle == nullptr)
        return false;

    const auto result = WaitForSingleObject(remoteThreadHandle, static_cast<DWORD>(timeout.count())) != WAIT_TIMEOUT;
    VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(remoteThreadHandle);
    return result;
}
