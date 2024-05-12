#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>
#include <winhttp.h>

typedef enum {
    mp4,
    mp3
} extension;

std::string http_get(const WCHAR *URL) {
    DWORD dwSize = 0;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
    URL_COMPONENTS urlComponents = { 0 };
    WCHAR szHostName[256], szUrlPath[2048];
    LPSTR pszOutBuffer;
    DWORD dwDownloaded = 0;

    hSession = WinHttpOpen(L"UserAgent/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
    urlComponents.lpszHostName = szHostName;
    urlComponents.dwHostNameLength = sizeof(szHostName) / sizeof(WCHAR);
    urlComponents.lpszUrlPath = szUrlPath;
    urlComponents.dwUrlPathLength = sizeof(szUrlPath) / sizeof(WCHAR);
    bResults = WinHttpCrackUrl(URL, wcslen(URL), 0, &urlComponents);

    if (hSession)
        hConnect = WinHttpConnect(hSession, szHostName, urlComponents.nPort, 0);

    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", szUrlPath,
            NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            (INTERNET_SCHEME_HTTPS == urlComponents.nScheme) ? WINHTTP_FLAG_SECURE : 0);

    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
            L"Accept: */*\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n", -1L,
            WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0);

    if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

    std::string output;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                printf("Error %lu in WinHttpQueryDataAvailable.\n", GetLastError());

            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) dwSize = 0;
            else {
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
                    printf("Error %lu in WinHttpReadData.\n", GetLastError());
                else {
                    std::string str_temp{pszOutBuffer};
                    if (!str_temp.empty()) output += str_temp;
                }

                delete[] pszOutBuffer;
            }
        } while (dwSize > 0);
    }

    if (!bResults) printf("Error %ld has occurred.\n", GetLastError());

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return output;
}

const char *find(const char *text, const char *target) {
    size_t tarlen = strlen(target);
    for (const char *p = text; *p; p += 1) if (strncmp(p, target, tarlen) == 0) return p;
    return NULL;
}

std::vector<std::string> selection(const char *string, extension ext, const std::string &buf) {
    std::vector<std::string> vec;
    char *buffer = new char[buf.size()];
    for (size_t i = 0; i < buf.size(); i++) buffer[i] = buf[i];
    char *tmp = buffer;
    const char *ext_string[2] = { ".mp4", ".mp3" };
    char *found_ext = NULL;
    size_t pos, ext_pos;
    std::string filename;
    const char *found1, *found2;
    switch (ext) {
    case mp4:
        found_ext = (char *)ext_string[mp4];
        break;
    case mp3:
        found_ext = (char *)ext_string[mp3];
        break;
    }
    do {
        filename = "";
        found1 = find(tmp, string);
        found2 = find(tmp, found_ext);
        if (found1 == NULL || found2 == NULL) break;
        pos = found1 - buffer;
        ext_pos = found2 - buffer + strlen(found_ext);
        for (size_t i = pos; i < ext_pos; i++) filename += buffer[i];
        vec.push_back(filename);
        tmp = (char *)found2 + strlen(found_ext) + 1;
    } while (1);
    delete[] buffer;
    return vec;
}

int wget(const std::string &URL) {
    DWORD dwSize = 0;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
    URL_COMPONENTS urlComponents = { 0 };
    WCHAR szHostName[256], szUrlPath[2048];
    BYTE *pszOutBuffer;
    DWORD dwDownloaded = 0;

    hSession = WinHttpOpen(L"UserAgent/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    std::wstring tmp = std::wstring(URL.begin(), URL.end());
    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
    urlComponents.lpszHostName = szHostName;
    urlComponents.dwHostNameLength = sizeof(szHostName) / sizeof(WCHAR);
    urlComponents.lpszUrlPath = szUrlPath;
    urlComponents.dwUrlPathLength = sizeof(szUrlPath) / sizeof(WCHAR);
    bResults = WinHttpCrackUrl(tmp.c_str(), wcslen(tmp.c_str()), 0, &urlComponents);

    if (hSession)
        hConnect = WinHttpConnect(hSession, szHostName, urlComponents.nPort, 0);

    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", szUrlPath,
            NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            (INTERNET_SCHEME_HTTPS == urlComponents.nScheme) ? WINHTTP_FLAG_SECURE : 0);

    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
            L"Accept: */*\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n", -1L,
            WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0);

    if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

    char *filename = strrchr(URL.c_str(), '/') + 1;
    FILE *pFile = fopen(filename, "wb");
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                printf("Error %lu in WinHttpQueryDataAvailable.\n", GetLastError());

            pszOutBuffer = new unsigned char[dwSize + 1];
            if (!pszOutBuffer) dwSize = 0;
            else {
                ZeroMemory(pszOutBuffer, dwSize + 1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
                    printf("Error %lu in WinHttpReadData.\n", GetLastError());
                else {
                    fwrite(pszOutBuffer, (size_t)dwDownloaded, (size_t)1, pFile);
                }

                delete[] pszOutBuffer;
            }
        } while (dwSize > 0);
    }

    fclose (pFile);

    if (!bResults) printf("Error %ld has occurred.\n", GetLastError());

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return 0;
}

int main() {
    std::string str = http_get(L"https://www.nurumayu.net/twidouga/real1.php");
    std::vector<std::string> vec = selection("https://video.twimg.com", mp4, str);
    if (vec.size() <= 0) {
        puts("fails to extract");
        return 1;
    }
    for (size_t i = 0; i < vec.size(); i++)
        wget(vec[i].c_str()) ? puts("Error!") : printf("%s Downloaded!\n", vec[i].c_str());
    return 0;
}

