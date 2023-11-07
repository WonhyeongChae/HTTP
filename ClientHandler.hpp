#pragma once

#include <string>

#include "Socket.hpp"

class ClientHandler
{
public:
    ClientHandler(SOCKET clientSocket);
    void HandleRequest();

private:
    SOCKET clientSocket;

    void ParseRequest();
    void ResolveHostName();
    void ConnectToHost();
    void ForwardRequest();
    void ReceiveResponse();
    void CleanUp();
};