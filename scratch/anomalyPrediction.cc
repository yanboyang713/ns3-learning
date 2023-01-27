//./ns3 run "anomalyPrediction --pcap=false --printRoutes=true --numOfUAVs=3 --time=5 --step=1 --EnableMonitor=true"
#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"

#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-mac-queue.h"

using namespace ns3;

class anomalyPrediction{
    public:
        anomalyPrediction ();
        bool Configure (int argc, char **argv);
        void Run ();
        void Report (std::ostream & os);

    private:
        // Number of UAVs
        uint32_t numOfUAVs;
        // Distance between nodes, meters
        double step;
        // Simulation time, seconds
        double totalTime;
        // Write per-device PCAP traces if true
        bool pcap;
        // Print routes if true
        bool printRoutes;

        //On-Off
        //Transport layer payload size in bytes.
        uint32_t payloadSize;
        //Application layer datarate
        std::string dataRate;

        bool enableFlowMonitor = true;
        uint16_t port = 9;   // Discard port (RFC 863)
        uint32_t bytesTotal;      //!< Total received bytes.
        uint32_t packetsReceived; //!< Total received packets.

        // UAVs network
        NodeContainer UAVs;
        NetDeviceContainer UAVdevices;
        Ipv4InterfaceContainer UAVinterfaces;

        //Ground Station
        NodeContainer groundStation;
        NetDeviceContainer groundStationDevice;
        Ipv4InterfaceContainer groundStationInterface;

        //FlowMonitor

        // Create the nodes
        void CreateNodes ();
        //Set Mobility Model
        void SetMobilityModel ();
        // Create the devices
        void CreateDevices ();
        // Create the network
        void InstallInternetStack ();
        // Routing
        void SetRouting (Ipv4StaticRoutingHelper &);

        // Create the simulation applications

        void ConfigConnect();

        void InstallApplications ();

        void ShowFlowMonitor (FlowMonitorHelper &, Ptr<FlowMonitor> &);
};
static void PrintPacketInfo (Ptr <const Packet> packet){
    std::cout << "Recv. Packet of size " << packet->GetSize() << std::endl;
    std::cout << "UID: " << packet->GetUid() << std::endl;
    packet->Print(std::cout);
    return;
}
static void RxPacketInfo(std::string context, Ptr <const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector,
                         MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId) {

    std::cout << "RX Packet Info" << std::endl;
    std::cout << context << std::endl;
    std::cout << "Signal= " << signalNoise.signal << " Noise= " << signalNoise.noise << std::endl;

    PrintPacketInfo (packet);

    return;
}

static void TxPacketInfo(std::string context, Ptr <const Packet> packet, uint16_t channelFreqMhz,
                         WifiTxVector txVector, MpduInfo aMpdu, uint16_t staId){

    std::cout << "TX Packet Info" << std::endl;
    std::cout << context << std::endl;
    PrintPacketInfo (packet);

    return;
}
static void DequeueTrace(std::string context, Ptr<const WifiMacQueueItem> item){
    double QueuingDelay = Simulator::Now() - item->GetTimeStamp();
    std::cout << "QueuingDelay: " << QueuingDelay << std::endl;

    return;
}
//static void PhyTxDropInfo(std::string context, Ptr <const Packet> packet, WifiPhyRxfailureReason reason){
static void PhyTxDropInfo(std::string context, Ptr <const Packet> packet){
    std::cout << "TX Drop Info" << std::endl;
    std::cout << context << std::endl;
    PrintPacketInfo (packet);

    return;
}

//static void PhyRxDropInfo(std::string context, Ptr <const Packet> packet){
static void PhyRxDropInfo(std::string context, Ptr <const Packet> packet, WifiPhyRxfailureReason reason){
    std::cout << "RX Drop Info" << std::endl;
    std::cout << context << std::endl;
    PrintPacketInfo (packet);
    std::cout << "RX Drop Reason: " << reason << std::endl;

    return;
}
int main (int argc, char **argv){
    anomalyPrediction main;

    if (!main.Configure (argc, argv))
        NS_FATAL_ERROR ("Configuration failed. Aborted.");

    main.Run ();

    return 0;
}

anomalyPrediction::anomalyPrediction ():
    numOfUAVs (3),
    step (50),
    totalTime (100),
    pcap (false),
    printRoutes (false),
    payloadSize (1472),
    dataRate ("100Mbps"),

    enableFlowMonitor (false),
    port (9),
    bytesTotal (0),
    packetsReceived (0)
{
}

bool anomalyPrediction::Configure (int argc, char **argv)
{
  SeedManager::SetSeed (12345);
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.

  CommandLine cmd (__FILE__);

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("numOfUAVs", "Number of UAVs.", numOfUAVs);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data ate", dataRate);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);

  return true;
}

void anomalyPrediction::ConfigConnect (){

    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback (&RxPacketInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback (&TxPacketInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDropInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDropInfo));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Dequeue", MakeCallback(&DequeueTrace));

    return;
}
void anomalyPrediction::Run (){
    CreateNodes ();
    SetMobilityModel ();
    CreateDevices ();
    InstallInternetStack ();
    InstallApplications ();


    std::cout << "Starting simulation for " << totalTime << " s ...\n";

    Simulator::Stop (Seconds (totalTime));
    ConfigConnect();

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Run ();

    if (enableFlowMonitor == true)
        ShowFlowMonitor (flowmon, monitor);

    Simulator::Destroy ();

    return;
}

void anomalyPrediction::ShowFlowMonitor (FlowMonitorHelper &flowmon, Ptr<FlowMonitor> &monitor){
    monitor->CheckForLostPackets ();

    // calculate tx, rx, throughput, end to end delay, dand packet delivery ratio
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){

        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";

        if ((t.sourceAddress=="10.1.1.4" && t.destinationAddress == "10.1.1.1")){
            std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
            std::cout << "  End to End Delay: " << i->second.delaySum.GetSeconds() / i->second.txPackets <<"\n";
            std::cout << "  Packet Delivery Ratio: " << ((i->second.rxPackets * 100) / i->second.txPackets) << "%" << "\n";
        }
    }

    return;
}
void anomalyPrediction::CreateNodes () {
    //Creating UAVs and Ground Station
    std::cout << "Creating " << (unsigned)numOfUAVs << " UAVs " << std::endl;
    UAVs.Create (numOfUAVs);
    // Name UAVs
    for (uint32_t i = 0; i < numOfUAVs; ++i) {
        std::ostringstream os;
        os << "UAV-" << i;
        Names::Add (os.str (), UAVs.Get (i));
    }


    std::cout << "Creating " << "Ground Station" << std::endl;
    groundStation.Create (1);

    //Name Ground Station
    std::string groundStationName = "Ground-Station";
    Names::Add (groundStationName, groundStation.Get (0));

    return;
}

void anomalyPrediction::SetMobilityModel () {
    // Create static grid
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (step),
                                   "DeltaY", DoubleValue (0),
                                   "GridWidth", UintegerValue (numOfUAVs + 1),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (groundStation);
    mobility.Install (UAVs);

    return;
}
void anomalyPrediction::CreateDevices () {
    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    //YansWifiChannelHelper wifiChannel;
    //wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
    wifiPhy.SetChannel (wifiChannel.Create ());
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211ac);

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
    UAVdevices = wifi.Install (wifiPhy, wifiMac, UAVs);
    groundStationDevice = wifi.Install (wifiPhy, wifiMac, groundStation);

    if (pcap == true){
        wifiPhy.EnablePcapAll (std::string ("UAVs-network"));
    }

    return;
}

void anomalyPrediction::InstallInternetStack () {
    Ipv4StaticRoutingHelper staticRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper (staticRouting);
    stack.Install (groundStation);
    stack.Install (UAVs);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    groundStationInterface = address.Assign (groundStationDevice);
    UAVinterfaces = address.Assign (UAVdevices);

    //Print IP Address
    std::cout << "Ground Station IP: " << groundStationInterface.GetAddress(0) << std::endl;
    for (uint32_t i = 0; i < numOfUAVs; i++){
        std::cout << "UAV " << i <<" IP: " << UAVinterfaces.GetAddress(i) << std::endl;
    }

    SetRouting (staticRouting);

    return;
}

void anomalyPrediction::SetRouting (Ipv4StaticRoutingHelper &staticRouting){
    //Ground Station uplink
    Ptr<Ipv4> GS = groundStation.Get (0)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> GSstaticRouting = staticRouting.GetStaticRouting (GS);


    for (uint32_t i = 0; i < numOfUAVs; i++){
        GSstaticRouting->AddHostRouteTo (UAVinterfaces.GetAddress(i), UAVinterfaces.GetAddress(0), 1);
    }

    //UAV uplink
    for (uint32_t m = 0; m < numOfUAVs; m++){
        Ptr<Ipv4> UAVobject = UAVs.Get (m)->GetObject<Ipv4> ();
        Ptr<Ipv4StaticRouting> UAVsUplinkStaticRouting = staticRouting.GetStaticRouting (UAVobject);

        for (uint32_t n = m + 1; n < numOfUAVs; n++){
            UAVsUplinkStaticRouting->AddHostRouteTo (UAVinterfaces.GetAddress(n), UAVinterfaces.GetAddress(m+1), 1);
        }
    }
    //UAV downlink
    for (int m = numOfUAVs - 1; m >= 0; m--){
        Ptr<Ipv4> UAVobject = UAVs.Get (m)->GetObject<Ipv4> ();
        Ptr<Ipv4StaticRouting> UAVsDownlinkStaticRouting = staticRouting.GetStaticRouting (UAVobject);

        if (m == 0){
            UAVsDownlinkStaticRouting->AddHostRouteTo (groundStationInterface.GetAddress(0), groundStationInterface.GetAddress(0), 1);
            break;
        }

        for (int n = m - 1; n >= 0; n--){
            UAVsDownlinkStaticRouting->AddHostRouteTo (UAVinterfaces.GetAddress(n), UAVinterfaces.GetAddress(m - 1), 1);
        }
        UAVsDownlinkStaticRouting->AddHostRouteTo (groundStationInterface.GetAddress(0), UAVinterfaces.GetAddress(m - 1), 1);
    }

    if (printRoutes == true){
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("static.routes", std::ios::out);
        staticRouting.PrintRoutingTableAllAt (Seconds (0.01), routingStream);
    }

    return;
}

void anomalyPrediction::InstallApplications ()
{
    // Install TCP/UDP Transmitter on the station

    OnOffHelper server ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (groundStationInterface.GetAddress(0), port)));
    server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
    server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=42]"));
    server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));

    ApplicationContainer serverApps = server.Install (UAVs.Get(numOfUAVs - 1));
    serverApps.Start (Seconds (0.1));
    serverApps.Stop (Seconds (totalTime) - Seconds (0.001));

    ApplicationContainer sinkApps;
    PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny (),port));
    sinkApps.Add(sink.Install(groundStation.Get(0)));
    sinkApps.Start (Seconds (0.2));
    sinkApps.Stop (Seconds (totalTime) - Seconds (0.001));

    return;
}
