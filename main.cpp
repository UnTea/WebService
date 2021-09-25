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
    Address() = default;

    explicit Address(uint16_t port_number) {
        addrinfo *res;
        addrinfo hints {
            .ai_flags    = AI_PASSIVE,
            .ai_family   = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
        };

        int result = getaddrinfo(nullptr, std::to_string(port_number).c_str(), &hints, &res);
        if (result != 0) {
            std::cerr << gai_strerror(result) << "\n";
            throw std::runtime_error("Cannot detect addresses!");
        }

        if (res == nullptr) {
            throw std::runtime_error("Cannot initialise addinfo structure!");
        }

        for (auto p = res; p != nullptr; p = p->ai_next) {
            m_family = p->ai_family;

            if (p->ai_family == AF_INET) {
                m_ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            } else {
                m_ipv6 = reinterpret_cast<sockaddr_in6*>(p->ai_addr);
            }
        }

    }

    [[nodiscard]] int GetFamily() const {
        return m_family;
    }

    [[nodiscard]] sockaddr_in* GetIPv4() {
        return m_ipv4;
    }

    [[nodiscard]] const sockaddr_in* GetIPv4() const {
        return m_ipv4;
    }

    [[nodiscard]] sockaddr_in6* GetIPv6() {
        return m_ipv6;
    }

    [[nodiscard]] const sockaddr_in6* GetIPv6() const {
        return m_ipv6;
    }

    friend std::ostream& operator<< (std::ostream& stream, const Address& address);

private:
    sockaddr_in*  m_ipv4;
    sockaddr_in6* m_ipv6;
    int           m_family;
};

std::ostream& operator<< (std::ostream& stream, const Address& address) {
    char ip_string[INET6_ADDRSTRLEN];
    void *ptr;

    if (address.m_family == AF_INET) {
        ptr = &(address.m_ipv4->sin_addr);
    } else {
        ptr = &(address.m_ipv6->sin6_addr);
    }

    inet_ntop(address.m_family, ptr, ip_string, sizeof(ip_string));
    stream << ip_string;

    return stream;
}

class Socket {
public:
    Socket() = default;

    explicit Socket(int family) {
        m_socket = ::socket(family, SOCK_STREAM, IPPROTO_TCP);

        if (m_socket == -1) {
            throw std::runtime_error("Error while creating m_socket!");
        }
    }

    void Bind(const Address& address) const {
        int bind_result;

        if (address.GetFamily() == AF_INET) {
            bind_result = bind(m_socket, reinterpret_cast<const sockaddr*>(address.GetIPv4()), sizeof(sockaddr_in));
        } else {
            bind_result = bind(m_socket, reinterpret_cast<const sockaddr*>(address.GetIPv6()), sizeof(sockaddr_in6));
        }

        if (bind_result == -1) {
            throw std::runtime_error("Error while binding m_socket!");
        }
    }

    void Listen(int backlog) const {
        int listen_result = listen(m_socket, backlog);

        if (listen_result == -1) {
            throw std::runtime_error("Error while Listening on m_socket!");
        }
    }

    //TODO Нужно в конструкторе по умолчанию у Address принимать family, желательно делать это из Accept
    Socket Accept(Address& new_client_address) const {
        SOCKET new_socket;
        socklen_t client_address_size;

        if (new_client_address.GetFamily() == AF_INET) {
            client_address_size = sizeof(sockaddr_in);
        } else {
            client_address_size = sizeof(sockaddr_in6);
        }

        if (new_client_address.GetFamily() == AF_INET) {
            new_socket = accept(m_socket, reinterpret_cast<sockaddr*>(new_client_address.GetIPv4()), &client_address_size);
        } else {
            new_socket = accept(m_socket, reinterpret_cast<sockaddr*>(new_client_address.GetIPv6()), &client_address_size);;
        }

        if (new_socket == -1) {
            throw std::runtime_error("Error while Accepting on socket!");
        }

        Socket socket{};
        socket.m_socket = new_socket;

        return std::move(socket);
    }

    void Send(std::string data) const {
        if (send(m_socket, data.data(), (int) data.length(), 0) != data.size()) {
            throw std::runtime_error("Error while Sending on socket!");
        }
    }

    Socket(const Socket&) = delete;

    Socket& operator= (const Socket&) = delete;

    Socket(Socket&& new_socket)  noexcept {
        std::swap(m_socket, new_socket.m_socket);
    }

    ~Socket() {
        closesocket(m_socket);
    }

private:
    SOCKET  m_socket{};
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
        Socket socket(address.GetFamily());
        socket.Bind(address);
        socket.Listen(8);

        while (true) {
            Address new_client_address{};
            Socket new_client = socket.Accept(new_client_address);
            new_client.Send("Hello world!");
            std::cout << new_client_address << std::endl;
        }

    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    WSACleanup();
}