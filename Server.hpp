#pragma once

#include <string>
#include <thread>
#include <vector>

#include "ClientHandler.hpp"
#include "ThreadPool.hpp"

class Server
{
public:
    Server(const std::string& port);
    ~Server();
    void Run();

private:
    std::string port;
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;

    void AcceptConnections();
    void InitializeWinsock();
    void CreateListenSocket();
    void BindAndListen();
    void CleanUp();
};