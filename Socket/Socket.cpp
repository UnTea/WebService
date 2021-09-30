#include "Socket.hpp"


Socket::Socket(Socket &&new_socket) noexcept {
    std::swap(m_socket, new_socket.m_socket);
}

Socket::Socket(Address &address)
: m_address(address)
{
    m_socket = ::socket(m_address.GetFamily(), SOCK_STREAM, IPPROTO_TCP);

    if (m_socket == -1) {
        throw std::runtime_error("Error while creating m_socket!");
    }
}

void Socket::Bind() const {
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

void Socket::Listen(int backlog) const {
    int listen_result = listen(m_socket, backlog);

    if (listen_result == -1) {
        throw std::runtime_error("Error while Listening on m_socket!");
    }
}

void Socket::Send(std::string data) const {
    if (send(m_socket, data.data(), (int) data.length(), 0) != data.size()) {
        throw std::runtime_error("Error while Sending on socket!");
    }
}

Socket Socket::Accept(Address &new_client_address) const {
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

Socket::~Socket() {
    closesocket(m_socket);
}
