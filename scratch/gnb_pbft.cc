
//测试基站跑pbft

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-module.h"
#include "ns3/config-store-module.h"
#include "ns3/antenna-module.h"

// 4×4

/**
 * \file cttc-3gpp-channel-nums.cc
 * \ingroup examples
 * \brief Simple topology numerologies example.
 *
 * This example allows users to configure the numerology and test the end-to-end
 * performance for different numerologies. In the following figure we illustrate
 * the simulation setup.
 *
 * For example, UDP packet generation rate can be configured by setting
 * "--lambda=1000". The numerology can be toggled by the argument,
 * e.g. "--numerology=1". Additionally, in this example two arguments
 * are added "bandwidth" and "frequency", both in Hz units. The modulation
 * scheme of this example is in test mode, and it is fixed to 28.
 *
 * By default, the program uses the 3GPP channel model, without shadowing and with
 * line of sight ('l') option. The program runs for 0.4 seconds and one single
 * packet is to be transmitted. The packet size can be configured by using the
 * following parameter: "--packetSize=1000".
 *
 * This simulation prints the output to the terminal and also to the file which
 * is named by default "cttc-3gpp-channel-nums-fdm-output" and which is by
 * default placed in the root directory of the project.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./ns3 run cttc-3gpp-channel-nums
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BlockchainSimulator");

NetDeviceContainer
NodesConnect(Ptr<Node> a, Ptr<Node> b) {
    PointToPointHelper p2phelp;
    p2phelp.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2phelp.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2phelp.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10))); // 10ms 延时
    NetDeviceContainer iD = p2phelp.Install(a, b);

    return iD;
}

int main(int argc, char *argv[]) {
    // enable logging or not
    bool logging = true;
    if (logging) {
        LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
        LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
        LogComponentEnable("LtePdcp", LOG_LEVEL_INFO);
        LogComponentEnable("BlockchainSimulator", LOG_INFO);
        LogComponentEnable("Pbft", LOG_LEVEL_INFO);
    }
    // set simulation time and mobility
    // double simTime         = 5;   // seconds
    // double udpAppStartTime = 0.4; // seconds

    // other simulation parameters default values
    uint16_t numerology = 0;

    uint16_t gNbNum      = 4;
    uint16_t ueNumPergNb = 2;

    double centralFrequency = 7e9;
    double bandwidth        = 100e6;
    double txPower          = 14;
    // double lambda           = 1000;
    // uint32_t udpPacketSize  = 1000;
    // bool udpFullBuffer = true;
    uint8_t fixedMcs = 28;
    bool useFixedMcs = true;
    // bool singleUeTopology   = false;
    // // Where we will store the output files.
    // std::string simTag = "default";
    // std::string outputDir = "./";

    CommandLine cmd;
    cmd.AddValue("gNbNum", "The number of gNbs in multiple-ue topology", gNbNum);
    cmd.AddValue("ueNumPergNb", "The number of UE per gNb in multiple-ue topology", ueNumPergNb);

    cmd.Parse(argc, argv);

    NS_ASSERT(ueNumPergNb > 0);

    // setup the nr simulation
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();

    // 物理层设置 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

    /*
     * Spectrum division. We create one operation band with one component carrier
     * (CC) which occupies the whole operation band bandwidth. The CC contains a
     * single Bandwidth Part (BWP). This BWP occupies the whole CC band.
     * Both operational bands will use the StreetCanyon channel modeling.
     */
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand           = 1; // in this example, both bands have a single CC
    BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::RMa_LoS;
    if (ueNumPergNb > 1) {
        scenario = BandwidthPartInfo::InH_OfficeOpen;
    }

    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, numCcPerBand,
                                                   scenario);

    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);

    /*
     * Initialize channel and pathloss, plus other things inside band1. If needed,
     * the band configuration can be done manually, but we leave it for more
     * sophisticated examples. For the moment, this method will take care
     * of all the spectrum initialization needs.
     */
    nrHelper->InitializeOperationBand(&band);

    BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps({band});

    /*
     * Continue setting the parameters which are common to all the nodes, like the
     * gNB transmit power or numerology.
     */
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPower));
    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(numerology));

    // Scheduler
    nrHelper->SetSchedulerTypeId(TypeId::LookupByName("ns3::NrMacSchedulerTdmaRR"));
    nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(useFixedMcs));
    nrHelper->SetSchedulerAttribute("FixedMcsUl", BooleanValue(useFixedMcs));

    if (useFixedMcs == true) {
        nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(fixedMcs));
        nrHelper->SetSchedulerAttribute("StartingMcsUl", UintegerValue(fixedMcs));
    }

    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<ThreeGppAntennaModel>()));

    // Beamforming method
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);

    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    //  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
    nrHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    // Error Model: UE and GNB with same spectrum error model.
    nrHelper->SetUlErrorModel("ns3::NrEesmIrT1");
    nrHelper->SetDlErrorModel("ns3::NrEesmIrT1");

    // Both DL and UL AMC will have the same model behind.
    nrHelper->SetGnbDlAmcAttribute(
        "AmcModel", EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
    nrHelper->SetGnbUlAmcAttribute(
        "AmcModel", EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

    //物理层设置 ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

    /*
     *  Create the gNB and UE nodes according to the network topology
     */
    NodeContainer gNbNodes;
    NodeContainer ueNodes;

    //场景设置
    int64_t randomStream = 1;
    GridScenarioHelper gridScenario;
    gridScenario.SetRows(gNbNum); //基站分布为几行几列
    gridScenario.SetColumns(gNbNum / 2);
    gridScenario.SetHorizontalBsDistance(50); //基站间距
    gridScenario.SetVerticalBsDistance(50);   //基站间距
    gridScenario.SetBsHeight(1.5);            //基站高度，二维可视化看不出来
    gridScenario.SetUtHeight(1.5);
    // must be set before BS number
    gridScenario.SetSectorization(GridScenarioHelper::SINGLE); //设置扇区数量
    gridScenario.SetBsNumber(gNbNum);
    gridScenario.SetUtNumber(ueNumPergNb * gNbNum);
    gridScenario.SetScenarioHeight(30); // Create a 3x3 scenario where the UE will
    gridScenario.SetScenarioLength(30); // be distribuited.  UE分布在基站周围的范围大小
    // gridScenario.SetStartingPosition(Vector (-50, -20, 0));
    gridScenario.SetStartingPosition(Vector(-20, -20, 0));
    randomStream += gridScenario.AssignStreams(randomStream);
    gridScenario.CreateScenario();

    ueNodes  = gridScenario.GetUserTerminals();
    gNbNodes = gridScenario.GetBaseStations();

    // Create EPC helper
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    nrHelper->SetEpcHelper(epcHelper);
    // Core latency
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    // gNb routing between Bearer and bandwidh part
    uint32_t bwpIdForBearer = 0;
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("GBR_CONV_VOICE", UintegerValue(bwpIdForBearer));
    // nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForBearer));

    // Initialize nrHelper
    nrHelper->Initialize();

    // Install nr net devices
    NetDeviceContainer gNbNetDev = nrHelper->InstallGnbDevice(gNbNodes, allBwps);

    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes, allBwps);

    // int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gNbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    // When all the configuration is done, explicitly call UpdateConfig ()

    for (auto it = gNbNetDev.Begin(); it != gNbNetDev.End(); ++it) {
        DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();
    }

    for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it) {
        DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();
    }

    //设置节点位置
    Ptr<Node> pgw = epcHelper->GetPgwNode();
    Ptr<Node> sgw = epcHelper->GetSgwNode();
    Ptr<Node> mme = epcHelper->GetMmeNode();

    MobilityHelper pos_EPC;
    Ptr<ListPositionAllocator> pos_sgw_node = CreateObject<ListPositionAllocator>();
    pos_sgw_node->Add(Vector(50, 20, 0));
    Ptr<ListPositionAllocator> pos_pgw_node = CreateObject<ListPositionAllocator>();
    pos_pgw_node->Add(Vector(50, 0, 0));
    Ptr<ListPositionAllocator> pos_mme_node = CreateObject<ListPositionAllocator>();
    pos_mme_node->Add(Vector(50, -20, 0));
    pos_EPC.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    pos_EPC.SetPositionAllocator(pos_sgw_node);
    pos_EPC.Install(sgw);
    pos_EPC.SetPositionAllocator(pos_pgw_node);
    pos_EPC.Install(pgw);
    pos_EPC.SetPositionAllocator(pos_mme_node);
    pos_EPC.Install(mme);

    //设置远端节点位置
    NodeContainer remoteHostContainer;
    uint32_t remote_n = 2;
    remoteHostContainer.Create(remote_n);

    Ptr<Node> remoteHost0 = remoteHostContainer.Get(0);
    Ptr<Node> remoteHost1 = remoteHostContainer.Get(1);
    MobilityHelper pos_remoteHost;
    Ptr<ListPositionAllocator> pos_r0 = CreateObject<ListPositionAllocator>();
    pos_r0->Add(Vector(100, 20, 0));
    Ptr<ListPositionAllocator> pos_r1 = CreateObject<ListPositionAllocator>();
    pos_r1->Add(Vector(100, 0, 0));
    pos_remoteHost.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    pos_remoteHost.SetPositionAllocator(pos_r0);
    pos_remoteHost.Install(remoteHost0);
    pos_remoteHost.SetPositionAllocator(pos_r1);
    pos_remoteHost.Install(remoteHost1);

    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    //连接EPC和远端节点
    NetDeviceContainer iD_rh0_pgw = NodesConnect(pgw, remoteHost0);
    NetDeviceContainer iD_rh1_pgw = NodesConnect(pgw, remoteHost1);

    //分配ipv4地址
    Ipv4AddressHelper ipv4h0;
    Ipv4AddressHelper ipv4h1;

    ipv4h0.SetBase("1.0.0.0", "255.0.0.0");
    ipv4h1.SetBase("2.0.0.0", "255.0.0.0");

    Ipv4InterfaceContainer internetIpIfaces0 = ipv4h0.Assign(iD_rh0_pgw);
    Ipv4InterfaceContainer internetIpIfaces1 = ipv4h1.Assign(iD_rh1_pgw);

    //设置路由
    Ipv4StaticRoutingHelper ipv4RoutingHelper0;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting0 =
        ipv4RoutingHelper0.GetStaticRouting(remoteHost0->GetObject<Ipv4>());
    remoteHostStaticRouting0->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);
    Ipv4StaticRoutingHelper ipv4RoutingHelper1;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting1 =
        ipv4RoutingHelper1.GetStaticRouting(remoteHost1->GetObject<Ipv4>());
    remoteHostStaticRouting1->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    internet.Install(ueNodes);

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Set the default gateway for the UEs
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    for (uint32_t j = 0; j < ueNodes.GetN(); ++j) {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ueNodes.Get(j)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    // attach UEs to the closest eNB
    nrHelper->AttachToClosestEnb(ueNetDev, gNbNetDev);

    // 设置共识↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
    Ipv4StaticRoutingHelper ipv4RoutingHelper11;
    for (unsigned int i = 0; i < gNbNodes.GetN(); i++) {
        NS_LOG_UNCOND(gNbNodes.Get(i)->GetId());

        Ptr<Ipv4StaticRouting> remoteHostStaticRouting11 =
            ipv4RoutingHelper11.GetStaticRouting(gNbNodes.Get(i)->GetObject<Ipv4>());
        remoteHostStaticRouting11->AddNetworkRouteTo(Ipv4Address("10.0.0.0"),
                                                     Ipv4Mask("255.0.0.0"), 1);
    }

    NetworkHelper networkHelper(gNbNodes.GetN());

    for (unsigned int i = 0; i < gNbNodes.GetN(); i++) {
        for (unsigned int j = 0; j < gNbNodes.GetN(); j++) {
            if (i != j) {
                Ipv4Address gnbAddress = gNbNodes.Get(j)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
                networkHelper.nodesConnectionsIps[gNbNodes.Get(i)->GetId()].push_back(gnbAddress);
            }
        }
    }
    ApplicationContainer nodeApp = networkHelper.Install(gNbNodes);

    nodeApp.Start(Seconds(0.0));
    nodeApp.Stop(Seconds(10.0));
    // enable the traces provided by the nr module
    nrHelper->EnableTraces();

    Simulator::Stop(Seconds(0.1));
    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
