#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

using boost::asio::ip::tcp;

std::string receive(tcp::socket& socket)
{
    boost::asio::streambuf buf;
    boost::system::error_code ec;
    read_until(socket, buf, "\n", ec);
    return boost::asio::buffer_cast<const char*>(buf.data());
}

void send(tcp::socket& socket, const std::string& message)
{
    boost::system::error_code ec;
    boost::asio::write(socket, boost::asio::buffer(message + "\n"), ec);
}

void get_name(std::string& name)
{
    while(true)
    {
        std::cout << "Enter your name: ";

        getline(std::cin, name);
        size_t found = name.find(" ");

        if (found != std::string::npos)
        {
            std::cout << "Name cannot have any spaces." << std::endl;
            continue;
        }
        if (name.size() == 0)
        {
            std::cout << "Name has to be at least one characters long." << std::endl;
            continue;
        }

        std::cout << "Client name: " << name << std::endl;
        break;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ./client <host>" << std::endl;
            return 1;
        }

        std::cout << "Command:\n" << std::endl;
        std::cout << "<name> <message>" << std::endl;
        std::cout << "\t • Send <message> to <name>\n" << std::endl ;

        std::string client_name, message, response;
        get_name(client_name);

        boost::asio::io_context io_context;

        // A resolver takes a host name and service name and turns them into a list of endpoints.
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "8080");

        // Create and connect the socket.
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        send(socket, "init " + client_name); // send client name

        // Set up struct for poll()
        struct pollfd fds[2];
        fds[0].fd = 0; // stdin
        fds[0].events = POLLIN;

        fds[1].fd = socket.native_handle();
        fds[1].events = POLLIN;

        while(true)
        {
            poll(fds, 2, -1);

            if (fds[0].revents != 0) // stdin
            {
                getline(std::cin, message);
                send(socket, "send " + message);
            }
            else if (fds[1].revents != 0) // socket
            {
                response = receive(socket);
                std::cout << response;

                if (response == "error: duplicate name\n")
                {
                    return 1;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}