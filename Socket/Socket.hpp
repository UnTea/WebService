#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <Address.hpp>


#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <ws2def.h>
#include <winsock2.h>
#include <ws2tcpip.h>


class Socket {
public:
    Socket() = default;
    Socket(const Socket&) = delete;
    Socket& operator= (const Socket&) = delete;
    Socket(Socket&& new_socket)  noexcept;
    explicit Socket(Address& address);

    void Bind() const;
    void Listen(int backlog = 8) const;
    void Send(std::string data) const;
    Socket Accept(Address& new_client_address) const;

    ~Socket();

private:
    SOCKET  m_socket{};
    Address m_address{};
};