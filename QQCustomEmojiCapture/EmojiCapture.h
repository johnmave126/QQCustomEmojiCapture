#pragma once
#include "stdafx.h"

const int _WMAPP_NOTIFYCALLBACK = (WM_APP + 1);
#define WMAPP_NOTIFYCALLBACK _WMAPP_NOTIFYCALLBACK

#define ID_HOTKEY 1000

HINSTANCE GetModuleFromProc(HWND hWnd);
