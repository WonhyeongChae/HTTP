// Combined.hpp
#pragma once

#include <iostream>

#include <string>
#include <array>
#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <functional>

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

// Forward declarations
class Server;
class ClientHandler;
class ThreadPool;

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

    void AcceptConnections()
    {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        while (true)
        {
            // Accept a client socket
            SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSocket == INVALID_SOCKET)
            {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK)
                {
                    std::cerr << "accept failed: " << error << std::endl;
                    // Handle error as appropriate
                }
                // If the error is WSAEWOULDBLOCK, we simply didn't have a connection to accept
                break;
            }
            else
            {
                // Successfully accepted a client, handle it in a separate thread
                clientThreads.emplace_back([clientSocket]()
                    {
                        ClientHandler handler(clientSocket);
                        handler.HandleRequest();
                    });
            }
        }
    }

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

class ClientHandler
{
public:
    ClientHandler(SOCKET clientSocket) : clientSocket(clientSocket) {}

    void HandleRequest()
    {
        // This function will be called in a separate thread for each client
        ParseRequest();
        ResolveHostName();
        ConnectToHost();
        ForwardRequest();
        ReceiveResponse();
        CleanUp();
    }

private:
    SOCKET clientSocket;

    void ParseRequest()
    {
        std::array<char, 4096> buffer{};
        int bytesReceived = recv(clientSocket, buffer.data(), buffer.size(), 0);
        if (bytesReceived <= 0)
        {
            // Handle error or connection closed
            return;
        }

        // Assuming the request is null-terminated; otherwise, we need to add a null terminator
        std::string requestStr(buffer.data(), bytesReceived);
        // Extract the Host header and the requested URL
        // ... (parsing logic here)
    }

    void ResolveHostName()
    {
        // Assume hostname is extracted to a std::string variable named 'hostname'
        struct addrinfo hints {}, * res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;

        // Resolve the domain name into a list of addresses
        int status = getaddrinfo(hostname.c_str(), "http", &hints, &res);
        if (status != 0)
        {
            // Handle error
            return;
        }
        // ... (use the resolved IP here)
    }

    void ConnectToHost()
    {
        // Assume we have a sockaddr structure filled out called 'serverAddr'
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET)
        {
            // Handle error
            return;
        }

        // Connect to the server
        int connResult = connect(serverSocket, &serverAddr, sizeof(serverAddr));
        if (connResult == SOCKET_ERROR)
        {
            // Handle error
            closesocket(serverSocket);
            return;
        }
        // ... (continue with the connection)
    }

    void ForwardRequest()
    {
        // Forward the request to the server
        // Assume we have the request in a std::string named 'httpRequest'
        int sendResult = send(serverSocket, httpRequest.c_str(), httpRequest.size() + 1, 0);
        if (sendResult == SOCKET_ERROR)
        {
            // Handle error
            return;
        }
        // ... (forward the request)
    }

    void ReceiveResponse()
    {
        std::array<char, 4096> buffer{};
        // Receive the response from the server and forward it to the client
        // ... (receiving and forwarding logic here)
    }

    void CleanUp()
    {
        // Close the client socket
        closesocket(clientSocket);
    }
};

class ThreadPool
{
public:
    ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back(&ThreadPool::WorkerThread, this);
        }
    }
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void EnqueueJob(std::function<void()> job)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            jobs.push(job);
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void WorkerThread()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stop || !jobs.empty(); });
                if (stop && jobs.empty())
                {
                    return;
                }
                job = jobs.front();
                jobs.pop();
            }
            job();
        }
    }
};
