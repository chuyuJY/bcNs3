#include "message.h"

namespace ns3 {

Message::Message() {
    // 绑定消息类型到 Key
    KeyFromString[FRequest]    = REQUEST;
    KeyFromString[FPrePrepare] = PRE_PREPARE;
    KeyFromString[FPrepare]    = PREPARE;
    KeyFromString[FCommit]     = COMMIT;
}

PrePrepare::PrePrepare() {
    this->block     = Block{};
    this->ViewId    = 0;
    this->Nonce     = 0;
    this->Digest    = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
    this->Signature = "10a34637ad661d98ba3344717656fcc76209c2f8";
}

PrePrepare::PrePrepare(Block block, int viewId, int nonce, std::string digest, std::string signature) {
    this->block     = block;
    this->ViewId    = viewId;
    this->Nonce     = nonce;
    this->Digest    = digest;
    this->Signature = signature;
}

void to_json(json &j, const PrePrepare &pp) {
    j = json{
        {"block", pp.block},
        {"ViewId", pp.ViewId},
        {"Nonce", pp.Nonce},
        {"Digest", pp.Digest},
        {"Signature", pp.Signature},
    };
}

void from_json(const json &j, PrePrepare &pp) {
    j.at("block").get_to(pp.block);
    j.at("ViewId").get_to(pp.ViewId);
    j.at("Nonce").get_to(pp.Nonce);
    j.at("Digest").get_to(pp.Digest);
    j.at("Signature").get_to(pp.Signature);
}

Prepare::Prepare() {
    this->ViewId    = 0;
    this->Nonce     = 0;
    this->NodeId    = 0;
    this->Digest    = "";
    this->Signature = "";
}

Prepare::Prepare(int viewId, int nonce, int nodeId, std::string digest, std::string signature) {
    this->ViewId    = viewId;
    this->Nonce     = nonce;
    this->NodeId    = nodeId;
    this->Digest    = digest;
    this->Signature = signature;
}

void to_json(json &j, const Prepare &p) {
    j = json{
        {"ViewId", p.ViewId},
        {"Nonce", p.Nonce},
        {"NodeId", p.NodeId},
        {"Digest", p.Digest},
        {"Signature", p.Signature},
    };
}

void from_json(const json &j, Prepare &p) {
    j.at("ViewId").get_to(p.ViewId);
    j.at("Nonce").get_to(p.Nonce);
    j.at("NodeId").get_to(p.NodeId);
    j.at("Digest").get_to(p.Digest);
    j.at("Signature").get_to(p.Signature);
}

Commit::Commit() {
    this->ViewId    = 0;
    this->Nonce     = 0;
    this->NodeId    = 0;
    this->Digest    = "";
    this->Signature = "";
}

Commit::Commit(int viewId, int nonce, int nodeId, std::string digest, std::string signature) {
    this->ViewId    = viewId;
    this->Nonce     = nonce;
    this->NodeId    = nodeId;
    this->Digest    = digest;
    this->Signature = signature;
}

void to_json(json &j, const Commit &c) {
    j = json{
        {"ViewId", c.ViewId},
        {"Nonce", c.Nonce},
        {"NodeId", c.NodeId},
        {"Digest", c.Digest},
        {"Signature", c.Signature},
    };
}
void from_json(const json &j, Commit &c) {
    j.at("ViewId").get_to(c.ViewId);
    j.at("Nonce").get_to(c.Nonce);
    j.at("NodeId").get_to(c.NodeId);
    j.at("Digest").get_to(c.Digest);
    j.at("Signature").get_to(c.Signature);
}

Reply::Reply() {
    this->ViewId    = 0;
    this->Nonce     = 0;
    this->NodeId    = 0;
    this->Digest    = "";
    this->Signature = "";
}

Reply::Reply(int viewId, int nonce, int nodeId, std::string digest, std::string signature) {
    this->ViewId    = viewId;
    this->Nonce     = nonce;
    this->NodeId    = nodeId;
    this->Digest    = digest;
    this->Signature = signature;
}

void to_json(json &j, const Reply &r) {
    j = json{
        {"ViewId", r.ViewId},
        {"Nonce", r.Nonce},
        {"NodeId", r.NodeId},
        {"Digest", r.Digest},
        {"Signature", r.Signature},
    };
}
void from_json(const json &j, Reply &r) {
    j.at("ViewId").get_to(r.ViewId);
    j.at("Nonce").get_to(r.Nonce);
    j.at("NodeId").get_to(r.NodeId);
    j.at("Digest").get_to(r.Digest);
    j.at("Signature").get_to(r.Signature);
}

// 用 '@' 作为分隔符
std::string jointToMessage(std::string prefix, std::string message) {
    // std::vector<char> bytes(prefixLength);
    // for (size_t i = 0; i < prefix.size(); i++) {
    //     bytes[i] = prefix[i];
    // }
    // std::string str(bytes.begin(), bytes.end());
    return prefix + Delimiter + message;
}

// 分割前12位标识与消息内容
void splitFromMessage(std::string *message, std::string *prefix) {
    int index = (*message).find_first_of(Delimiter);
    *prefix   = (*message).substr(0, index);
    *message  = (*message).substr(index + 1, (*message).length());
}

// 切割多次
// std::vector<std::string> splitString(const std::string &str, const std::string &delim) {
//     std::regex re{delim};
//     return std::vector<std::string>{
//         std::sregex_token_iterator(str.begin(), str.end(), re, -1),
//         std::sregex_token_iterator()};
// }

} // namespace ns3
