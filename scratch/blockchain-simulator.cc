#include "ns3/core-module.h"           //core模块，定义了ns-3的核心功能（如模拟事件，事件调度）
#include "ns3/network-module.h"        //network模块，基本的网络组件（如网络结点，分组和地址等）
#include "ns3/internet-module.h"       //internet模块，定义了TCP/IP协议栈
#include "ns3/point-to-point-module.h" //point-to-point模块
#include "ns3/applications-module.h"   //application模块,定义了应用层的分组收发模型
// #include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BlockchainSimulator");

// 创建网络
void startSimulator(int N) {
    NodeContainer nodes;
    nodes.Create(N);

    NetworkHelper networkHelper(N); // 区块链相关
    // 默认pointToPint只能连接两个节点，需要手动连接
    // 通过helper类实例化网络设备，像设备添加MAC地址和队列，然后安装到节点
    //使用PointToPointHelper，负责设置网络设备和信道属性
    PointToPointHelper pointToPoint;

    // 节点总带宽24Mbps，分到每一个点对点通道上为3Mbps
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("3Mbps"));
    //告诉PointToPointHelper使用"3ms"（3毫秒）作为每一个被创建的点到点信道传输延时值。
    pointToPoint.SetChannelAttribute("Delay", StringValue("3ms"));
    // uint32_t nNodes = nodes.GetN ();

    // 给每一个节点安装一个协议栈
    InternetStackHelper stack;
    stack.Install(nodes);
    /*
        从1.0.0.0开始以子网掩码为255.255.255.0分配地址
        地址分配默认是从1开始并单调的增长.
    */
    Ipv4AddressHelper address;
    address.SetBase("1.0.0.0", "255.255.255.0");

    // 两两建立连接
    // FIXME: 此处已修改
    NS_LOG_INFO("正在分配IP地址...");
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            Ptr<Node> p1 = nodes.Get(i);
            Ptr<Node> p2 = nodes.Get(j);
            // NetDeviceContainer对象存放NetDevice（网络设备）对象列表
            NetDeviceContainer device = pointToPoint.Install(p1, p2);
            // 该容器可以管理分配的IP地址
            Ipv4InterfaceContainer interface = address.Assign(device);
            // 分别为索引号为0, 1的node分配地址
            // interface.Add(address.Assign(device.Get(0)));
            // interface.Add(address.Assign(device.Get(1)));
            NS_LOG_INFO("node" << i << " address: " << interface.GetAddress(0));
            NS_LOG_INFO("node" << j << " address: " << interface.GetAddress(1));
            // 构建邻接表, 双向建立
            networkHelper.nodesConnectionsIps[i].push_back(interface.GetAddress(1));
            networkHelper.nodesConnectionsIps[j].push_back(interface.GetAddress(0));

            // 创建新的网络: 如果不增加网络的话, 所有ip都在一个字网，而最后一块device会覆盖之前的设置，导致无法通过ip访问到之前的邻居节点
            // 应该的设置：每个device连接的两个节点在一个子网内，所以每分配一次ip，地址应该增加一个网段
            address.NewNetwork();
        }
    }
    ApplicationContainer nodeApp = networkHelper.Install(nodes);
    // 打开所有节点的app
    // 进入模拟后，1s后开始，10s后结束
    nodeApp.Start(Seconds(0.0));
    nodeApp.Stop(Seconds(10.0));

    // AnimationInterface anim("bc.xml");
    // 在Simulator::Run ()调用之前
    // 一般会利用Simulator::Schedule()和Simulator::ScheduleWithContext()将事件压栈，
    // 这个函数就是开始消耗栈内事件。
    Simulator::Run();
    Simulator::Destroy();
}

int main(int argc, char *argv[]) {
    // 启用日志
    LogComponentEnable("BlockchainSimulator", LOG_INFO);

    CommandLine cmd;
    cmd.Parse(argc, argv);

    // numbers of nodes in network
    int N = 4;

    // 设置当前使用的时间单位
    Time::SetResolution(Time::NS);

    // 1.need changed to a specific protocol class
    LogComponentEnable("Pbft", LOG_LEVEL_INFO);

    // start the simulator
    startSimulator(N);

    return 0;
}