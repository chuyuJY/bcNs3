#ifndef BLOCK_H
#define BLOCK_H
#include <algorithm>
#include <bits/stdc++.h>
#include "crypto.h"
#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map.hpp"

// 默认json, 使用 std::map, 按照字符串顺序, 不太行
// using json = nlohmann::json;

// 设定json序列化保持插入顺序, 很行
template <class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using unordered_json         = nlohmann::basic_json<my_workaround_fifo_map>;
using json                   = unordered_json;

namespace ns3 {

class Transaction {
public:
    std::string sender;    // 事务发起方地址
    std::string recive;    // 事务接收方地址
    std::string data;      // 事务数据内容
    int value;             // 价格
    int nonce;             // 发起方事务序号
    std::string txHash;    // 事务哈希
    std::string signature; // 发起方签名

    Transaction();
    Transaction(std::string sender, std::string recive, std::string data, int value, int nonce);

    std::string hashTransaction();
    // TODO: sign by senders
};

void to_json(json &j, const Transaction &tx);
void from_json(const json &j, Transaction &tx);

class BlockBody {
public:
    std::vector<Transaction> Transactions;

    BlockBody();                             // 随机产生 10 个事务，并添加进区块体
    BlockBody(std::vector<Transaction> txs); // 自定 Tx
    Transaction getRandomTx();               // 随机产生事务
    std::string hashTxRoot();                // 得到事务列表的根哈希
};

void to_json(json &j, const BlockBody &bb);
void from_json(const json &j, BlockBody &bb);

class BlockHeader {
public:
    int blockNumber;                   // 区块号
    int leader;                        // 区块当前主节点索引号
    std::string parentHash;            // 父区块哈希
    std::string headerHash;            // 区块头哈希
    std::string transactionsRootHash;  // 事务根哈希
    time_t timeStamp;                  // 出块时间戳
    std::string extraData;             // 额外数据
    std::vector<int> replica;          // 区块当前副本节点索引集合
    std::vector<std::string> signList; // pbft-commit消息签名集合

    BlockHeader();
    BlockHeader(int bn, int ld, std::string ph, std::string trh, std::string ed, std::vector<int> r, std::vector<std::string> sl);
    std::string hashBlockHeader(); // 获取区块头哈希值
};

void to_json(json &j, const BlockHeader &bh);
void from_json(const json &j, BlockHeader &bh);

class Block {
public:
    BlockHeader header;
    BlockBody body;

    Block();
    Block(BlockHeader bh, BlockBody bb);
    virtual ~Block();
    void showBlock(); // 显示区块内容
};

void to_json(json &j, const Block &b);
void from_json(const json &j, Block &b);

int getTimestamp(); // 获取时间戳

} // namespace ns3
#endif // BLOCK_H
