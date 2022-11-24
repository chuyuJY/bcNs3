#ifndef PBFT_NODE_H
#define PBFT_NODE_H

#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include "ns3/block.h"
#include "ns3/message.h"
#include <map>

namespace ns3 {

class Address;
class Socket;
class Packet;

class BcNode {
public:
    int nodeId;                                      // 节点标识
    Ptr<Socket> socket;                              // 监听的socket
    std::vector<Ipv4Address> peersAddresses;         // 邻节点列表
    std::map<Ipv4Address, Ptr<Socket>> peersSockets; // 邻节点的socket列表
    RSA *rsaPubKey;                                  // 公钥
    RSA *rsaPriKey;                                  // 私钥

    BcNode();
    BcNode(int nodeId, std::vector<Ipv4Address> peersAddresses);
    void SetPeersAddresses(const std::vector<Ipv4Address> &peers); // 设置所有邻节点的地址
};

class Pbft : public Application {
public:
    BcNode node;                                              // 节点信息
    std::vector<int> values;                                  // 存储要更新的数值的容器
    std::map<std::string, Block> tmpBlockPool;                // 临时区块池
    std::vector<Block> localBlockPool;                        // 持久化区块池 TODO: 模拟持久化层, 通过 Reply 阶段后写入
    int view;                                                 // 视图编号
    int nonce;                                                // 交易编号
    int leader;                                               // 当前leader节点的编号 默认为容器第一个节点
    int blockNumber;                                          // 当前区块号
    int nodeNums;                                             // Pbft网络总节点数目
    std::map<std::string, std::vector<int>> prepareVoteCount; // 存储prepare投票信息(至少需要收到并确认2f个), 根据摘要索引
    std::map<std::string, std::vector<int>> commitVoteCount;  // 存储commit投票信息(至少需要收到并确认2f+1个), 根据摘要索引
    std::map<std::string, bool> isCommit;                     // 该区块是否已进行过Commit广播(避免重复广播)
    std::map<std::string, bool> isReply;                      // 该区块是否已完成过(避免重复广播)

    Pbft(void);
    virtual ~Pbft(void);
    static TypeId GetTypeId(void);
    virtual void StartApplication(void);                            // 继承 Application 类必须实现的虚函数
    virtual void StopApplication(void);                             // 继承 Application 类必须实现的虚函数
    void HandleRead(Ptr<Socket> socket);                            // 处理消息, 根据不同消息调用处理函数
    void HandleRequest(void);                                       // 处理 Request
    void HandlePrePrepare(std::string message);                     // 处理 PrePrePare
    void HandlePrepare(std::string message);                        // 处理 Prepare
    void HandleCommit(std::string message);                         // 处理 Commit
    std::string getPacketContent(Ptr<Packet> packet, Address from); // 将数据包中的消息解析为字符串
    void Broadcast(std::string msgType, std::string message);       // 向所有邻节点广播消息
    void Send(uint8_t data[], Address from);                        // 向某个指定地址的节点发送消息
    void SendBlock(void);                                           // 向所有邻节点广播区块
    void viewChange(void);                                          // 视图切换
};
} // namespace ns3
#endif