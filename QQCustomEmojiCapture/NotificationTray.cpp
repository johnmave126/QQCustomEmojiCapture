#include "stdafx.h"
#include "EmojiCapture.h"
#include "NotificationTray.h"
#include "resource.h"

class __declspec(uuid("be92ee76-cf10-4478-897a-47b1e56c7144")) EmojiCaptureIcon;
void ShowContextMenu(HWND hWnd, POINT pt);

BOOL InitNotificationIcon(HINSTANCE hInstance, HWND hWnd) {
    NOTIFYICONDATAW nid = { sizeof(nid) };
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    nid.guidItem = __uuidof(EmojiCaptureIcon);
    nid.hIcon = static_cast<HICON>(LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
    nid.hWnd = hWnd;
    LoadStringW(hInstance, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIconW(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIconW(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon() {
    NOTIFYICONDATAW nid = { sizeof(nid) };
    nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(EmojiCaptureIcon);
    return Shell_NotifyIconW(NIM_DELETE, &nid);
}

LRESULT CALLBACK TrayProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(lParam)) {
    case WM_CONTEXTMENU:
        POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
        ShowContextMenu(hWnd, pt);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

void ShowContextMenu(HWND hWnd, POINT pt) {
    HINSTANCE hInstance = GetModuleFromProc(hWnd);
    HMENU hMenu = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDR_MENU1));
    if (hMenu == nullptr) {
        return;
    }
    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (hSubMenu == nullptr) {
        DestroyMenu(hMenu);
        return;
    }
    SetForegroundWindow(hWnd);

    UINT uFlags = TPM_RIGHTBUTTON;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
        uFlags |= TPM_RIGHTALIGN;
    }
    else {
        uFlags |= TPM_LEFTALIGN;
    }

    TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hWnd, NULL);
}
