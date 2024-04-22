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
    ip_(ip), port_(port),connectTimeout_(connectTimeout), readTimeout_(readTimeout){}

TcpConnect::~TcpConnect() {}

void TcpConnect::EstablishConnection() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("Не удалось создать сокет");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // Обнуление структуры
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, ip_.c_str(), &serv_addr.sin_addr) <= 0) {
        throw std::runtime_error("Неверный адрес IP");
    }


    struct timeval tv;
    tv.tv_sec = connectTimeout_.count() / 1000;
    tv.tv_usec = (connectTimeout_.count() % 1000) * 1000;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    if (connect(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Не удалось установить соединение");
    }
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
