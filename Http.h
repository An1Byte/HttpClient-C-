#ifndef __HTTP__
#define __HTTP__

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

class Http {
private:
    LPCWSTR DEFAULT_USER_AGENT;
    std::string data;
    std::wstring method;
    std::string header;
    size_t headerSize;

    struct WURL { // ��������� � ������� ����� �������������� ������ URL.
        std::wstring host, path; // ���� � ����.
        INTERNET_PORT port; // ����.
        DWORD dwFlags; // �����. �������� ������� ssl.
    };

    struct HTTP_REQ_OPTS { // ��������� ����������, ������� ����� ������������ � �������� ������� ��������� ������� http_request.
        std::wstring method = L"GET"; // ������ ����� GET ����� �� ���������, ����� ������� �� ����� �������� � ������� http_request ������ ���������� ������ ��������� {}.
        std::vector<uint8_t> data = {};
    };

    inline WURL parse_url(std::wstring url) { // ������� �������� url - ������.
        WURL wurl = {};
        size_t index = url.find(L"://");
        if (index != std::string::npos) { // ���� :// � url �������...
            const std::wstring& protocol = url.substr(0, index); // ����� ������ ��������...
            if (protocol.compare(L"https") == 0) { // ���� �������� == https...
                wurl.dwFlags = WINHTTP_FLAG_SECURE; // ������������� ���� ������������ �������� ������.
                wurl.port = 443; // ������������� ���� 443 ��� https �������.
            }
            else if (protocol.compare(L"http") == 0) { // � ���� �������� == http...
                if ((wurl.dwFlags & WINHTTP_FLAG_SECURE) == WINHTTP_FLAG_SECURE) { // ���� ���� ������ �������� ����������, �����...
                    wurl.dwFlags ^= WINHTTP_FLAG_SECURE; // ��������� ������������� ����, ����� ��� �� �������������.
                }
                wurl.port = 80; // ������������� ���� 80 ��� http �������.
            }
            url = url.substr(index + 3);
            index = url.find(L"/"); // ������� ������� ���������� �����, �������� ����� ://
            if (index == std::string::npos) { // ���� ������ �� ������(����� ���)
                wurl.host = url; // ����� � �������� ����� ����� �������� ��� url ������ �� ������� https://.
                wurl.path = L"/"; // � � �������� ���� ������ ������ ����.
            }
            else { // ���� ���� ������...
                wurl.host = url.substr(0, index); // ����� ����� ������ ����� �������� ������ ����� 2�� ������� (https://  host   /)
                wurl.path = url.substr(index); // � � ���� ����� ���������� url ������ ����� �����.
            }
            // ���������� ����� �����, ���� �� ������ � url:
            index = wurl.host.find(L":");
            if (index != std::string::npos) { // ���� ���� ������...
                wurl.port = _wtoi(wurl.host.substr(index + 1).c_str()); // ����� ������������� � �������� ����� ��������� ����.
                wurl.host = wurl.host.substr(0, index); // � � �������� ����� ��� ��, ��� ��������� ����� :// � �� : � �����.
            }
            // � ������ �������� ����� ����� �������� ������� ����, �������� ����� �� get - ���������, ������ ����� ����� ? � ��� �����...
        }
        return wurl; // ���������� ���������. Url - ���������!
    }

    // ������� �������� ��������:
    // ������� ��������� � �������� ���������� ������(url) � ��������� (options). ������ � ���������� ���������� ����� ����������...
    inline std::vector<uint8_t> http_request(const std::wstring& url, const HTTP_REQ_OPTS& options = { L"GET", {} }) {
        const WURL& wurl = parse_url(url); // ������ ������ url.
        std::vector<uint8_t> answer = {}; // ������� ������, ������� ����� ������� �� �������.
        if (HINTERNET hSession = WinHttpOpen(DEFAULT_USER_AGENT, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)) {
            if (HINTERNET hConnect = WinHttpConnect(hSession, wurl.host.c_str(), wurl.port, 0)) {
                if (HINTERNET hRequest = WinHttpOpenRequest(hConnect, options.method.c_str(), wurl.path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, wurl.dwFlags)) {
                    LPVOID data = WINHTTP_NO_REQUEST_DATA; // ��������� ���������� � ������� ����� ��������� ������, ������� ����� ����������.
                    const IN DWORD& data_len = options.data.size(); // ��������� ����������, � ������� ����� ����������� ����� ���������� ������.
                    if (data_len > 0) data = (LPVOID)options.data.data(); // ���� ����� > 0, ����� ������������ ���������� � ��� LPVOID.

                    // --------------------- ���� ���������� ����������:
                    std::wstring tempstr(header.begin(), header.end());
                    WinHttpAddRequestHeaders(hRequest, tempstr.c_str(), headerSize, WINHTTP_ADDREQ_FLAG_REPLACE);
                    // -------------------------------------------------------------------------------------------

                    if (bool bResult = WinHttpSendRequest(hRequest, tempstr.c_str(), headerSize, data, data_len, data_len, 0)) { // ���� ������ ����������...
                        if (bResult = WinHttpReceiveResponse(hRequest, nullptr)) { // ���� ����� �� ����� �������...

                            // ------------------ ���� ������ ���������:

                            DWORD bufferSize = 0;
                            wchar_t headerBuffer[10000] = {};
                            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF | WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
                                WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                                &bufferSize, WINHTTP_NO_HEADER_INDEX);

                            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                                bool Res = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF |
                                    WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
                                    WINHTTP_HEADER_NAME_BY_INDEX,
                                    headerBuffer, &bufferSize,
                                    WINHTTP_NO_HEADER_INDEX);

                                std::wstring str = L"";
                                for (DWORD i = 0; i < bufferSize; i++) {
                                    str += headerBuffer[i];
                                }

                                std::string temp(str.begin(), str.end());
                                header = temp;
                                headerSize = temp.size();
                            }
                            // --------------------------------------------------------

                            char szBuffer[8192] = { 0 }; // ������� ����� ��������������������� 0.
                            DWORD dwByLesRead = 0; // � ������� ����������-���������, ������� ����� ������� � ���� ��, ������� ������ ���������.
                            do { // ���� ���������� ������ � �������:
                                bResult = WinHttpReadData(hRequest, szBuffer, sizeof(szBuffer), &dwByLesRead);
                                if (!bResult) break; // ���� ������ �� �������, �� ������� �� ����� ����������.
                                answer.insert(answer.end(), szBuffer, szBuffer + dwByLesRead); // ��������� ����������� ������ � ����� �������, ������� �� � ����� ������ � �������� ������ �� �������.
                                ZeroMemory(szBuffer, dwByLesRead); // �������� ������ ����������� ����� ������.
                            } while (dwByLesRead);
                        }
                    }
                    // ��������� ������, ����������� � ������ ������������� �������:
                    // � �������� ��������� �������� ��, ��� ����� ���������, � ����� ����� � ������� ����� ��-��...
                    WinHttpCloseHandle(hRequest); // ��������� ������.
                }
                WinHttpCloseHandle(hConnect); // ��������� �����������.
            }
            WinHttpCloseHandle(hSession); // ��������� ������.
        }
        return answer; // ���������� ����� �� �������.
    }

public:
    Http() { // ������ User Agent-�:
        DEFAULT_USER_AGENT = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:72.0) Gecko/20100101 Firefox/72.0";
        headerSize = 0;
    }
    // ---------------------------- ������ �������:
    std::string GET(std::wstring url, std::string datas = "") {
        data = datas;
        method = L"GET";
        std::vector<uint8_t> answer = http_request(url, { method, {std::vector<uint8_t>(data.begin(), data.end())} });
        std::string temp = std::string(answer.begin(), answer.end());
        return temp;
    }

    std::string POST(std::wstring url, std::string datas = "") {
        data = datas;
        method = L"POST";
        std::vector<uint8_t> answer = http_request(url, { method, {std::vector<uint8_t>(data.begin(), data.end())} });
        std::string temp = std::string(answer.begin(), answer.end());
        return temp;
    }

    std::string HEAD(std::wstring url, std::string datas = "") {
        data = datas;
        method = L"HEAD";
        std::vector<uint8_t> answer = http_request(url, { method, {std::vector<uint8_t>(data.begin(), data.end())} });
        std::string temp = std::string(answer.begin(), answer.end());
        return temp;
    }

    std::string getHeader() {
        return header;
    }

    void setHeader(std::string _header) {
        header = _header;
        headerSize = header.size();
    }
};

#endif