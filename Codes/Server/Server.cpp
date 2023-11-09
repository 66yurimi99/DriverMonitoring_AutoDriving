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


/*** 1. ���� ��� �ʱ�ȭ ***/
void Server::initialize()
{
    hInternet = InternetOpen(L"HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
    {
        cerr << "InternetOpen failed." << endl;
    }
}


/*** 2. ���� ��� ���� ***/
void Server::cleanup()
{
    if (hInternet)
    {
        InternetCloseHandle(hInternet);
    }
}


/*** 3. HTTP GET ��û ���� ***/
// 3-1. ������ ��ȸ : ���� ���� (PUBLIC)
bool Server::sendGETRequest(wstring& response)
{
    return sendGETRequest(L"", response);
}

// 3-2. ������ ������Ʈ (������ �ִ� ���)
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
