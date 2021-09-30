#pragma once


#include <string>
#include <cstdint>
#include <iostream>


#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <ws2def.h>
#include <winsock2.h>
#include <ws2tcpip.h>


class Address {
public:
    Address() = default;
    explicit Address(uint16_t port_number);

    void SetFamily(int family);
    [[nodiscard]] int GetFamily() const;
    [[nodiscard]] sockaddr_in* GetIPv4();
    [[nodiscard]] const sockaddr_in* GetIPv4() const;
    [[nodiscard]] sockaddr_in6* GetIPv6();
    [[nodiscard]] const sockaddr_in6* GetIPv6() const;

    friend std::ostream& operator<< (std::ostream& stream, const Address& address);

private:
    union {
        sockaddr_in  m_ipv4;
        sockaddr_in6 m_ipv6;
    };
    int m_family;
};