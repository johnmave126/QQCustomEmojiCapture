#include "stdafx.h"
#include "EmojiCapture.h"
#include "NotificationTray.h"
#include "Clipboard.h"
#include "resource.h"

wchar_t const szWindowClass[] = L"EmojiCapture";

HWND                InitWindow(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    HWND hWnd = InitWindow(hInstance);
    if (hWnd == nullptr) {
        return GetLastError();
    }
    ShowWindow(hWnd, SW_HIDE);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        return static_cast<DWORD>(hr);
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

HWND InitWindow(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = { sizeof(wcex) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = szWindowClass;
    if (RegisterClassExW(&wcex) != 0) {
        HWND hWnd = CreateWindowExW(0, szWindowClass, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, hInstance);
        if (hWnd == nullptr) {
            UnregisterClassW(szWindowClass, hInstance);
        }
        return hWnd;
    }
    return nullptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HINSTANCE hInstance = nullptr;
    LPCREATESTRUCTW pCreate = nullptr;
    int command;
    switch (message) {
    case WM_CREATE:
        pCreate = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        hInstance = reinterpret_cast<HINSTANCE>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(hInstance));
        if (!InitNotificationIcon(hInstance, hWnd)) {
            MessageBoxW(hWnd, L"Failed to initialize tray icon", L"Error", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
            return -1;
        }

        if (!RegisterHotKey(hWnd, ID_HOTKEY, MOD_CONTROL | MOD_ALT, 0x53 /* S */)) {
            MessageBoxW(hWnd, L"Failed to assign hotkey Ctrl-Alt-S, please use context menu instead", L"Warning", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        }
        break;
    case WM_COMMAND:
        command = LOWORD(wParam);
        switch (command) {
        case IDM_CAPTURE:
            CaptureEmoji(hWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    case WM_HOTKEY:
        switch (LOWORD(wParam)) {
        case ID_HOTKEY:
            CaptureEmoji(hWnd);
            break;
        }
        break;
    case WMAPP_NOTIFYCALLBACK:
        return TrayProc(hWnd, message, wParam, lParam);
    case WM_DESTROY:
        DeleteNotificationIcon();
        UnregisterHotKey(hWnd, ID_HOTKEY);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

HINSTANCE GetModuleFromProc(HWND hWnd) {
    LONG_PTR ptr = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    return reinterpret_cast<HINSTANCE>(ptr);
}
