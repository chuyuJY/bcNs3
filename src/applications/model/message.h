#ifndef MESSAGE_H
#define MESSAGE_H
#include <algorithm>
#include <bits/stdc++.h>
#include "block.h"

namespace ns3 {

const int prefixLength        = 12;
const std::string Delimiter   = "@";
const std::string FRequest    = "request";
const std::string FPrePrepare = "preprepare";
const std::string FPrepare    = "prepare";
const std::string FCommit     = "commit";

enum Key {
    REQUEST,         // 0       客户端请求
    PRE_PREPARE,     // 1       预准备消息
    PREPARE,         // 2       准备消息
    COMMIT,          // 3       提交
    PRE_PREPARE_RES, // 4       预准备消息的响应
    PREPARE_RES,     // 5       准备消息响应
    COMMIT_RES,      // 6       提交响应
    REPLY,           // 7       对客户端的回复
    VIEW_CHANGE      // 8       view_change消息
};

class Message {
public:
    Message();
    std::map<std::string, int> KeyFromString;
};

// <REQUEST,o,t,c>
class Request {
public:
    Transaction transaction;
    time_t TimeStamp;
    int ClientId;
};

// <<PRE-PREPARE,v,n,d>,m>
class PrePrepare {
public:
    // Request RequestMsg;
    Block block;           // 区块
    int ViewId;            // 试图编号
    int Nonce;             // 编号
    std::string Digest;    // 摘要, 就是区块头的hash
    std::string Signature; // 签名信息

    PrePrepare();
    PrePrepare(Block block, int viewId, int nonce, std::string digest, std::string signature);
};

void to_json(json &j, const PrePrepare &pp);
void from_json(const json &j, PrePrepare &p);

// <PREPARE,v,n,d,i>
class Prepare {
public:
    int ViewId;            // 试图编号
    int Nonce;             // 编号
    int NodeId;            // 节点标识
    std::string Digest;    // 摘要: 同PrePrepare
    std::string Signature; // 签名信息

    Prepare();
    Prepare(int viewId, int nonce, int nodeId, std::string digest, std::string signature);
};
void to_json(json &j, const Prepare &p);
void from_json(const json &j, Prepare &p);

// <COMMIT,v,n,d,i>
class Commit {
public:
    int ViewId;            // 试图编号
    int Nonce;             // 编号
    int NodeId;            // 节点标识
    std::string Digest;    // 摘要: 同PrePrepare
    std::string Signature; // 签名信息

    Commit();
    Commit(int viewId, int nonce, int nodeId, std::string digest, std::string signature);
};

void to_json(json &j, const Commit &c);
void from_json(const json &j, Commit &c);

class Reply {
public:
    int ViewId;            // 试图编号
    int Nonce;             // 编号
    int NodeId;            // 节点标识
    std::string Digest;    // 摘要: 同PrePrepare
    std::string Signature; // 签名信息

    Reply();
    Reply(int viewId, int nonce, int nodeId, std::string digest, std::string signature);
};

void to_json(json &j, const Reply &r);
void from_json(const json &j, Reply &r);

std::string jointToMessage(std::string prefix, std::string message);
void splitFromMessage(std::string *message, std::string *prefix);
} // namespace ns3

#endif // MESSAGE_H