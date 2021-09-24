#include <string>
#include <iostream>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ws2def.h>
#include <winsock2.h>
#include <ws2tcpip.h>


class Socket {
public:


private:

};

class Address {
public:
    explicit Address(uint16_t port_number) {
        addrinfo *res, hints {
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
            throw std::runtime_error("Fuck you!");
        }

        for (auto p = res; p != NULL; p = p->ai_next) {
            ai_family = p->ai_family;

            if (p->ai_family == AF_INET) {
                ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            } else {
                ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
            }
        }
    }

    friend std::ostream& operator<< (std::ostream& stream, const Address& address);

private:
    sockaddr_in  *ipv4;
    sockaddr_in6 *ipv6;
    int          ai_family;
};

std::ostream& operator<< (std::ostream& stream, const Address& address) {
    char ip_string[INET6_ADDRSTRLEN];
    void *ptr;

    if (address.ai_family == AF_INET) {
        ptr = &(address.ipv4->sin_addr);
    } else {
        ptr = &(address.ipv6->sin6_addr);
    }

    inet_ntop(address.ai_family, ptr, ip_string, sizeof(ip_string));
    stream << ip_string;

    return stream;
}


int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        exit(1);
    }

    try {
        Address address(80);
        std::cout << address << std::endl;
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

#if 0

    auto port_number = "80";
    const unsigned int back_log = 8;  // number of connections allowed on the incoming queue

    addrinfo hints, *res, *p;    // we need 2 pointers, res to hold and p to iterate over
    memset(&hints, 0, sizeof(hints));

    // for more explanation, man socket
    hints.ai_family = AF_UNSPEC;    // don't specify which IP version to use yet
    hints.ai_socktype = SOCK_STREAM;  // SOCK_STREAM refers to TCP, SOCK_DGRAM will be?
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(NULL, port_number, &hints, &res);
    if (result != 0) {
        std::cerr << gai_strerror(result) << "\n";
        return -2;
    }

    std::cout << "Detecting addresses" << std::endl;

    unsigned int address_count = 0;
    char ip_string[INET6_ADDRSTRLEN];    // ipv6 length makes sure both ipv4/6 addresses can be stored in this variable

    // Now since getaddrinfo() has given us a list of addresses
    // we're going to iterate over them and ask user to choose one
    // address for program to bind to
    for (p = res; p != NULL; p = p->ai_next) {
        void *address;
        std::string ip_ver;

        if (p->ai_family == AF_INET) {
            ip_ver = "IPv4";
            sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            address = &(ipv4->sin_addr);
            address_count++;
        } else {
            ip_ver = "IPv6";
            sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
            address = &(ipv6->sin6_addr);
            address_count++;
        }

        // convert IPv4 and IPv6 addresses from binary to text form
        inet_ntop(p->ai_family, address, ip_string, sizeof(ip_string));
        std::cout << "(" << address_count << ") " << ip_ver << " : " << ip_string << std::endl;
    }

    // if no addresses found :(
    if (!address_count) {
        std::cerr << "Found no host address to use\n";
        return -3;
    }

#endif

#if 0

    p = res;

    // let's create a new socket, socketFD is returned as descriptor
    // man socket for more information
    // these calls usually return -1 as result of some error
    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cerr << "Error while creating socket\n";
        freeaddrinfo(res);
        return -4;
    }

    // Let's bind address to our socket we've just created
    int bindR = bind(sockFD, p->ai_addr, p->ai_addrlen);
    if (bindR == -1) {
        std::cerr << "Error while binding socket\n";

        // if some error occurs, make sure to close socket and free resources
        closesocket(sockFD);
        freeaddrinfo(res);
        return -5;
    }

    // finally start listening for connections on our socket
    int listenR = listen(sockFD, back_log);
    if (listenR == -1) {
        std::cerr << "Error while Listening on socket\n";

        // if some error occurs, make sure to close socket and free resources
        closesocket(sockFD);
        freeaddrinfo(res);
        return -6;
    }

    // structure large enough to hold client's address
    sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    const std::string response = "Hello World";

    // a fresh infinite loop to communicate with incoming connections
    // this will take client connections one at a time
    // in further examples, we're going to use fork() call for each client connection
    while (1) {
        // accept call will give us a new socket descriptor
        int newFD = accept(sockFD, (sockaddr*) &client_addr, &client_addr_size);
        if (newFD == -1) {
            std::cerr << "Error while Accepting on socket\n";
            continue;
        }

        // send call sends the data you specify as second param and it's length as 3rd param, also returns how many bytes were actually sent
        auto bytes_sent = send(newFD, response.data(), response.length(), 0);
        closesocket(newFD);
    }

    closesocket(sockFD);
    freeaddrinfo(res);

#endif

    WSACleanup();

    return 0;
}