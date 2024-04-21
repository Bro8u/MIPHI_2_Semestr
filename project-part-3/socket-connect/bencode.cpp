#include "bencode.h"

namespace Bencode {


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

}
