#ifndef BLOCK_H
#define BLOCK_H
#include <algorithm>
#include <bits/stdc++.h>

namespace ns3 {

class Transaction {
public:
    std::string sender; // 事务发起方地址
    std::string recive; // 事务接收方地址
    std::string data;   // 事务数据内容
    int value;          // 价格
    int nonce;          // 发起方事务序号
    std::string txHash; // 事务哈希
    std::string sign;   // 发起方签名

    Transaction();
    Transaction(std::string sender, std::string recive, std::string data, int value, int nonce, std::string txHash, std::string sign);
};

class BlockHeader {
public:
    int blockNumber;                   // 区块号
    std::string parentHash;            // 父区块哈希
    std::string transactionsRootHash;  // 事务根哈希
    int timeStamp;                     // 出块时间戳
    std::string extraData;             // 额外数据
    int primary;                       // 区块当前主节点索引号
    std::vector<int> replica;          // 区块当前副本节点索引集合
    std::string headerHash;            // 区块头哈希
    std::vector<std::string> signList; // pbft-commit消息签名集合

    BlockHeader();
    BlockHeader(int bn, std::string ph, std::string trh, int ts, std::string ed, int p, std::vector<int> r, std::string hh, std::vector<std::string> sl);
};

class BlockBody {
public:
    std::vector<Transaction> Transactions;
};

class Block {
public:
    BlockHeader header;
    BlockBody body;

    Block();
    virtual ~Block();
    void showBlock();          // 显示区块内容
    Transaction getRandomTx(); // 随机产生事务
};
} // namespace ns3

class Transaction {
public:
    std::string sender; // 事务发起方地址
    std::string recive; // 事务接收方地址
    std::string data;   // 事务数据内容
    int value;          // 价格
    int nonce;          // 发起方事务序号
    std::string txHash; // 事务哈希
    std::string sign;   // 发起方签名

    Transaction();
    Transaction(std::string sender, std::string recive, std::string data, int value, int nonce, std::string txHash, std::string sign);
};

class BlockHeader {
public:
    int blockNumber;                   // 区块号
    std::string parentHash;            // 父区块哈希
    std::string transactionsRootHash;  // 事务根哈希
    int timeStamp;                     // 出块时间戳
    std::string extraData;             // 额外数据
    int primary;                       // 区块当前主节点索引号
    std::vector<int> replica;          // 区块当前副本节点索引集合
    std::string headerHash;            // 区块头哈希
    std::vector<std::string> signList; // pbft-commit消息签名集合

    BlockHeader();
    BlockHeader(int bn, std::string ph, std::string trh, int ts, std::string ed, int p, std::vector<int> r, std::string hh, std::vector<std::string> sl);
};

class BlockBody {
public:
    std::vector<Transaction> Transactions;
};

class Block {
public:
    BlockHeader header;
    BlockBody body;

    Block();
    virtual ~Block();
    void showBlock();          // 显示区块内容
    Transaction getRandomTx(); // 随机产生事务
};
#endif // BLOCK_H
