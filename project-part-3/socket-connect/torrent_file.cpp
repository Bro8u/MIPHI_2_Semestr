#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>


using namespace Bencode;
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
//d8:announce41:http://bttracker.debian.org:6969/announce7:comment35:"Debian CD from cdimage.debian.org"10:created by13:mktorrent 1.113:creation datei1671279444e4:infod6:lengthi406847488e4:name31:debian-11.6.0-amd64-netinst.iso12:piece lengthi262144e6:pieces31040:��1rY�ڄ�	�Oy�^"yZa��'^�(C�V�N'R�ͮ.�2��k,��MW�	��n	�����6�~�{�����