#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class Socket
{
public:
    Socket() : tcpSocket(INVALID_SOCKET)
    {
        Initialize();
        CreateSocket();
    }

    ~Socket()
    {
        CleanUp();
    }

    void Connect(const std::string& address, const std::string& port)
    {
        SetDestinationAddress(address, port);
        ConnectSocket();
    }

    void Send(const std::string& message)
    {
        SendBuffer(message.c_str());
    }

    std::string Receive()
    {
        return RecvBuffer();
    }

private:
    WSADATA wsaData;
    SOCKET tcpSocket;
    sockaddr_in destinationAddr;

    void Initialize()
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            std::cerr << "Error in WSAStartup: " << WSAGetLastError() << std::endl;
        }
    }

    void CreateSocket()
    {
        tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (tcpSocket == INVALID_SOCKET)
        {
            std::cerr << "Error creating socket " << WSAGetLastError() << std::endl;
            WSACleanup();
            throw std::runtime_error("Socket creation failed");
        }

        u_long mode = 1;  // 1 to enable non-blocking socket
        ioctlsocket(tcpSocket, FIONBIO, &mode);
    }

    void SetDestinationAddress(const std::string& address, const std::string& port)
    {
        struct addrinfo hints, * res;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int result = getaddrinfo(address.c_str(), port.c_str(), &hints, &res);
        if (result != 0)
        {
            std::cerr << "getaddrinfo error: " << gai_strerror(result) << std::endl;
            WSACleanup();
        }

        destinationAddr = *(struct sockaddr_in*)res->ai_addr;
        freeaddrinfo(res);
    }

    void ConnectSocket()
    {
        int result = connect(tcpSocket, reinterpret_cast<sockaddr*>(&destinationAddr), sizeof(destinationAddr));
        if (result == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                std::cerr << "Error at socket connect: " << WSAGetLastError() << std::endl;
                closesocket(tcpSocket);
                WSACleanup();
                throw std::runtime_error("Socket connect failed");
            }
        }
    }

    void SendBuffer(const char* message)
    {
        int sendResult;
        do
        {
            sendResult = send(tcpSocket, message, static_cast<int>(strlen(message)), 0);
            if (sendResult == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK && error != WSAENOTCONN)
                {
                    std::cerr << "Send failed with error: " << error << std::endl;
                    closesocket(tcpSocket);
                    WSACleanup();
                    throw std::runtime_error("Send buffer failed");
                }
            }
        } while (sendResult == SOCKET_ERROR && (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAENOTCONN));

        if (shutdown(tcpSocket, SD_SEND) == SOCKET_ERROR)
        {
            std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
            closesocket(tcpSocket);
            WSACleanup();
            throw std::runtime_error("Shutdown failed");
        }
    }

    std::string RecvBuffer()
    {
        std::string response;
        constexpr int BUFFER_LENGTH = 1500;
        char recvBuffer[BUFFER_LENGTH] = { 0 };

        while (true)
        {
            int result = recv(tcpSocket, recvBuffer, BUFFER_LENGTH - 1, 0);
            if (result > 0)
            {
                response.append(recvBuffer, result);
                std::fill_n(recvBuffer, BUFFER_LENGTH, 0);
            }
            else if (result == 0)
            {
                break;
            }
            else
            {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK)
                {
                    std::cerr << "recv failed: " << error << std::endl;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        return response;
    }

    void CleanUp()
    {
        if (tcpSocket != INVALID_SOCKET)
        {
            if (closesocket(tcpSocket) != 0)
            {
                std::cerr << "Error in close socket: " << WSAGetLastError() << std::endl;
            }
        }
        WSACleanup();
    }
};
