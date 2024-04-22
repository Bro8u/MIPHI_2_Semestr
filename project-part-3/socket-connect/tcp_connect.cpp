#include "tcp_connect.h"
#include "byte_tools.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>
#include <cstring>
#include <iostream>

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout):
        ip_(ip), port_(port), connectTimeout_(connectTimeout), readTimeout_(readTimeout) {}

TcpConnect::~TcpConnect() {
    CloseConnection();
}

void TcpConnect::EstablishConnection() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("Не удалось создать сокет");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &serv_addr.sin_addr) <= 0) {
        close(sock_);  // Clean up the socket before throwing
        throw std::runtime_error("Неверный адрес IP");
    }

    if (connect(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock_);  // Clean up the socket before throwing
        throw std::runtime_error("Не удалось установить соединение");
    }
}

void TcpConnect::SendData(const std::string& data) const {
    if (send(sock_, data.c_str(), data.size(), 0) < 0) {
        throw std::runtime_error("Ошибка при отправке данных");
    }
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    if (bufferSize) {
        std::vector<char> buffer(bufferSize);
        ssize_t bytes_received = recv(sock_, buffer.data(), bufferSize, 0);
        if (bytes_received < 0) {
            throw std::runtime_error("Ошибка при получении данных");
        } else if (bytes_received == 0) {
            throw std::runtime_error("Соединение закрыто");
        }
        return std::string(buffer.data(), bytes_received);
    }

    char lengthBuffer[4];
    ssize_t bytes_received = recv(sock_, lengthBuffer, 4, 0);
    if (bytes_received != 4) {
        throw std::runtime_error("Ошибка при чтении длины данных");
    }
    int length = BytesToInt(lengthBuffer);
    std::vector<char> res(length);
    bytes_received = recv(sock_, res.data(), length, 0);
    if (bytes_received < 0) {
        throw std::runtime_error("Ошибка при получении данных");
    } else if (bytes_received == 0) {
        throw std::runtime_error("Соединение закрыто");
    }
    return std::string(res.data(), bytes_received);
}

void TcpConnect::CloseConnection() {
    if (sock_ != -1) {
        close(sock_);
        sock_ = -1;  // Invalidate the socket descriptor after closing
    }
}

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}
