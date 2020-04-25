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

    struct WURL { // СТРУКТУРА В КОТОРУЮ БУДЕТ РАСПАРСИВАТЬСЯ ССЫЛКА URL.
        std::wstring host, path; // Хост и путь.
        INTERNET_PORT port; // Порт.
        DWORD dwFlags; // Флаги. Например наличие ssl.
    };

    struct HTTP_REQ_OPTS { // СТРУКТУРА ПАРАМЕТРОВ, КОТОРАЯ БУДЕТ ПЕРЕДАВАТЬСЯ В КАЧЕСТВЕ ВТОРОГО АРГУМЕНТА ФУНКЦИИ http_request.
        std::wstring method = L"GET"; // Пускай метод GET будет по умолчанию, таким образом мы можем передать в функцию http_request вторым параметром пустую структуру {}.
        std::vector<uint8_t> data = {};
    };

    inline WURL parse_url(std::wstring url) { // ФУНКЦИЯ ПАРСИНГА url - ссылки.
        WURL wurl = {};
        size_t index = url.find(L"://");
        if (index != std::string::npos) { // Если :// в url найдено...
            const std::wstring& protocol = url.substr(0, index); // тогда парсим протокол...
            if (protocol.compare(L"https") == 0) { // Если протокол == https...
                wurl.dwFlags = WINHTTP_FLAG_SECURE; // Устанавливаем флаг защищенности передачи данных.
                wurl.port = 443; // Устанавливаем порт 443 для https запроса.
            }
            else if (protocol.compare(L"http") == 0) { // А если протокол == http...
                if ((wurl.dwFlags & WINHTTP_FLAG_SECURE) == WINHTTP_FLAG_SECURE) { // Если флаг защиты передачи установлен, тогда...
                    wurl.dwFlags ^= WINHTTP_FLAG_SECURE; // исключаем установленный флаг, делая его не установленным.
                }
                wurl.port = 80; // Устанавливаем порт 80 для http запроса.
            }
            url = url.substr(index + 3);
            index = url.find(L"/"); // Находим позицию следующего слэша, стоящего после ://
            if (index == std::string::npos) { // Если индекс не найден(слэша нет)
                wurl.host = url; // тогда в качестве хоста будет являться вся url строка не включая https://.
                wurl.path = L"/"; // а в качестве пути просто ставим слэш.
            }
            else { // Если слэш найден...
                wurl.host = url.substr(0, index); // тогда нашим хостом будет являться строка между 2мя слэшами (https://  host   /)
                wurl.path = url.substr(index); // а в пути будет содержимое url идущее после слэша.
            }
            // Определяем номер порта, если он указан в url:
            index = wurl.host.find(L":");
            if (index != std::string::npos) { // Если порт найден...
                wurl.port = _wtoi(wurl.host.substr(index + 1).c_str()); // тогда устанавливаем в качестве порта найденный порт.
                wurl.host = wurl.host.substr(0, index); // а в качестве хоста все то, что находится после :// и до : и порта.
            }
            // В данный алгоритм можно будет добавить парсинг пути, например какие то get - параметры, идущие после знака ? и так далее...
        }
        return wurl; // Возвращаем структуру. Url - распарсен!
    }

    // ФУНКЦИЯ ОТПРАВКИ ЗАПРОСОВ:
    // Функция принимает в качестве аргументов ссылку(url) и структуру (options). Ссылку в дальнейшем необходимо будет распарсить...
    inline std::vector<uint8_t> http_request(const std::wstring& url, const HTTP_REQ_OPTS& options = { L"GET", {} }) {
        const WURL& wurl = parse_url(url); // Парсим ссылку url.
        std::vector<uint8_t> answer = {}; // Создаем вектор, который будет ответом от сервера.
        if (HINTERNET hSession = WinHttpOpen(DEFAULT_USER_AGENT, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)) {
            if (HINTERNET hConnect = WinHttpConnect(hSession, wurl.host.c_str(), wurl.port, 0)) {
                if (HINTERNET hRequest = WinHttpOpenRequest(hConnect, options.method.c_str(), wurl.path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, wurl.dwFlags)) {
                    LPVOID data = WINHTTP_NO_REQUEST_DATA; // Объявляем переменную в которой будут находится данные, которые будут отсылаться.
                    const IN DWORD& data_len = options.data.size(); // Объявляем переменную, в которой будет содержаться длина отсылаемых данных.
                    if (data_len > 0) data = (LPVOID)options.data.data(); // Если длина > 0, тогда конвертируем содержимое в тип LPVOID.

                    // --------------------- БЛОК ПОДГОТОВКИ ЗАГОЛОВКОВ:
                    std::wstring tempstr(header.begin(), header.end());
                    WinHttpAddRequestHeaders(hRequest, tempstr.c_str(), headerSize, WINHTTP_ADDREQ_FLAG_REPLACE);
                    // -------------------------------------------------------------------------------------------

                    if (bool bResult = WinHttpSendRequest(hRequest, tempstr.c_str(), headerSize, data, data_len, data_len, 0)) { // Если запрос отправился...
                        if (bResult = WinHttpReceiveResponse(hRequest, nullptr)) { // Если ответ не равен пустоте...

                            // ------------------ БЛОК ЧТЕНИЯ ЗАГОЛОВКА:

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

                            char szBuffer[8192] = { 0 }; // создаем буфер проинициализированный 0.
                            DWORD dwByLesRead = 0; // и создаем переменную-указатель, которая будет хранить в себе то, сколько байтов прочитано.
                            do { // Цикл считывания данных с сервера:
                                bResult = WinHttpReadData(hRequest, szBuffer, sizeof(szBuffer), &dwByLesRead);
                                if (!bResult) break; // Если ничего не считали, то выходим из цикла считывания.
                                answer.insert(answer.end(), szBuffer, szBuffer + dwByLesRead); // Добавляем прочитанные данные в конец вектора, который мы в итоге вернем в качестве ответа от сервера.
                                ZeroMemory(szBuffer, dwByLesRead); // Затираем нулями прочитанную часть буфера.
                            } while (dwByLesRead);
                        }
                    }
                    // Закрываем запрос, подключение и сессию универсальным методом:
                    // В качестве аргумента передаем то, что будем закрывать, а крыть будем в порядке стека ес-но...
                    WinHttpCloseHandle(hRequest); // Закрываем запрос.
                }
                WinHttpCloseHandle(hConnect); // Закрываем подключение.
            }
            WinHttpCloseHandle(hSession); // Закрываем сессию.
        }
        return answer; // Возвращаем ответ от сервера.
    }

public:
    Http() { // Строка User Agent-а:
        DEFAULT_USER_AGENT = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:72.0) Gecko/20100101 Firefox/72.0";
        headerSize = 0;
    }
    // ---------------------------- МЕТОДЫ ЗАПРОСА:
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