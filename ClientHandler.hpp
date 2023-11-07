#pragma once

#include <string>

#include "Socket.hpp"

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
        // Implement parsing of the HTTP request to extract the Host header and the requested URL
    }
    void ResolveHostName()
    {
        // Implement DNS resolution or IP address parsing for the host specified in the HTTP request
    }
    void ConnectToHost()
    {
        // Implement connection logic to the target web server
    }
    void ForwardRequest()
    {
        // Implement forwarding of the HTTP request to the target web server
    }
    void ReceiveResponse()
    {
        // Implement receiving the HTTP response from the target web server and forwarding it to the client
    }
    void CleanUp()
    {
        // Implement cleanup of resources, such as closing sockets
    }
};