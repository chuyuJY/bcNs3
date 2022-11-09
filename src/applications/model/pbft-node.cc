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
#include "pbft-node.h"
#include "stdlib.h"
#include "ns3/ipv4.h"
#include <ctime>
#include <map>

// 全局变量 是所有节点间共用的
int tx_size;
int tx_speed; // 交易生成的速率，单位为op/s
int n;        // 当前消息在视图中的编号
int v;        // 当前视图编号
int val;
float timeout; // 发送区块的间隔时间  默认为 50ms
int n_round;   // 共识轮数，其实应该是链高度？

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("PbftNode");

NS_OBJECT_ENSURE_REGISTERED(PbftNode);

TypeId
PbftNode::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::PbftNode")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<PbftNode>();

    return tid;
}

PbftNode::PbftNode(void) {
}

PbftNode::~PbftNode(void) {
    NS_LOG_FUNCTION(this);
}

static char intToChar(int a) {
    return a + '0';
}

static int charToInt(char a) {
    return a - '0';
}

// 信息接收延迟 3 - 6 ms
float getRandomDelay() {
    return ((rand() % 3) * 1.0 + 3) / 1000;
}

void printVector(std::vector<int> vec) {
    for (unsigned int i = 0; i < vec.size(); i++) {
        NS_LOG_INFO(vec[i] + " ");
    }
}

// 生成num笔交易 每个交易 tx_size KB
// 这个函数其实应该就是 generateBlock
static uint8_t *generateTX(int num) {
    int size = num * tx_size;
    uint8_t *data = (uint8_t *)std::malloc(size);
    /*
        TODO:
        1. 增加交易字段
        2. 增加签名方法
    */
    int i;
    for (i = 0; i < size; i++) {
        data[i] = '1';
    }
    data[i] = '\0';
    // NS_LOG_INFO("初始化成功: " << data);
    data[0] = intToChar(PRE_PREPARE);
    data[1] = intToChar(v);
    data[2] = intToChar(n);
    data[3] = intToChar(n); // 充当要修改的value

    return data;
}

void PbftNode::StartApplication() {
    // 初始化全局变量
    v = 1;          // 视图编号
    n = 0;          // 当前视图的交易序号
    leader = 0;     // 当前leader
    tx_size = 1024; // 1 KB
    tx_speed = 100; // 1000 tx/s
    timeout = 0.05; // 50 ms

    block_num = 0;
    // 共识轮数：执行了几轮共识
    n_round = 0;

    // 交易要更新的值
    val = intToChar(m_id);

    // 需要修改的value值
    // int value = 3;

    // 初始化socket
    if (!m_socket) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);

        // 注意 相当于监听所有网卡ip
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 7071);
        m_socket->Bind(local); // 绑定本机的ip和port
        m_socket->Listen();
    }
    // TODO: 查询SetRecvCallback作用
    // 在新数据可用时读取应用程序
    // 此回调旨在通知在阻塞套接字模型中已阻止数据可用于读取的套接字
    m_socket->SetRecvCallback(MakeCallback(&PbftNode::HandleRead, this));
    m_socket->SetAllowBroadcast(true);

    // iter是个指针，遍历当前节点的邻接表
    std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();
    // 与所有节点建立连接
    NS_LOG_INFO("node" << m_id << "正在与其他节点建立socket连接...");
    while (iter != m_peersAddresses.end()) {
        NS_LOG_INFO("node" << m_id << " 与 " << *iter << " 已建立socket连接");
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        Ptr<Socket> socketClient = Socket::CreateSocket(GetNode(), tid);
        socketClient->Connect(InetSocketAddress(*iter, 7071));
        m_peersSockets[*iter] = socketClient;
        iter++;
    }
    Simulator::Schedule(Seconds(timeout), &PbftNode::SendBlock, this);
    // n++;
    // }
}

void PbftNode::StopApplication() {
    // printVector(values);
}

void PbftNode::HandleRead(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    // NS_LOG_INFO("Hello World");
    while ((packet = socket->RecvFrom(from))) {
        socket->SendTo(packet, 0, from); // TODO: 没搞懂这步是干啥的，又发回给from？
        if (packet->GetSize() == 0) {    // EOF
            break;
        }
        if (InetSocketAddress::IsMatchingType(from)) {
            std::string msg = getPacketContent(packet, from);

            // NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s, Node " << GetNode ()->GetId () << " received " << packet->GetSize () << " bytes, msg[0]: "<< msg[0]);
            //     InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            //     InetSocketAddress::ConvertFrom (from).GetPort ());

            // // 打印接收到的结果
            // NS_LOG_INFO("node" << GetNode ()->GetId () << " 接收到消息: " << msg);
            uint8_t data[4];
            switch (charToInt(msg[0])) {
            case PRE_PREPARE: {
                // 收到预准备消息
                // FIXME: 解决leader节点多接收到一个prepare的问题
                NS_LOG_INFO("node" << GetNode()->GetId() << " 收到PrePrepare"
                                                            " 当前时间: "
                                   << Simulator::Now().GetSeconds() << "s");
                data[0] = intToChar(PREPARE);
                data[1] = msg[1]; // v
                data[2] = msg[2]; // n
                data[3] = msg[3]; // value
                // 序号值
                int num = charToInt(msg[2]);

                // 存储交易中的value值
                tx[num].val = charToInt(msg[3]);
                // if (num > n) {
                //     n = num;
                // }
                // 广播准备消息
                Send(data);
                break;
            }
            case PREPARE: {
                // 收到准备消息
                NS_LOG_INFO("node" << GetNode()->GetId() << " 收到Prepare"
                                                            " 当前时间: "
                                   << Simulator::Now().GetSeconds() << "s");
                data[0] = intToChar(PREPARE_RES);
                data[1] = msg[1]; // v
                data[2] = msg[2]; // n
                data[3] = intToChar(SUCCESS);
                // data[3] = msg[3];
                // 回复准备消息响应
                Send(data, from);
                break;
            }
            case PREPARE_RES: {
                // 收到准备消息响应
                int index = charToInt(msg[2]);
                if (charToInt(msg[3]) == SUCCESS) {
                    tx[index].prepare_vote++;
                }
                // if 超过半数SUCCESS, 则广播COMMIT
                // TODO:共识达成的条件
                if (tx[index].prepare_vote >= 2 * N / 3) {
                    data[0] = intToChar(COMMIT);
                    data[1] = msg[1]; // v
                    data[2] = msg[2]; // n
                    Send(data);
                    // NS_LOG_INFO("node"<< m_id << "收到Prepare: " << tx[index].prepare_vote);
                    tx[index].prepare_vote = 0;
                }
                break;
            }
            case COMMIT: {
                // 收到提交消息
                int index = charToInt(msg[2]);
                // NS_LOG_INFO("node"<< m_id << "收到Commit: " << tx[index].val);
                NS_LOG_INFO("node" << m_id << " 收到Commit"
                                              " 当前时间: "
                                   << Simulator::Now().GetSeconds() << "s");
                tx[index].commit_vote++;
                // 超过半数则 回复提交消息响应
                if (tx[index].commit_vote > 2 * N / 3) {
                    data[0] = intToChar(COMMIT_RES);
                    data[1] = intToChar(v);
                    data[2] = intToChar(n);
                    data[3] = SUCCESS; // n
                    // Send(data);
                    tx[index].commit_vote = 0;

                    // 记录交易到队列中
                    values.push_back(tx[index].val);

                    // NS_LOG_INFO("node"<< m_id << "加入交易 " << tx[index].val);
                    // NS_LOG_INFO("当前ViewId: " << v << ", node "<< m_id << " 完成 " << block_num << "th 区块, 改变的值为: " << values[block_num] << " 当前时间: " << Simulator::Now ().GetSeconds () << "s");
                    NS_LOG_INFO("当前ViewId: " << v << ", node " << m_id << " 完成 " << block_num << "th 区块"
                                               << " 当前时间: " << Simulator::Now().GetSeconds() << "s");
                    block_num++;
                    // n = n + 1;
                }
                // Send(data, from);
                break;
            }
                // case COMMIT_RES:
                // {
                //     // 如果超过半数则表示提交成功，reply客户端成功
                // }

            case VIEW_CHANGE: {
                int vt = charToInt(msg[1]);
                int lt = charToInt(msg[2]);
                v = vt;
                leader = lt;
                if (m_id == leader) {
                    NS_LOG_INFO("view-change完成, 当前Leader: node" << leader << " ViewId: " << v);
                }
            }

            default: {
                NS_LOG_INFO("Wrong msg");
                break;
            }
            }
        }
        socket->GetSockName(localAddress);
    }
}

void PbftNode::viewChange(void) {
    NS_LOG_INFO("当前ViewId: " << v << " Leader发生异常, 开始进行视图切换 当前时间: " << Simulator::Now().GetSeconds() << "s");
    uint8_t data[4];
    leader = (leader + 1) % N;
    v += 1;
    data[0] = intToChar(VIEW_CHANGE);
    data[1] = intToChar(v);
    data[2] = intToChar(leader);
    Send(data);
}

std::string
PbftNode::getPacketContent(Ptr<Packet> packet, Address from) {
    // NS_LOG_INFO("包大小" << packet->GetSize ());
    char *packetInfo = new char[packet->GetSize() + 1];
    std::ostringstream totalStream;
    packet->CopyData(reinterpret_cast<uint8_t *>(packetInfo), packet->GetSize());
    packetInfo[packet->GetSize()] = '\0'; // ensure that it is null terminated to avoid bugs
    /**
     * Add the buffered data to complete the packet
     */
    totalStream << m_bufferedData[from] << packetInfo;
    std::string totalReceivedData(totalStream.str());

    return totalReceivedData;
}

void SendPacket(Ptr<Socket> socketClient, Ptr<Packet> p) {
    socketClient->Send(p);
}

// 向某个指定地址的节点发送消息
void PbftNode::Send(uint8_t data[], Address from) {
    Ptr<Packet> p;
    p = Create<Packet>(data, 4);
    // NS_LOG_INFO("packet: " << p);

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    Ptr<Socket> socketClient;
    if (!m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()]) {
        socketClient = Socket::CreateSocket(GetNode(), tid);
        socketClient->Connect(InetSocketAddress(InetSocketAddress::ConvertFrom(from).GetIpv4(), 7071));
        m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()] = socketClient;
    }
    socketClient = m_peersSockets[InetSocketAddress::ConvertFrom(from).GetIpv4()];
    Simulator::Schedule(Seconds(getRandomDelay()), SendPacket, socketClient, p);
}

// 向所有邻居节点广播消息
void PbftNode::Send(uint8_t data[]) {
    // NS_LOG_INFO("广播消息");
    Ptr<Packet> p;
    p = Create<Packet>(data, 4);

    // TODO:
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

    while (iter != m_peersAddresses.end()) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

        Ptr<Socket> socketClient = m_peersSockets[*iter];
        double delay = getRandomDelay();
        // TODO:测试schedule
        Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
        iter++;
    }
}

// 向所有邻居节点广播区块
void PbftNode::SendBlock(void) {
    Ptr<Packet> p;
    // TODO: 定义Packet类，广播的内容
    int num = tx_speed / (1000 / (timeout * 1000)); // num = 交易数/区块数
    uint8_t *data = generateTX(num);
    int size = tx_size * num;
    p = Create<Packet>(data, size);

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

    if (m_id == leader) {
        NS_LOG_INFO("Leader node" << m_id << " 开始广播区块 当前时间: " << Simulator::Now().GetSeconds() << "s");
        // 还需要 FIXME:
        // tx[charToInt(data[2])].val = charToInt(data[3]);
        // 广播
        while (iter != m_peersAddresses.end()) {
            // TODO: tid???
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

            Ptr<Socket> socketClient = m_peersSockets[*iter];
            double delay = getRandomDelay();
            // 事件发生在 Seconds(delay)
            Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
            iter++;
        }
        n_round++;
        n++;

        // view_change， 概率为1/10
        // TODO: 此处的视图切换并不严格，是按照概率进行切换的；且只能由leader发起
        if (rand() % 10 == 5) {
            viewChange();
        }
    }

    blockEvent = Simulator::Schedule(Seconds(timeout), &PbftNode::SendBlock, this);
    // TODO: 发送40个区块就退出
    // 可以改成由leader打印.
    if (n_round == 10) {
        if (m_id == leader) {
            NS_LOG_INFO("已经发送了" << n_round << "个区块 at time: " << Simulator::Now().GetSeconds() << "s");
        }
        // Sleep(5 * 1000);    //程序延时5s
        Simulator::Cancel(blockEvent);
    }
}

// // 向所有邻居节点广播交易
// void
// RaftNode::SendTX (uint8_t data[], int num)
// {
//   NS_LOG_INFO("广播区块: " << round << ", time: " << Simulator::Now ().GetSeconds () << " s");
//   Ptr<Packet> p;
//   int size = tx_size * num;
//   p = Create<Packet> (data, size);

//   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

//   std::vector<Ipv4Address>::iterator iter = m_peersAddresses.begin();

//   while(iter != m_peersAddresses.end()) {
//     TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

//     Ptr<Socket> socketClient = m_peersSockets[*iter];
//     double delay = getRandomDelay();
//     Simulator::Schedule(Seconds(delay), SendPacket, socketClient, p);
//     iter++;
//   }
//   round++;
//   if (round == 50) {
//     NS_LOG_INFO("node" << m_id << " 已经发送了 "<< round << "个区块 at time: " << Simulator::Now ().GetSeconds () << "s");
//     // Simulator::Cancel (m_nextHeartbeat);
//     add_change_value = 0;
//   }
// }
} // namespace ns3