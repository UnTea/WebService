#include <Address.hpp>
#include <Socket.hpp>


#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <ws2def.h>
#include <winsock2.h>
#include <ws2tcpip.h>


int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        exit(1);
    }

    try {
        Address address(80);
        std::cout << address << std::endl;
        Socket socket(address);
        socket.Bind();
        socket.Listen();

        while (true) {
            Address new_client_address{};
            Socket new_client = socket.Accept(new_client_address);
            new_client.Send("HTTP/1.1 200 OK\n"
                            "Date: Sun, 10 Oct 2010 23:26:07 GMT\n"
                            "Server: Apache/2.2.8 (Ubuntu) mod_ssl/2.2.8 OpenSSL/0.9.8g\n"
                            "Last-Modified: Sun, 26 Sep 2010 22:04:35 GMT\n"
                            "ETag: \"45b6-834-49130cc1182c0\"\n"
                            "Accept-Ranges: bytes\n"
                            "Content-Length: 12\n"
                            "Connection: close\n"
                            "Content-Type: text/html\n"
                            "\n"
                            "Hello world!");
            std::cout << new_client_address << std::endl;
        }

    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    WSACleanup();
}