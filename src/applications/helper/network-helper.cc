#include "ns3/core-module.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "network-helper.h"
#include "../model/pbft-node.h"

namespace ns3 {

NetworkHelper::NetworkHelper(int totalNoNodes) {
    // 3.need changed to a specific typeId
    m_factory.SetTypeId("ns3::Pbft");
    nodeNums = totalNoNodes;
}

ApplicationContainer NetworkHelper::Install(NodeContainer c) {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); i++) {
        // 4.need changed to a specific protocol class
        // 配置blockchain app
        // 构造PbftNode
        Ptr<Pbft> app = m_factory.Create<Pbft>();
        app->nodeNums = nodeNums;
        int nodeId    = (*i)->GetId();
        app->node     = BcNode(nodeId, nodesConnectionsIps[nodeId]);
        app->leader   = (*c.Begin())->GetId(); // 默认leader为第一个节点
        // FIXME: 此处需要更改为无线节点类型，当前是ipv4
        (*i)->AddApplication(app);
        apps.Add(app);
    }
    // 返回的是所有app的集合apps
    return apps;
}

} // namespace ns3
