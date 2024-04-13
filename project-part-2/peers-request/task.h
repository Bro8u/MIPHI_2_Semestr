#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>
#include <iostream>

struct BencodeValue;
using BencodeInt = int64_t;
using BencodeString = std::string;
using BencodeList = std::vector<std::shared_ptr<BencodeValue>>;
using BencodeDict = std::map<BencodeString, std::shared_ptr<BencodeValue>>;

struct BencodeValue : public std::variant<std::monostate, BencodeInt, BencodeString, BencodeList, BencodeDict> {
    using variant::variant;
};

BencodeValue ParseBencode(const std::string& contents, size_t& pos);
BencodeValue ParseList(const std::string& contents, size_t& pos);
BencodeValue ParseDict(const std::string& contents, size_t& pos);
BencodeInt ParseInteger(const std::string& contents, size_t& pos);
BencodeString ParseString(const std::string& contents, size_t& pos);

BencodeInt ParseInteger(const std::string& contents, size_t& pos) {
    std::string res = "";
    while (contents[pos] != 'e') {
        res += contents[pos];
        ++pos;
    }
    ++pos;
    return std::stoll(res);
}
BencodeString ParseString(const std::string& contents, size_t& pos) {
    size_t delimiterPos = contents.find(':', pos);
    if (delimiterPos == std::string::npos) {
        throw std::runtime_error("String delimiter ':' not found");
    }
    int lengthg = std::stoll(contents.substr(pos, delimiterPos - pos));
    pos = delimiterPos + 1;
    BencodeString result = contents.substr(pos, lengthg);
    pos += lengthg;
    return result;
}

BencodeValue ParseList(const std::string& contents, size_t& pos) {
    std::vector<std::shared_ptr<BencodeValue>> res;
    while (contents[pos] != 'e') {
        res.push_back(std::make_shared<BencodeValue>(ParseBencode(contents, pos)));
    }
    ++pos;
    return res;
}

BencodeValue ParseDict(const std::string& contents, size_t& pos) {
    std::map<BencodeString, std::shared_ptr<BencodeValue>> res;
    BencodeString now_key;
    BencodeValue now_value;
    while (contents[pos] != 'e') {
        now_key = ParseString(contents, pos);
        now_value = ParseBencode(contents, pos);
        res.insert({now_key, std::make_shared<BencodeValue>(now_value)});
    }
    ++pos;
    return res;
}
BencodeValue ParseBencode(const std::string& contents, size_t& pos) {
    if (pos >= contents.length()) {
        throw std::runtime_error("Unexpected end of data");
    }
    char type = contents[pos];
    switch (type) {
        case 'i': {
            // Обработка целого числа
            return ParseInteger(contents, ++pos);
        }
        case 'l': {
            // Обработка списка
            return ParseList(contents, ++pos);
        }
        case 'd': {
            // Обработка словаря
            return ParseDict(contents, ++pos);
        }
        default: {
            if (isdigit(type)) {
                // Обработка строки
                return ParseString(contents, pos);
            }
            throw std::runtime_error("Invalid Bencode format");
        }
    }
}
void EncodeBencode(const BencodeValue& value, std::string& out) {

    if (std::holds_alternative<BencodeInt>(value)) {
        out += 'i';
        out += std::to_string(std::get<BencodeInt>(value));
        out += 'e';
    } else if (std::holds_alternative<BencodeString>(value)) {
        const auto& str = std::get<BencodeString>(value);
        out += std::to_string(str.size());
        out += ':';
        out += str;
    } else if (std::holds_alternative<BencodeList>(value)) {
        out += 'l';
        for (const auto& item : std::get<BencodeList>(value)) {
            EncodeBencode(*item, out);
        }
        out += 'e';
    } else if (std::holds_alternative<BencodeDict>(value)) {
        out += 'd';
        for (const auto& [key, val] : std::get<BencodeDict>(value)) {
            out += std::to_string(key.size());
            out += ':';
            out += key;
            EncodeBencode(*val, out);
        }
        out += 'e';
    }

}


std::vector<std::string> splitPiecesString(std::string& pieces) {
    const size_t SHA1_HASH_SIZE = 20;
    std::vector<std::string> hashes;
    for (size_t i = 0; i < pieces.length(); i += SHA1_HASH_SIZE) {
        hashes.push_back(pieces.substr(i, SHA1_HASH_SIZE));
    }
    return hashes;
}
std::string ComputeSHA1Hash(const std::string& data, const size_t& count) {
    char hash_helper[count];
    unsigned char hash[20];
    for (size_t i = 0; i < data.size(); ++i) {
        hash_helper[i] = data[i];
    }
    SHA1((const unsigned char*) hash_helper, count, hash);
    std::string res = "";
    for (int i = 0; i < 20; ++i) {
        res += hash[i];
    }
    return res;
}

TorrentFile ConvertParsedDataToTorrentFile(const BencodeValue& data) {
    TorrentFile file;


    const auto& dict = std::get<BencodeDict>(data);


    if (auto it = dict.find("announce"); it != dict.end() && std::holds_alternative<BencodeString>(*(it->second))) {
        file.announce = std::get<BencodeString>(*(it->second));
    } else {
        throw std::runtime_error("Announce field is missing or not a string");
    }

    if (auto it = dict.find("comment"); it != dict.end() && std::holds_alternative<BencodeString>(*(it->second))) {
        file.comment = std::get<BencodeString>(*(it->second));
    } else{
        throw std::runtime_error("NO comment");
    }

    const auto& info = std::get<BencodeDict>(*(dict.at("info")));

    if (auto it = info.find("name"); it != info.end() && std::holds_alternative<BencodeString>(*(it->second))) {
        file.name = std::get<BencodeString>(*(it->second));
    } else {
        throw std::runtime_error("Name field is missing or not a string");
    }

    if (auto it = info.find("piece length"); it != info.end() && std::holds_alternative<BencodeInt>(*(it->second))) {
        file.pieceLength = std::get<BencodeInt>(*(it->second));
    } else {
        throw std::runtime_error("Piece length field is missing or not an integer");
    }

    if (auto it = info.find("length"); it != info.end() && std::holds_alternative<BencodeInt>(*(it->second))) {
        file.length = std::get<BencodeInt>(*(it->second));
    } else {
        throw std::runtime_error("Length field is missing or not an integer");
    }

    if (auto it = info.find("pieces"); it != info.end() && std::holds_alternative<BencodeString>(*(it->second))) {
        file.pieceHashes = splitPiecesString(std::get<BencodeString>(*(it->second)));
    } else {
        throw std::runtime_error("Pieces field is missing or not a string");
    }

    std::string info_content;
    EncodeBencode(info,  info_content);
    file.infoHash = ComputeSHA1Hash(info_content, info_content.size());
//    std::cout << file.infoHash << '\n';
    return file;
}
TorrentFile LoadTorrentFile(const std::string& filename) {
    // Открытие файла
    std::ifstream fileStream(filename, std::ios::binary);
    if (!fileStream) {
        // Обработка ошибки открытия файла
        throw std::runtime_error("Cannot open file: " + filename);
    }

    // Чтение всего файла в строку
    std::string contents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    // Парсинг Bencode
    size_t pos = 0;
    auto parsedData = ParseBencode(contents, pos);
    // Обработка и преобразование распарсенных данных в вашу структуру данных
    TorrentFile torrentFile = ConvertParsedDataToTorrentFile(parsedData); // `ConvertParsedDataToTorrentFile` нужно реализовать
//    std::cout << "announce " << torrentFile.announce << '\n';
//    std::cout << "comment " << torrentFile.comment << '\n';
//    std::cout << "name " << torrentFile.name << '\n';
//    std::cout << "pieceLength " << torrentFile.pieceLength << '\n';
//    std::cout << "length " << torrentFile.length << '\n';
    return torrentFile;
}

class TorrentTracker {
public:
    /*
     * url - адрес трекера, берется из поля announce в .torrent-файле
     */
    TorrentTracker(const std::string& url): url_(url){}


    /*
     * Получить список пиров у трекера и сохранить его для дальнейшей работы.
     * Запрос пиров происходит посредством HTTP GET запроса, данные передаются в формате bencode.
     * Такой же формат использовался в .torrent файле.
     * Подсказка: посмотрите, что было написано в main.cpp в домашнем задании torrent-file
     *
     * tf: структура с разобранными данными из .torrent файла из предыдущего домашнего задания.
     * peerId: id, под которым представляется наш клиент.
     * port: порт, на котором наш клиент будет слушать входящие соединения (пока что мы не слушаем и на этот порт никто
     *  не сможет подключиться).
     */
    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port){
        cpr::Response res = cpr::Get(
                cpr::Url{url_},
                cpr::Parameters {
                        {"info_hash", tf.infoHash},
                        {"peer_id", peerId},
                        {"port", std::to_string(port)},
                        {"uploaded", std::to_string(0)},
                        {"downloaded", std::to_string(0)},
                        {"left", std::to_string(tf.length)},
                        {"compact", std::to_string(1)}
                },
                cpr::Timeout{20000}
        );
        size_t pos = 0;
        auto parsedData = ParseBencode(res.text, pos);
        const auto& dict = std::get<BencodeDict>(parsedData);
        std::string binaryData;
        if (auto it = dict.find("peers"); it != dict.end() && std::holds_alternative<BencodeString>(*(it->second))) {
            binaryData = std::get<BencodeString>(*(it->second));
        }
        for (size_t i = 0; i < binaryData.size(); i += 6) {

            std::stringstream ip;
            ip << (unsigned int)(unsigned char)binaryData[i] << '.'
               << (unsigned int)(unsigned char)binaryData[i + 1] << '.'
               << (unsigned int)(unsigned char)binaryData[i + 2] << '.'
               << (unsigned int)(unsigned char)binaryData[i + 3];


            int portt = ((unsigned char)binaryData[i + 4] << 8)
                       + (unsigned char)binaryData[i + 5];


            peers_.push_back({ip.str(), portt});
        }
    }

    /*
     * Отдает полученный ранее список пиров
     */
    const std::vector<Peer>& GetPeers() const{
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};


