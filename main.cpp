#include <string>
#include <iostream>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ws2def.h>
#include <winsock2.h>
#include <ws2tcpip.h>


class Address {
public:
    explicit Address(uint16_t port_number) {
        addrinfo *res;
        addrinfo hints {
            .ai_flags    = AI_PASSIVE,
            .ai_family   = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
        };

        int result = getaddrinfo(NULL, std::to_string(port_number).c_str(), &hints, &res);
        if (result != 0) {
            std::cerr << gai_strerror(result) << "\n";
            throw std::runtime_error("Cannot detect addresses!");
        }

        if (res == nullptr) {
            throw std::runtime_error("Cannot initialise addinfo structure!");
        }

        for (auto p = res; p != NULL; p = p->ai_next) {
            family = p->ai_family;

            if (p->ai_family == AF_INET) {
                ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            } else {
                ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
            }
        }

    }

    [[nodiscard]] int GetFamily() const {
        return family;
    }

    [[nodiscard]] sockaddr_in* GetIPv4() {
        return ipv4;
    }

    [[nodiscard]] sockaddr_in6* GetIPv6() {
        return ipv6;
    }

    friend std::ostream& operator<< (std::ostream& stream, const Address& address);

private:
    sockaddr_in*  ipv4;
    sockaddr_in6* ipv6;
    int           family;
};

std::ostream& operator<< (std::ostream& stream, const Address& address) {
    char ip_string[INET6_ADDRSTRLEN];
    void *ptr;

    if (address.family == AF_INET) {
        ptr = &(address.ipv4->sin_addr);
    } else {
        ptr = &(address.ipv6->sin6_addr);
    }

    inet_ntop(address.family, ptr, ip_string, sizeof(ip_string));
    stream << ip_string;

    return stream;
}

class Socket {
public:
    explicit Socket(Address address) {
        SOCKET socket_file_descriptor = socket(address.GetFamily(), SOCK_STREAM, IPPROTO_TCP);

        if (socket_file_descriptor == -1) {
            throw std::runtime_error("Error while creating socket!");
        }

        int bind_result;

        if (address.GetFamily() == AF_INET) {
            bind_result = bind(socket_file_descriptor, reinterpret_cast<sockaddr*>(address.GetIPv4()), sizeof(sockaddr_in));
        } else {
            bind_result = bind(socket_file_descriptor, reinterpret_cast<sockaddr*>(address.GetIPv6()), sizeof(sockaddr_in6));
        }

        if (bind_result == -1) {
            closesocket(socket_file_descriptor);
            throw std::runtime_error("Error while binding socket!");
        }

        int listen_result = listen(socket_file_descriptor, 8);

        if (listen_result == -1) {
            closesocket(socket_file_descriptor);
            throw std::runtime_error("Error while Listening on socket!");
        }

        const std::string response = "Hello World";

        while (true) {
            SOCKET new_file_descriptor;
            socklen_t client_address_size;
            Address new_client(81);

            if (address.GetFamily() == AF_INET) {
                client_address_size = sizeof(sockaddr_in);
            } else {
                client_address_size = sizeof(sockaddr_in6);
            }

            if (address.GetFamily() == AF_INET) {
                new_file_descriptor = accept(socket_file_descriptor, reinterpret_cast<sockaddr*>(new_client.GetIPv4()), &client_address_size);
            } else {
                new_file_descriptor = accept(socket_file_descriptor, reinterpret_cast<sockaddr*>(new_client.GetIPv6()), &client_address_size);;
            }

            if (new_file_descriptor == -1) {
                throw std::runtime_error("Error while Accepting on socket!");
            }

            std::cout << new_client;

            int bytes_sent = send(new_file_descriptor, response.data(), response.length(), 0);
            std::cout << " count of sent bytes: " <<  bytes_sent << std::endl;
            closesocket(new_file_descriptor);
        }
        closesocket(socket_file_descriptor);
    }

private:

};


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
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    WSACleanup();

    return 0;
}