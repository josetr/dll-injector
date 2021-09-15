#include "resource.h"
#include "framework.h"
#include "injector.h"

#include <filesystem>

INT_PTR CALLBACK Dialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InjectDLL(HWND hWnd, const std::wstring& executablePath, const std::wstring& dllPath);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    const auto handle = CreateMutex(nullptr, TRUE, L"JoseTorres:dll-injector");
    if (handle == nullptr || GetLastError() != 0)
    {
        MessageBox(nullptr, L"This application is already running!", nullptr, MB_ICONERROR);
        return 0;
    }

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), nullptr, Dialog);
    return 0;
}

INT_PTR CALLBACK Dialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON)));
        SetDlgItemText(hWnd, IDC_EXE, L"Application.exe");
        SetDlgItemText(hWnd, IDC_DLL, L"Dll.dll");
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_START:
        {
            wchar_t executablePath[MAX_PATH + 1]{};
            GetDlgItemText(hWnd, IDC_EXE, executablePath, static_cast<int>(std::size(executablePath)) - 1);

            wchar_t dllPath[MAX_PATH + 1]{};
            GetDlgItemText(hWnd, IDC_DLL, dllPath, static_cast<int>(std::size(executablePath)) - 1);

            InjectDLL(hWnd, executablePath, dllPath);
            break;
        }
        case IDC_ABOUT:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, About);
            break;
        }
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return 0;
}

INT_PTR CALLBACK About(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    case WM_COMMAND:
        EndDialog(hWnd, 0);
        break;
    }

    return 0;
}

void InjectDLL(HWND hWnd, const std::wstring& executablePath, const std::wstring& dllPath)
{
    PROCESS_INFORMATION processInfo{};
    STARTUPINFO info{ sizeof(info) };

    if (!std::filesystem::exists(executablePath))
        return ShowError(hWnd, std::format(L"Executable '{}' doesn't exist.", executablePath));

    if (!std::filesystem::exists(dllPath))
        return ShowError(hWnd, std::format(L"DLL '{}' doesn't exist.", dllPath));

    if (!CreateProcess(executablePath.data(), nullptr, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &info, &processInfo))
        return ShowWindowsError(hWnd, L"CreateProcess()");

    if (!InjectDLL(processInfo.hProcess, dllPath))
        ShowWindowsError(hWnd);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
}
