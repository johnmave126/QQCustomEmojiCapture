#include "stdafx.h"
#include "Clipboard.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <string>
#include <sstream>
#include <ShlObj.h>


void SaveFile(HWND hWnd, LPCWSTR wszFilepath);

void CaptureEmoji(HWND hWnd) {
    if (!OpenClipboard(hWnd)) {
        return;
    }
    UINT iFormatCode = 0;
    wchar_t sFormatNameBuf[128];
    do {
        iFormatCode = EnumClipboardFormats(iFormatCode);
        GetClipboardFormatNameW(iFormatCode, sFormatNameBuf, 127);
        if (lstrcmpW(sFormatNameBuf, L"QQ_Unicode_RichEdit_Format") == 0) {
            break;
        }
    } while (iFormatCode != 0);

    if (iFormatCode == 0) {
        CloseClipboard();
        return;
    }
    HANDLE hData = GetClipboardData(iFormatCode);
    if (hData == nullptr) {
        CloseClipboard();
        return;
    }
    BYTE* pData = static_cast<BYTE*>(GlobalLock(hData));
    if (pData == nullptr) {
        CloseClipboard();
        return;
    }
    SIZE_T size = GlobalSize(hData);
    auto pStrData = std::make_unique<wchar_t[]>(size + 2);
    std::copy(pData, pData + size, pStrData.get());
    GlobalUnlock(hData);
    CloseClipboard();
    pStrData.get()[size] = pStrData.get()[size + 1] = '\0';

    rapidxml::xml_document<WCHAR> doc;
    try {
        doc.parse<0>(pStrData.get());
    }
    catch (const rapidxml::parse_error&) {
        return;
    }

    auto child = doc.first_node(L"QQRichEditFormat");
    if (!child) {
        return;
    }
    child = child->first_node(L"EditElement");
    if (!child) {
        return;
    }
    auto attr = child->first_attribute(L"filepath");
    if (!attr) {
        return;
    }
    LPCWSTR wszFilepath = attr->value();
    SaveFile(hWnd, wszFilepath);
}

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"GIF (*.gif)",       L"*.gif"}
};

void SaveFile(HWND hWnd, LPCWSTR wszFilepath) {
    HRESULT hr;
    PWSTR wsUserPicPath;
    std::wstring strTargetPath;
    hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &wsUserPicPath);
    if (FAILED(hr)) {
        return;
    }
    strTargetPath = wsUserPicPath;
    strTargetPath += L"\\QQCustomEmoji";
    CoTaskMemFree(wsUserPicPath);
    if (!CreateDirectoryW(strTargetPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return;
    }
    IShellItem *psiDefaultFolder = nullptr;
    hr = SHCreateItemFromParsingName(strTargetPath.c_str(), nullptr, IID_PPV_ARGS(&psiDefaultFolder));
    if (FAILED(hr)) {
        return;
    }
    IFileDialog *pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (FAILED(hr)) {
        return;
    }
    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->SetOptions(dwFlags | FOS_STRICTFILETYPES);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->SetDefaultExtension(L"gif");
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->SetDefaultFolder(psiDefaultFolder);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->AddPlace(psiDefaultFolder, FDAP_TOP);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    hr = pfd->Show(NULL);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    IShellItem *psiResult;
    hr = pfd->GetResult(&psiResult);
    if (FAILED(hr)) {
        pfd->Release();
        return;
    }
    PWSTR pszFilePath = NULL;
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    if (FAILED(hr)) {
        psiResult->Release();
        pfd->Release();
        return;
    }
    CopyFileW(wszFilepath, pszFilePath, false);
    CoTaskMemFree(pszFilePath);
    psiResult->Release();
    pfd->Release();
}
