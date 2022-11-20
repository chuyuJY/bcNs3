#include "block.h"

namespace ns3 {

std::string addr[] = {"0x01", "0x02", "0x03", "0x04", "0x05"};

Block::Block() {
}

Block::Block(BlockHeader bh, BlockBody bb) {
    this->header = std::move(bh);
    this->body   = std::move(bb);
}

Block::~Block() {
    // dtor
}

void to_json(json &j, const Block &b) {
    j = json{
        {"header", b.header},
        {"body", b.body},
    };
}

void from_json(const json &j, Block &b) {
    json jHeader = j["header"];
    json jBody   = j["body"];
    auto header  = jHeader.get<ns3::BlockHeader>();
    auto body    = jBody.get<ns3::BlockBody>();
    b.header     = header;
    b.body       = body;
}

void Block::showBlock() {
    std::cout << "---------------------BlockHeader---------------------" << std::endl;
    std::cout << std::left << std::setw(25) << " blockNumber: " << header.blockNumber << std::endl;
    std::cout << std::left << std::setw(25) << " parentHash: " << header.parentHash << std::endl;
    std::cout << std::left << std::setw(25) << " headerHash: " << header.headerHash << std::endl;
    std::cout << std::left << std::setw(25) << " transactionsRootHash: " << header.transactionsRootHash << std::endl;
    std::cout << std::left << std::setw(25) << " timeStamp: " << header.timeStamp << std::endl;
    std::cout << std::left << std::setw(25) << " extraData: " << header.extraData << std::endl;

    std::cout << "----------------------BlockBody----------------------" << std::endl;
    for (auto &tx : body.Transactions) {
        std::cout << " 发起方地址: " << tx.sender << " --> 接收方地址: " << tx.recive;
        std::cout << " : " << tx.value << std::endl;
    }
    // std::cout << std::endl;
}

BlockHeader::BlockHeader() {
    this->blockNumber          = 0;
    this->leader               = 0;
    this->parentHash           = "00000000000000000000000000000000";
    this->headerHash           = "00000000000000000000000000000000";
    this->transactionsRootHash = "00000000000000000000000000000000";
    this->timeStamp            = 000000;
    this->extraData            = "zjy_test";
}

BlockHeader::BlockHeader(int bn, int ld, std::string ph, std::string trh, std::string ed, std::vector<int> r, std::vector<std::string> sl) {
    this->blockNumber          = bn;
    this->leader               = ld;
    this->parentHash           = std::move(ph);
    this->transactionsRootHash = std::move(trh);
    this->extraData            = std::move(ed);
    this->timeStamp            = getTimestamp();
    this->replica              = std::move(r);
    this->signList             = std::move(sl);
    this->headerHash           = this->hashBlockHeader();
}

void to_json(json &j, const BlockHeader &bh) {
    j = json{
        {"blockNumber", bh.blockNumber},
        {"leader", bh.leader},
        {"parentHash", bh.parentHash},
        {"headerHash", bh.headerHash},
        {"transactionsRootHash", bh.transactionsRootHash},
        {"timeStamp", bh.timeStamp},
        {"extraData", bh.extraData},
        {"replica", bh.replica},
        {"signList", bh.signList},
    };
}

void from_json(const json &j, BlockHeader &bh) {
    j.at("blockNumber").get_to(bh.blockNumber);
    j.at("leader").get_to(bh.leader);
    j.at("parentHash").get_to(bh.parentHash);
    j.at("headerHash").get_to(bh.headerHash);
    j.at("transactionsRootHash").get_to(bh.transactionsRootHash);
    j.at("timeStamp").get_to(bh.timeStamp);
    j.at("extraData").get_to(bh.extraData);
    j.at("replica").get_to(bh.replica);
    j.at("signList").get_to(bh.signList);
}

Transaction::Transaction() {
    this->sender    = "0x000001";
    this->recive    = "0x000002";
    this->data      = "transaction";
    this->value     = 0;
    this->nonce     = 1;
    this->txHash    = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
    this->signature = "10a34637ad661d98ba3344717656fcc76209c2f8";
}

Transaction::Transaction(std::string sender, std::string recive, std::string data, int value, int nonce) {
    this->sender = std::move(sender);
    this->recive = std::move(recive);
    this->data   = std::move(data);
    this->value  = std::move(value);
    this->nonce  = std::move(nonce);
    this->txHash = this->hashTransaction();
    // TODO: sign by senders
}

void to_json(json &j, const Transaction &tx) {
    j = json{
        {"sender", tx.sender},
        {"recive", tx.recive},
        {"data", tx.data},
        {"value", tx.value},
        {"nonce", tx.nonce},
        {"txHash", tx.txHash},
        {"signature", tx.signature},
    };
}

void from_json(const json &j, Transaction &tx) {
    j.at("sender").get_to(tx.sender);
    j.at("recive").get_to(tx.recive);
    j.at("data").get_to(tx.data);
    j.at("value").get_to(tx.value);
    j.at("nonce").get_to(tx.nonce);
    j.at("txHash").get_to(tx.txHash);
    j.at("signature").get_to(tx.signature);
}

BlockBody::BlockBody() {
    for (int i = 0; i < 10; i++) {
        Transaction tmpTx = getRandomTx();
        this->Transactions.emplace_back(tmpTx);
    }
}

BlockBody::BlockBody(std::vector<Transaction> txs) {
    this->Transactions = std::move(txs);
}

Transaction BlockBody::getRandomTx() {
    Transaction tx;
    int senderIdx = rand() % 5;
    int reciveIdx = rand() % 5;
    while (senderIdx == reciveIdx) {
        reciveIdx = rand() % 5;
    }
    tx.sender = addr[senderIdx];
    tx.recive = addr[reciveIdx];
    tx.data   = "empty";
    tx.value  = rand();
    tx.txHash = tx.hashTransaction();
    // TODO: sign by users
    return tx;
}

void to_json(json &j, const BlockBody &bb) {
    for (auto &tx : bb.Transactions) {
        json jTx = tx;
        // to_json(jTx, tx);
        j["Transactions"].emplace_back(jTx);
    }
}

void from_json(const json &j, BlockBody &bb) {
    for (auto &jTx : j["Transactions"]) {
        auto tx = jTx.get<ns3::Transaction>();
        // from_json(jTx, tx);
        bb.Transactions.emplace_back(tx);
    }
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
    time_t timeStamp                                                                      = tpMicro.time_since_epoch().count();
    return timeStamp;
}

std::string BlockHeader::hashBlockHeader() {
    json j = json{
        {"blockNumber", this->blockNumber},
        {"leader", this->leader},
        {"parentHash", this->parentHash},
        {"transactionsRootHash", this->transactionsRootHash},
        {"timeStamp", this->timeStamp},
        {"extraData", this->extraData},
        {"replica", this->replica},
        {"signList", this->signList},
    };
    std::string strJ       = j.dump();
    std::string hashHeader = sha256(strJ);
    // std::cout << "hashHeader: " << hashHeader << std::endl;
    return hashHeader;
}

std::string Transaction::hashTransaction() {
    json j = json{
        {"sender", this->sender},
        {"recive", this->recive},
        {"data", this->data},
        {"value", this->value},
        {"nonce", this->nonce},
    };
    std::string strJ   = j.dump();
    std::string hashTx = sha256(strJ);
    // std::cout << "hashHeader: " << hashHeader << std::endl;
    return hashTx;
}

std::string BlockBody::hashTxRoot() {
    // 级联所有交易的哈希值, 再计算哈希, 暂时先不做 Merkle tree
    std::string txsHash;
    for (auto &tx : this->Transactions) {
        txsHash.append(tx.txHash);
    }
    std::string hashTxsHash = sha256(txsHash);
    return hashTxsHash;
}

} // namespace ns3