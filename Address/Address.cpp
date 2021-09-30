#include "Address.hpp"


Address::Address(uint16_t port_number) {
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
            m_ipv4 = *reinterpret_cast<sockaddr_in *>(p->ai_addr);
        } else {
            m_ipv6 = *reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
        }
    }

    freeaddrinfo(res);
}

void Address::SetFamily(int family) {
    m_family = family;
}

int Address::GetFamily() const {
    return m_family;
}

sockaddr_in *Address::GetIPv4() {
    return &m_ipv4;
}

const sockaddr_in *Address::GetIPv4() const {
    return &m_ipv4;
}

sockaddr_in6 *Address::GetIPv6() {
    return &m_ipv6;
}

const sockaddr_in6 *Address::GetIPv6() const {
    return &m_ipv6;
}

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