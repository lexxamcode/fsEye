#pragma once
#include <iostream>
#include <stringapiset.h>

namespace localization
{
    static std::wstring strtows(const std::string& str, UINT codePage)
    {
        std::wstring ws;
        int n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/NULL, 0);
        if (n)
        {
            ws.resize(n - 1);
            if (MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/&ws[0], n) == 0)
                ws.clear();
        }
        return ws;
    }

    static std::string wstostr(const std::wstring& ws, UINT codePage)
    {
        std::string str;
        int n = WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size() + 1, /*dst*/NULL, 0, /*defchr*/0, NULL);
        if (n)
        {
            str.resize(n - 1);
            if (WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size() + 1, /*dst*/&str[0], n, /*defchr*/0, NULL) == 0)
                str.clear();
        }
        return str;
    }

    static std::string chcp(const std::string& str, UINT codePageSrc, UINT codePageDst)
    {
        return wstostr(strtows(str, codePageSrc), codePageDst);
    }
}