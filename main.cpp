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
            std::cerr << gai_strerror(result) << std::endl;
            throw std::runtime_error("Cannot detect addresses!");
        }

        if (res == nullptr) {
            throw std::runtime_error("Cannot initialise addinfo structure!");
        }

        for (auto p = res; p != nullptr; p = p->ai_next) {
            m_family = p->ai_family;

            if (p->ai_family == AF_INET) {
                m_ipv4 = *reinterpret_cast<sockaddr_in*>(p->ai_addr);
            } else {
                m_ipv6 = *reinterpret_cast<sockaddr_in6*>(p->ai_addr);
            }
        }

    }

    void SetFamily(int family) {
        m_family = family;
    }

    [[nodiscard]] int GetFamily() const {
        return m_family;
    }

    [[nodiscard]] sockaddr_in* GetIPv4() {
        return &m_ipv4;
    }

    [[nodiscard]] const sockaddr_in* GetIPv4() const {
        return &m_ipv4;
    }

    [[nodiscard]] sockaddr_in6* GetIPv6() {
        return &m_ipv6;
    }

    [[nodiscard]] const sockaddr_in6* GetIPv6() const {
        return &m_ipv6;
    }

    friend std::ostream& operator<< (std::ostream& stream, const Address& address);

private:
    union {
        sockaddr_in m_ipv4;
        sockaddr_in6 m_ipv6;
    };
    int m_family;
};

std::ostream& operator<< (std::ostream& stream, const Address& address) {
    char ip_string[INET6_ADDRSTRLEN];
    const void *ptr;

    if (address.m_family == AF_INET) {
        ptr = &(address.m_ipv4.sin_addr);
    } else {
        ptr = &(address.m_ipv6.sin6_addr);
    }

    inet_ntop(address.m_family, ptr, ip_string, sizeof(ip_string));
    stream << ip_string;

    return stream;
}

class Socket {
public:
    Socket() = default;

    explicit Socket(Address& address)
    : m_address(address)
    {
        m_socket = ::socket(m_address.GetFamily(), SOCK_STREAM, IPPROTO_TCP);

        if (m_socket == -1) {
            throw std::runtime_error("Error while creating m_socket!");
        }
    }

    void Bind() const {
        int bind_result;

        if (m_address.GetFamily() == AF_INET) {
            bind_result = bind(m_socket, reinterpret_cast<const sockaddr*>(m_address.GetIPv4()), sizeof(sockaddr_in));
        } else {
            bind_result = bind(m_socket, reinterpret_cast<const sockaddr*>(m_address.GetIPv6()), sizeof(sockaddr_in6));
        }

        if (bind_result == -1) {
            throw std::runtime_error("Error while binding m_socket!");
        }
    }

    void Listen(int backlog = 8) const {
        int listen_result = listen(m_socket, backlog);

        if (listen_result == -1) {
            throw std::runtime_error("Error while Listening on m_socket!");
        }
    }

    Socket Accept(Address& new_client_address) const {
        SOCKET new_socket;
        socklen_t client_address_size;

        new_client_address.SetFamily(m_address.GetFamily());
        if (new_client_address.GetFamily() == AF_INET) {
            client_address_size = sizeof(sockaddr_in);
            new_socket = accept(m_socket, reinterpret_cast<sockaddr*>(new_client_address.GetIPv4()), &client_address_size);
        } else {
            client_address_size = sizeof(sockaddr_in6);
            new_socket = accept(m_socket, reinterpret_cast<sockaddr*>(new_client_address.GetIPv6()), &client_address_size);
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
    SOCKET m_socket{};
    Address m_address{};
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