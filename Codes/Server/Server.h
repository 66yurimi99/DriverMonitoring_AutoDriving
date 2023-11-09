#pragma once
#include <iostream>
#include <windows.h>
#include <wininet.h>

using namespace std;

class Server {
public:
    Server(const wstring& url);
    ~Server();

    bool sendGETRequest(wstring& response);
    bool sendGETRequest(const wstring& query, wstring& response);

private:
    HINTERNET hInternet;
    wstring url;

    void initialize();
    void cleanup();
};
