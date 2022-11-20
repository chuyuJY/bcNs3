#include <iostream>
#include <cstdlib>
#include <ctime>
#include "block.h"

std::string addr[] = {"0x01", "0x02", "0x03", "0x04", "0x05"};

Block::Block() {
    // 随机产生 10 个事务，并添加进区块体
    for (int i = 0; i < 10; ++i) {
        Transaction tmpTx = getRandomTx();
        body.Transactions.emplace_back(tmpTx);
    }
    header.timeStamp = getTimestamp();
}

Block::~Block() {
    // dtor
}

void Block::showBlock() {
    std::cout << "BlockHeader" << std::endl;
    std::cout << " blockNumber: " << header.blockNumber << std::endl;
    std::cout << " parentHash: " << header.parentHash << std::endl;
    std::cout << " transactionsRootHash: " << header.transactionsRootHash << std::endl;
    std::cout << " timeStamp: " << header.timeStamp << std::endl;
    std::cout << " extraData: " << header.extraData << std::endl;
    std::cout << std::endl;

    std::cout << "BlockBody" << std::endl;
    for (auto &tx : body.Transactions) {
        std::cout << " " << tx.sender << " -> " << tx.recive;
        std::cout << " : " << tx.value << std::endl;
    }
}

BlockHeader::BlockHeader() {
    this->blockNumber = 0;
    this->parentHash = "0x0000000";
    this->transactionsRootHash = "0x0000000";
    this->timeStamp = 000000;
    this->extraData = "";
    this->primary = 0;
    this->headerHash = "0x0000000";
}
BlockHeader::BlockHeader(int bn, std::string ph, std::string trh, int ts, std::string ed, int p, std::vector<int> r, std::string hh, std::vector<std::string> sl) {
    this->blockNumber = bn;
    this->parentHash = std::move(ph);
    this->transactionsRootHash = std::move(trh);
    this->timeStamp = ts;
    this->extraData = std::move(ed);
    this->primary = p;
    this->replica = std::move(r);
    this->headerHash = std::move(hh);
    this->signList = std::move(sl);
}

Transaction::Transaction() {
    this->sender = "0x000001";
    this->recive = "0x000002";
    this->data = "transaction";
    this->value = 0;
    this->nonce = 1;
    this->txHash = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
    this->sign = "10a34637ad661d98ba3344717656fcc76209c2f8";
}

Transaction::Transaction(std::string sender, std::string recive, std::string data, int value, int nonce, std::string txHash, std::string sign) {
    this->sender = sender;
    this->recive = recive;
    this->data = data;
    this->value = 0;
    this->nonce = nonce;
    this->txHash = txHash;
    this->sign = sign;
}

Transaction Block::getRandomTx() {
    Transaction tx;
    int senderIdx = rand() % 5;
    int reciveIdx = rand() % 5;
    while (senderIdx == reciveIdx) {
        reciveIdx = rand() % 5;
    }
    tx.sender = addr[senderIdx];
    tx.recive = addr[reciveIdx];
    tx.data = "empty";
    tx.value = rand();
    return tx;
}

// 产生 160位的随机地址
std::string getRandomAddr() {
    std::string addr = "0x";
    for (int i = 0; i < 10; ++i) {
        addr += char(rand() % 10);
    }
    return addr;
}

// 通过chrono库获取当前系统时间戳
int getTimestamp() {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpMicro = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    time_t timeStamp = tpMicro.time_since_epoch().count();
    return timeStamp;
}
