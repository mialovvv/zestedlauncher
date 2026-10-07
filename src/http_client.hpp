#pragma once

#include <windows.h>
#include <shlobj.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>

#ifndef WINHTTP_NO_REFERRER
#define WINHTTP_NO_REFERRER WINHTTP_NO_REFERER
#endif

class HttpClient {
public:
    static HttpClient& instance() {
        static HttpClient client;
        return client;
    }

    HttpClient() {
        hSession_ = WinHttpOpen(L"ZestedLauncher/1.0 (Windows NT 10.0; Win64; x64)",
                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                WINHTTP_NO_PROXY_NAME,
                                WINHTTP_NO_PROXY_BYPASS, 0);
        if (hSession_) {
            // Set reasonable timeouts (Resolve: 10s, Connect: 15s, Send: 30s, Receive: 60s)
            WinHttpSetTimeouts(hSession_, 10000, 15000, 30000, 60000);
            DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
            WinHttpSetOption(hSession_, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols));
        }
    }

    ~HttpClient() {
        if (hSession_) {
            WinHttpCloseHandle(hSession_);
        }
    }

    struct Response {
        int status_code = 0;
        std::string body;
        bool success = false;
        std::string error;
    };

    static std::wstring to_wide(const std::string& str) {
        if (str.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
        std::wstring wstr(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size);
        return wstr;
    }

    static std::string to_utf8(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string str(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size, nullptr, nullptr);
        return str;
    }

    Response get(const std::string& url, const std::vector<std::string>& headers = {}) {
        return request("GET", url, "", headers);
    }

    Response post(const std::string& url, const std::string& body, const std::vector<std::string>& headers = {}) {
        return request("POST", url, body, headers);
    }

    Response request(const std::string& method, const std::string& url, const std::string& postData, const std::vector<std::string>& headers) {
        Response resp;
        if (!hSession_) {
            resp.error = "WinHttp session not initialized";
            return resp;
        }

        std::wstring wurl = to_wide(url);
        URL_COMPONENTS urlComp;
        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = (DWORD)-1;
        urlComp.dwHostNameLength = (DWORD)-1;
        urlComp.dwUrlPathLength = (DWORD)-1;
        urlComp.dwExtraInfoLength = (DWORD)-1;

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            resp.error = "Invalid URL";
            return resp;
        }

        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring urlPath;
        if (urlComp.dwUrlPathLength > 0) {
            urlPath.append(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        }
        if (urlComp.dwExtraInfoLength > 0) {
            urlPath.append(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
        }
        if (urlPath.empty()) urlPath = L"/";

        INTERNET_PORT port = urlComp.nPort;
        bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hConnect = WinHttpConnect(hSession_, hostName.c_str(), port, 0);
        if (!hConnect) {
            resp.error = "Connection failed";
            return resp;
        }

        std::wstring wmethod = to_wide(method);
        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, wmethod.c_str(), urlPath.c_str(), nullptr,
                                               WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            resp.error = "Open request failed";
            return resp;
        }

        // Enable redirects
        DWORD opt = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &opt, sizeof(opt));

        // Add headers
        std::wstring allHeaders;
        for (const auto& h : headers) {
            allHeaders += to_wide(h) + L"\r\n";
        }

        BOOL sendRes = WinHttpSendRequest(
            hRequest,
            allHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : allHeaders.c_str(),
            (DWORD)allHeaders.length(),
            postData.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)postData.data(),
            (DWORD)postData.length(),
            (DWORD)postData.length(),
            0
        );

        if (!sendRes || !WinHttpReceiveResponse(hRequest, nullptr)) {
            resp.error = "Send or receive response failed";
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            return resp;
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);
        resp.status_code = statusCode;

        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        std::string body;
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;

            std::vector<char> buffer(dwSize + 1);
            if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                body.append(buffer.data(), dwDownloaded);
            }
        } while (dwSize > 0);

        resp.body = body;
        resp.success = (statusCode >= 200 && statusCode < 300);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        return resp;
    }

    bool download_file(const std::string& url, const std::string& destination_path,
                       std::function<void(uint64_t downloaded, uint64_t total)> progress_callback = nullptr) {
        if (!hSession_) return false;

        std::wstring wurl = to_wide(url);
        URL_COMPONENTS urlComp;
        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = (DWORD)-1;
        urlComp.dwHostNameLength = (DWORD)-1;
        urlComp.dwUrlPathLength = (DWORD)-1;
        urlComp.dwExtraInfoLength = (DWORD)-1;

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            return false;
        }

        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring urlPath;
        if (urlComp.dwUrlPathLength > 0) {
            urlPath.append(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        }
        if (urlComp.dwExtraInfoLength > 0) {
            urlPath.append(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
        }
        if (urlPath.empty()) urlPath = L"/";

        INTERNET_PORT port = urlComp.nPort;
        bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hConnect = WinHttpConnect(hSession_, hostName.c_str(), port, 0);
        if (!hConnect) return false;

        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath.c_str(), nullptr,
                                               WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            return false;
        }

        // Enable redirects
        DWORD opt = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &opt, sizeof(opt));

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
            !WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            return false;
        }

        // Query content length
        uint64_t totalLength = 0;
        wchar_t contentLengthStr[64] = {0};
        DWORD headerLen = sizeof(contentLengthStr);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX,
                                contentLengthStr, &headerLen, WINHTTP_NO_HEADER_INDEX)) {
            totalLength = _wtoi64(contentLengthStr);
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX)) {
            if (statusCode < 200 || statusCode >= 300) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                return false;
            }
        }

        // Create parent directories if needed - normalize slashes to backslashes for Windows API
        std::wstring wdest = to_wide(destination_path);
        for (auto& c : wdest) {
            if (c == L'/') c = L'\\';
        }
        size_t lastSlash = wdest.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos) {
            std::wstring dir = wdest.substr(0, lastSlash);
            SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
        }

        HANDLE hFile = CreateFileW(wdest.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            return false;
        }

        uint64_t totalDownloaded = 0;
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        char buffer[32768];

        bool success = true;
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                success = false;
                break;
            }
            if (dwSize == 0) break;

            DWORD bytesToRead = (dwSize > sizeof(buffer)) ? sizeof(buffer) : dwSize;
            if (WinHttpReadData(hRequest, buffer, bytesToRead, &dwDownloaded)) {
                DWORD bytesWritten = 0;
                WriteFile(hFile, buffer, dwDownloaded, &bytesWritten, nullptr);
                totalDownloaded += dwDownloaded;
                if (progress_callback) {
                    progress_callback(totalDownloaded, totalLength);
                }
            } else {
                success = false;
                break;
            }
        } while (dwSize > 0);

        CloseHandle(hFile);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);

        if (!success) {
            DeleteFileW(wdest.c_str());
        }
        return success;
    }

private:
    HINTERNET hSession_ = nullptr;
};
