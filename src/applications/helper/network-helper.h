#ifndef NETWORK_HELPER_H
#define NETWORK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/uinteger.h"
#include <map>

namespace ns3 {

class NetworkHelper {
public:
    NetworkHelper(int totalNoNodes); // 构造函数

    std::map<uint32_t, std::vector<Ipv4Address>> nodesConnectionsIps; // 邻接表

    ApplicationContainer Install(NodeContainer c);

    // TODO:缺少设置参数

private:
    ObjectFactory m_factory;
    int nodeNums;
};
} // namespace ns3

#endif