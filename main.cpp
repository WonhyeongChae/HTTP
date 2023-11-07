/**********************************************************************
 * \file   CS260_Assignment_3.cpp
 * \brief  Main file for the CS260_Assignment_3 HTTP Proxy Server.
 *
 * \author Wonhyeong Chae
 * Login:  w.chae
 * Email:  w.chae@digipen.edu
 * \date   November 2nd, 2023
 *********************************************************************/

#include "HTTP.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    try
    {
        Server server(argv[1]);
        server.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
