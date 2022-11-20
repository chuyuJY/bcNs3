#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "stdlib.h"
#include "ns3/ipv4.h"
#include <ctime>
#include <map>
#include "pbft-node.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("Pbft");
NS_OBJECT_ENSURE_REGISTERED(Pbft);

TypeId
Pbft::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::Pbft")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<Pbft>();
    return tid;
}

// 信息接收延迟 3 - 6 ms
float getRandomDelay() {
    return ((rand() % 3) * 1.0 + 3) / 1000;
}

BcNode::BcNode() {
}

// 构造BcNode
BcNode::BcNode(int nodeId, std::vector<Ipv4Address> peersAddresses) {
    this->nodeId         = nodeId;
    this->peersAddresses = peersAddresses;
    // 分配密钥
    this->rsaPubKey = RSA_new();
    this->rsaPriKey = RSA_new();
    getRSAKeyPair(this->nodeId, this->rsaPubKey, this->rsaPriKey);
}

// 构造PbftNode
Pbft::Pbft(void) {
    // 初始化PbftNode对象
    this->view        = 0; // 视图编号
    this->nonce       = 0; // 当前视图的交易序号
    this->leader      = 0; // 当前leader
    this->blockNumber = 0; // 区块高度
}

Pbft::~Pbft(void) {
    NS_LOG_FUNCTION(this);
}

void Pbft::StartApplication() {
    // 初始化 node.socket
    if (!this->node.socket) {
        TypeId tid              = TypeId::LookupByName("ns3::UdpSocketFactory");
        this->node.socket       = Socket::CreateSocket(Application::GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 7071); //监听所有网卡ip
        this->node.socket->Bind(local);                                           // 绑定本机的ip和port
        this->node.socket->Listen();
    }
    // 在新数据可用时调用应用程序
    // 此回调旨在通知在阻塞套接字模型中已阻止数据可用于读取的套接字
    node.socket->SetRecvCallback(MakeCallback(&Pbft::HandleRead, this));
    node.socket->SetAllowBroadcast(true);
    // 与所有节点建立连接
    std::vector<Ipv4Address>::iterator iter = node.peersAddresses.begin(); // iter是个指针，遍历当前节点的邻接表
    NS_LOG_INFO("node" << node.nodeId << "正在与其他节点建立socket连接...");
    while (iter != node.peersAddresses.end()) {
        NS_LOG_INFO("node" << node.nodeId << " 与 " << *iter << " 已建立socket连接");
        TypeId tid               = TypeId::LookupByName("ns3::UdpSocketFactory");
        Ptr<Socket> socketClient = Socket::CreateSocket(GetNode(), tid);
        socketClient->Connect(InetSocketAddress(*iter, 7071));
        node.peersSockets[*iter] = socketClient;
        iter++;
    }
    // Simulator::Schedule(Seconds(timeout), &Pbft::SendBlock, this);
    if (leader == this->node.nodeId) {
        Simulator::ScheduleNow(&Pbft::HandleRequest, this);
        // Simulator::Schedule(Seconds(timeout), &Pbft::HandleRequest, this);
    }
}

void Pbft::StopApplication() {
    // printVector(values);
}

void Pbft::HandleRead(Ptr<Socket> socket) {
    // FIXME: 考虑上锁, 需要查 ns3消息处理顺序是咋做的
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) { // EOF
            break;
        }
        if (InetSocketAddress::IsMatchingType(from)) {
            std::string message = getPacketContent(packet, from);
            // NS_LOG_INFO("handleRead message: " << message);
            std::string prefix;
            splitFromMessage(&message, &prefix);
            // NS_LOG_INFO("prefix: " << prefix << " message" << message);
            // NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s, Node " << GetNode ()->GetId () << " received " << packet->GetSize () << " bytes, msg[0]: "<< msg[0]);
            //     InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            //     InetSocketAddress::ConvertFrom (from).GetPort ());
            Message *msgKey = new Message();
            int key         = (*msgKey).KeyFromString[prefix];
            delete msgKey;
            switch (key) {
            case REQUEST: {
                if (this->node.nodeId == this->leader) {
                    HandleRequest();
                }
                break;
            }
            case PRE_PREPARE: {
                HandlePrePrepare(message);
                break;
            }
            case PREPARE: {
                HandlePrepare(message);
                break;
            }
            case COMMIT: {
                HandleCommit(message);
                break;
            }
            default: {
                NS_LOG_INFO("Wrong message");
                break;
            }
            }
        }
        // socket->GetSockName(localAddress);
    }
}

void Pbft::HandleRequest(void) {
    // RSA *pri = RSA_new();
    // getPriKey(0, pri);
    std::string str = "this is test";
    int length      = RSA_size(this->node.rsaPriKey);
    std::cout << "私钥长度为: " << length << std::endl;
    // getPriKey(this->node.nodeId, this->node.rsaPriKey);
    // std::string privateFile = KeysPath + "/node" + std::to_string(0) + "/private.pem";
    // BIO *pri                = BIO_new_file(privateFile.c_str(), "r");
    // key                     = PEM_read_bio_RSAPrivateKey(pri, &key, NULL, NULL);
    // BIO_free(pri);
    std::string s = sign(str, this->node.rsaPriKey);
    std::cout << "签名为: " << s << std::endl;

    NS_LOG_INFO("Leader: node" << GetNode()->GetId() << " 收到Request 当前时间: " << Simulator::Now().GetSeconds() << "s");
    // Note: 目前由主节点直接随机生成交易, Request只作为触发
    /* TODO:
        1. 增加消息池;
        2. 增加 hash及签名机制;
    */
    //  生成区块
    Block block;
    block.header.blockNumber = this->blockNumber;
    block.header.leader      = this->leader;
    // 如果是首个区块，父哈希默认为 00000000000000000000000000000000
    if (this->localBlockPool.size() > 0) {
        block.header.parentHash = this->localBlockPool.back().header.headerHash;
    }
    block.header.timeStamp            = getTimestamp();
    block.header.transactionsRootHash = block.body.hashTxRoot();
    block.header.headerHash           = block.header.hashBlockHeader();
    NS_LOG_INFO("生成区块: ");
    block.showBlock();
    NS_LOG_INFO("Leader: node" << GetNode()->GetId() << " 已将区块存入临时区块池... 当前时间: " << Simulator::Now().GetSeconds() << "s");
    this->tmpBlockPool[block.header.headerHash] = block;
    std::string digest                          = block.header.headerHash;
    std::string test                            = "this is a test...";
    std::string signature                       = sign(test, this->node.rsaPriKey);
    if (signature.size() == 0) {
        NS_LOG_INFO("node" << GetNode()->GetId() << " 签名失败... ");
    }
    this->nonce++; // 考虑上锁
    // 生成PrePrepare
    PrePrepare prePrepare = {
        block,
        this->view,
        this->nonce,
        digest,
        signature,
    };
    json jPrePrepare = prePrepare;
    NS_LOG_INFO("Leader: node" << GetNode()->GetId() << " 生成PrePrepare消息: " << jPrePrepare);
    // 广播
    std::string strPrePrepare = jPrePrepare.dump();
    this->Broadcast(FPrePrepare, strPrePrepare);
    NS_LOG_INFO("Leader node" << GetNode()->GetId() << " 已完成PrePrepare广播... 当前时间: " << Simulator::Now().GetSeconds() << "s");
}

// 处理 PrePrePare
void Pbft::HandlePrePrepare(std::string message) {
    NS_LOG_INFO("node" << GetNode()->GetId() << " 收到PrePrepare 当前时间: " << Simulator::Now().GetSeconds() << "s");
    // 解析 message为 PrePrepare对象
    auto jMessage   = json::parse(message);
    auto prePrepare = jMessage.get<PrePrepare>();
    // TEST:
    // std::cout << prePrepare.block.header.blockNumber << std::endl;
    RSA *leaderPubKey = RSA_new();
    getPubKey(this->leader, leaderPubKey);
    if (this->view != prePrepare.ViewId || this->nonce + 1 != prePrepare.Nonce) {
        NS_LOG_INFO("PrePrepare: 消息序号或视图序号验证失败...");
        return;
    } else if (prePrepare.Digest != prePrepare.block.header.hashBlockHeader()) {
        NS_LOG_INFO("PrePrepare: 摘要验证失败...");
        return;
    } else if (!verifySign(prePrepare.Digest, leaderPubKey, prePrepare.Signature)) {
        NS_LOG_INFO("PrePrepare: 签名验证失败...");
        return;
    }
    NS_LOG_INFO("区块" << prePrepare.block.header.blockNumber << "已经过验证 node" << GetNode()->GetId()
                       << " 将区块存入临时区块池... 当前时间: " << Simulator::Now().GetSeconds() << "s");
    this->tmpBlockPool[prePrepare.block.header.headerHash] = prePrepare.block;
    this->nonce                                            = prePrepare.Nonce;
    // 构造prepare消息
    std::string signature = sign(prePrepare.Digest, this->node.rsaPriKey);
    Prepare prepare       = {
              prePrepare.ViewId,
              prePrepare.Nonce,
              this->node.nodeId,
              prePrepare.Digest,
              signature,
    };
    // 转化为 json
    json jPrepare = prepare;
    NS_LOG_INFO("node" << GetNode()->GetId() << " 生成Prepare消息: " << jPrepare);
    // 广播
    std::string strPrepare = jPrepare.dump();
    this->Broadcast(FPrepare, strPrepare);
    NS_LOG_INFO("node" << GetNode()->GetId() << " 已完成Prepare广播... 当前时间: " << Simulator::Now().GetSeconds() << "s");
}

// 处理 Prepare
void Pbft::HandlePrepare(std::string message) {
    // FIXME: 解决leader节点多接收到一个prepare的问题  note:已解决, 暂未测试新版本
    NS_LOG_INFO("node" << GetNode()->GetId() << " 收到Prepare 当前时间: " << Simulator::Now().GetSeconds() << "s");
    // 解析为 Prepare对象
    auto jMessage   = json::parse(message);
    Prepare prepare = jMessage.get<Prepare>();
    // TEST:
    // std::cout << prepare.NodeId << std::endl;
    RSA *nodePubKey = RSA_new();
    getPubKey(prepare.NodeId, nodePubKey);
    if (this->view != prepare.ViewId || this->nonce != prepare.Nonce) {
        NS_LOG_INFO("Prepare: 消息序号或视图序号验证失败...");
        return;
    } else if (this->tmpBlockPool.count(prepare.Digest) == 0) { // 临时区块池无此区块
        NS_LOG_INFO("Prepare: 摘要验证失败...");
        return;
    } else if (!verifySign(prepare.Digest, nodePubKey, prepare.Signature)) {
        NS_LOG_INFO("Prepare: 签名验证失败...");
        return;
    }
    // 若通过, 将投票节点存入
    prepareVoteCount[prepare.Digest].emplace_back(prepare.NodeId); // FIXME: 是否需要上锁?
    int count    = prepareVoteCount[prepare.Digest].size();
    int minVotes = this->nodeNums;
    minVotes     = this->node.nodeId == leader ? this->nodeNums / 3 * 2 : (this->nodeNums / 3 * 2) - 1;
    if (count >= minVotes && !isCommit[prepare.Digest]) {
        std::string signature = sign(prepare.Digest, this->node.rsaPriKey);
        Commit commit         = {
                    prepare.ViewId,
                    prepare.Nonce,
                    this->node.nodeId,
                    prepare.Digest,
                    signature,
        };
        json jCommit = commit;
        NS_LOG_INFO("node" << GetNode()->GetId() << " 生成Commit消息: " << jCommit);
        // 广播
        std::string strCommit = jCommit.dump();
        this->Broadcast(FCommit, strCommit);
        NS_LOG_INFO("node" << GetNode()->GetId() << " 已完成Commit广播... 当前时间: " << Simulator::Now().GetSeconds() << "s");
        isCommit[commit.Digest] = true;
    }
}

// 处理 Commit
void Pbft::HandleCommit(std::string message) {
    NS_LOG_INFO("node" << GetNode()->GetId() << " 收到Commit 当前时间: " << Simulator::Now().GetSeconds() << "s");
    // 解析为 Commit对象
    auto jMessage = json::parse(message);
    Commit commit = jMessage.get<Commit>();
    // Test
    // std::cout << commit.NodeId << std::endl;
    RSA *nodePubKey = RSA_new();
    getPubKey(commit.NodeId, nodePubKey);
    if (this->view != commit.ViewId || this->nonce != commit.Nonce) {
        NS_LOG_INFO("Commit: 消息序号或视图序号验证失败...");
        return;
    } else if (this->prepareVoteCount.count(commit.Digest) == 0) { // prepare投票池无此消息
        NS_LOG_INFO("Commit: 摘要验证失败...");
        return;
    } else if (!verifySign(commit.Digest, nodePubKey, commit.Signature)) {
        NS_LOG_INFO("Commit: 签名验证失败...");
        return;
    }
    // 若通过, 将投票节点存入
    commitVoteCount[commit.Digest].emplace_back(commit.NodeId); // FIXME: 是否需要上锁?
    int count    = commitVoteCount[commit.Digest].size();
    int minVotes = this->nodeNums / 3 * 2;
    if (count >= minVotes && !isReply[commit.Digest] && isCommit[commit.Digest]) {
        // TODO: 模拟持久化层, 此处应替代为 sql
        Block block = this->tmpBlockPool[commit.Digest];
        this->localBlockPool.emplace_back(block);
        NS_LOG_INFO("node" << GetNode()->GetId() << " 已将区块写入本地区块池... 块高: "
                           << block.header.blockNumber << " 当前时间: " << Simulator::Now().GetSeconds() << "s");
        this->blockNumber++;
        // TODO: 是否添加客户端返回?
        isReply[commit.Digest] = true;
        // TODO: 继续出块, 暂时这样设置
        if (this->node.nodeId == leader) {
            EventId blockEvent = Simulator::ScheduleNow(&Pbft::HandleRequest, this);
        }
    }
}

// FIXME: view change
// void Pbft::viewChange(void) {
//     NS_LOG_INFO("当前ViewId: " << v << " Leader发生异常, 开始进行视图切换 当前时间: " << Simulator::Now().GetSeconds() << "s");
//     uint8_t data[4];
//     leader = (leader + 1) % N;
//     v += 1;
//     data[0] = intToChar(VIEW_CHANGE);
//     data[1] = intToChar(v);
//     data[2] = intToChar(leader);
//     Send(data);
// }

void SendPacket(Ptr<Socket> socketClient, Ptr<Packet> p) {
    socketClient->Send(p);
}

// 向所有邻居节点广播消息
void Pbft::Broadcast(std::string msgType, std::string message) {
    message = jointToMessage(msgType, message);
    // NS_LOG_INFO("广播消息: " << message << "消息长度: " << message.length());
    // std::string pre;
    // splitFromMessage(&message, &pre);
    // NS_LOG_INFO("前缀: " << pre << " " << pre.length());
    // NS_LOG_INFO("消息: " << message);
    uint8_t content[4096];
    // NS_LOG_INFO(" message.length() = " << message.length());
    uint32_t length = message.length();
    for (uint32_t i = 0; i < length; i++) {
        content[i] = message[i];
    }
    content[length] = '\0';
    // std::cout << "sizeof(content) = " << sizeof(content) << std::endl;
    Ptr<Packet> p = Create<Packet>(content, sizeof(content));
    // TODO: 改为无线
    TypeId tid                              = TypeId::LookupByName("ns3::UdpSocketFactory");
    std::vector<Ipv4Address>::iterator iter = node.peersAddresses.begin();
    while (iter != node.peersAddresses.end()) {
        TypeId tid               = TypeId::LookupByName("ns3::UdpSocketFactory");
        Ptr<Socket> socketClient = node.peersSockets[*iter];
        double delay             = getRandomDelay();
        Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
        iter++;
    }
}

std::string Pbft::getPacketContent(Ptr<Packet> packet, Address from) {
    // NS_LOG_INFO("包大小" << packet->GetSize());
    // FIXME:序列化需要小心这个地方，不是原始数组
    char *packetInfo = new char[packet->GetSize() + 1];
    std::ostringstream totalStream;
    packet->CopyData(reinterpret_cast<uint8_t *>(packetInfo), packet->GetSize());
    packetInfo[packet->GetSize()] = '\0'; // ensure that it is null terminated to avoid bugs
    std::map<Address, std::string> m_bufferedData;
    totalStream << m_bufferedData[from] << packetInfo;
    std::string totalReceivedData(totalStream.str());
    // std::cout << totalReceivedData << std::endl;
    return totalReceivedData;
}

// TODO: 向客户端发送消息使用, 暂时不用
/* void Pbft::Send(uint8_t data[], Address from) {
    Ptr<Packet> p;
    p = Create<Packet>(data, 4);
    // NS_LOG_INFO("packet: " << p);

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    Ptr<Socket> socketClient;
    if (!peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()]) {
        socketClient = Socket::CreateSocket(GetNode(), tid);
        socketClient->Connect(InetSocketAddress(InetSocketAddress::ConvertFrom(from).GetIpv4(), 7071));
        m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()] = socketClient;
    }
    socketClient = m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()];
    Simulator::Schedule(Seconds(getRandomDelay()), SendPacket, socketClient, p);
} */

} // namespace ns3