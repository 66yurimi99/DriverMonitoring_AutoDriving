#include "Server.h"

Server::Server(const wstring& url)
{
    this->url = url;
    initialize();
}

Server::~Server()
{
    cleanup();
}


/*** 1. 서버 통신 초기화 ***/
void Server::initialize()
{
    hInternet = InternetOpen(L"HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
    {
        cerr << "InternetOpen failed." << endl;
    }
}


/*** 2. 서버 통신 종료 ***/
void Server::cleanup()
{
    if (hInternet)
    {
        InternetCloseHandle(hInternet);
    }
}


/*** 3. HTTP GET 요청 전송 ***/
// 3-1. 데이터 조회 : 쿼리 없음 (PUBLIC)
bool Server::sendGETRequest(wstring& response)
{
    return sendGETRequest(L"", response);
}

// 3-2. 데이터 업데이트 (쿼리가 있는 경우)
bool Server::sendGETRequest(const wstring& query, wstring& response)
{
    if (!hInternet)
    {
        return false;
    }

    // Request to web server
    HINTERNET hConnect = InternetOpenUrl(hInternet, (url + query).c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect)
    {
        cerr << "InternetOpenUrl failed." << endl;
        return false;
    }

    DWORD bytesRead;
    wchar_t buffer[4096];

    // Get response data
    wstring responseData;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
    {
        responseData += wstring(buffer, bytesRead);
    }
    response = responseData;

    // Clean up
    InternetCloseHandle(hConnect);

    return true;
}
