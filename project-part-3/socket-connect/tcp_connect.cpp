#include "tcp_connect.h"
#include "byte_tools.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>
#include <vector>

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout):
    ip_(ip), port_(port),connectTimeout_(connectTimeout), readTimeout_(readTimeout){}

TcpConnect::~TcpConnect() {}

void TcpConnect::EstablishConnection() {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &serv_addr.sin_addr); // Преобразование IP адреса
    connect(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); // Подключение к серверу
}

void TcpConnect::SendData(const std::string& data) const {
    send(sock_, data.c_str(), data.size(), 0);
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    if (bufferSize) {
        std::vector<char> buffer(bufferSize);
        ssize_t bytes_received = recv(sock_, buffer.data(), bufferSize, 0);
        if (bytes_received > 0) {
            return std::string(buffer.data(), bytes_received);
        }
        return "";
    }

    char lengthBuffer[4];
    ssize_t bytes_received = recv(sock_, lengthBuffer, 4, 0);
    if (bytes_received != 4) {
        throw std::runtime_error("Ошибка при чтении длины данных");
    }
    int length = BytesToInt(lengthBuffer);
    std::vector<char> res(length);
    bytes_received = recv(sock_, res.data(), length, 0);
    if (bytes_received > 0) {
        return std::string(res.data(), bytes_received);
    }
    return "";
}

void TcpConnect::CloseConnection() {
    close(sock_);
}

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}
