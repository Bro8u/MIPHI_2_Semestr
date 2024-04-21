#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <openssl/sha.h>


namespace Bencode {

/*
 * В это пространство имен рекомендуется вынести функции для работы с данными в формате bencode.
 * Этот формат используется в .torrent файлах и в протоколе общения с трекером
 */
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

    void EncodeBencode(const BencodeValue& value, std::string& out);
    std::string ComputeSHA1Hash(const std::string& data, const size_t& count);
    std::vector<std::string> splitPiecesString(std::string& pieces);

}
