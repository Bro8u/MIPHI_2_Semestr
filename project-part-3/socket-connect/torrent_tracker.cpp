#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>

using namespace Bencode;

TorrentTracker::TorrentTracker(const std::string& url): url_(url){}


void TorrentTracker::UpdatePeers(const TorrentFile& tf, std::string peerId, int port){
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


const std::vector<Peer>& TorrentTracker::GetPeers() const{
    return peers_;
}



