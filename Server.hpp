#pragma once

#include <string>
#include <thread>
#include <vector>

#include "ClientHandler.hpp"
#include "ThreadPool.hpp"

class Server
{
public:
    Server(const std::string& port) : port(port), listenSocket(INVALID_SOCKET)
    {
        InitializeWinsock();
        CreateListenSocket();
        BindAndListen();
    }

    ~Server()
    {
        CleanUp();
    }

    void Run()
    {
        while (true)
        {
            AcceptConnections();
            // Sleep to prevent CPU spinning
            Sleep(100);
        }
    }

private:
    std::string port;
    SOCKET listenSocket;
    std::vector<std::thread> clientThreads;

    void AcceptConnections();
    void InitializeWinsock()
    {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            exit(1);
        }
    }
    void CreateListenSocket()
    {
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET)
        {
            std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(1);
        }

        // Set the socket to non-blocking mode
        u_long mode = 1;
        ioctlsocket(listenSocket, FIONBIO, &mode);
    }
    void BindAndListen()
    {
        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = inet_addr("127.0.0.1");
        service.sin_port = htons(std::stoi(port));

        if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
        {
            std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            exit(1);
        }

        if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        {
            std::cerr << "Error at listen(): " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            exit(1);
        }
    }
    void CleanUp()
    {
        // Close the listening socket if it's valid
        if (listenSocket != INVALID_SOCKET)
        {
            closesocket(listenSocket);
        }

        // Clean up Winsock
        WSACleanup();

        // Join all threads
        for (auto& thread : clientThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }
};