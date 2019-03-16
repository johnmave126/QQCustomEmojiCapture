#pragma once
#include "stdafx.h"

BOOL InitNotificationIcon(HINSTANCE hInstance, HWND hWnd);
BOOL DeleteNotificationIcon();
LRESULT CALLBACK TrayProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
