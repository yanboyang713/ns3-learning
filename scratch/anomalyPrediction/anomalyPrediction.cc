//CXX="clang++" LDFLAGS="-lpqxx -lpq" ./ns3 configure
//./ns3 run "anomalyPrediction --pcap=false --printRoutes=true --numOfUAVs=3 --time=5 --step=1 --EnableMonitor=true --hostname=test"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "ns3/sha256.h"
#include "ns3/database.h"

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
        //The operating frequency band in GHz: 2.4, 5 or 6
        double frequency;
        //The constant channel width in MHz (only for 11n/ac/ax)
        uint16_t channelWidth;
        //The guard interval in nanoseconds (800 or 400 for 11n/ac, 800 or 1600 or 3200 for 11 ax)
        uint16_t guardIntervalNs;
        uint16_t ChannelNumber;

        // The 802.11 standard
        std::string standard;
        // Data Mode
        std::string dataMode;
        //Control Mode
        std::string controlMode;

        // Host Name
        char hostname[1024];

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

        bool enableFlowMonitor;
        // Discard port (RFC 863)
        uint16_t port;

        // UAVs network
        NodeContainer UAVs;
        NetDeviceContainer UAVdevices;
        Ipv4InterfaceContainer UAVinterfaces;

        //Ground Station
        NodeContainer groundStation;
        NetDeviceContainer groundStationDevice;
        Ipv4InterfaceContainer groundStationInterface;

        //database
        //database data("dbname = anomalyprediction user = postgres password = password \
      //hostaddr = 127.0.0.1 port = 5432");
        //database data;

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

        void RunRecord ();

        std::string generateRunID ();
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
/*
static void DequeueTrace(std::string context, Ptr<const WifiMacQueueItem> item){
    double QueuingDelay = Simulator::Now() - item->GetTimeStamp();
    std::cout << "QueuingDelay: " << QueuingDelay << std::endl;

    return;
}
 */
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
    database data;
    bool succ = data.connect();
    if (succ == true) {
        std::cout << "database connect success" << std::endl;
    }

    anomalyPrediction main;


    if (!main.Configure (argc, argv))
        NS_FATAL_ERROR ("Configuration failed. Aborted.");

    main.Run ();

    return 0;
}
std::string anomalyPrediction::generateRunID (){
    // current date/time based on current system
    time_t now = time(0);
	// Providing a seed value
	srand(now);

    int randomNum = rand();
    std::string seed = std::to_string(randomNum) + hostname;

    //http://www.zedwood.com/article/cpp-sha256-function
    return sha256(seed);
}
void anomalyPrediction::RunRecord (){

    std::string runID = generateRunID();
    std::cout << "RunID: " << runID << std::endl;
    std::cout << "Hostname: " << hostname << std::endl;

    std::cout << "Data Mode: " << dataMode << std::endl;
    std::cout << "Control Mode: " << controlMode << std::endl;

    std::cout << "numOfUAVs: " << numOfUAVs << std::endl;
    std::cout << "Simulation time: " << totalTime << std::endl;
    std::cout << "Grid step (m): " << step << std::endl;
    std::cout << "payloadSize (bytes)" << payloadSize << std::endl;
    std::cout << "dataRate: " << dataRate << std::endl;
    std::cout << "ChannelNumber: " << ChannelNumber << std::endl;
    std::cout << "frequency: " << frequency << std::endl;
    std::cout << "WIFI Standard: " << standard << std::endl;
    std::cout << "channelWidth: " << channelWidth << std::endl;
    std::cout << "guardIntervalNs: " << guardIntervalNs << std::endl;

    return;
}

anomalyPrediction::anomalyPrediction ():
    frequency (6), //< The operating frequency band in GHz: 2.4, 5 or 6
    channelWidth (160), //< The constant channel width in MHz (only for 11n/ac/ax)
    guardIntervalNs (3200),
    ChannelNumber (15),
    standard("11ax"), ///< the 802.11 standard
    numOfUAVs (3),
    step (50),
    totalTime (100),
    pcap (false),
    printRoutes (false),
    payloadSize (1472),
    dataRate ("700Mbps"),

    dataMode ("HeMcs11"),
    controlMode ("HeMcs0"),

    enableFlowMonitor (false),
    port (9)
{
    //Set Hostname
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
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
  cmd.AddValue ("DataMode", "Data Mode", dataMode);
  cmd.AddValue ("ControlMode", "Control Mode", controlMode);


  //https://www.nsnam.org/docs/models/html/wifi-user.html#wifiphy-channelnumber
  cmd.AddValue("ChannelNumber", "Set the operating channel number", ChannelNumber);
  cmd.AddValue("hostname", "Set Hostname", hostname);
  cmd.AddValue("frequency", "Set the operating frequency band in GHz: 2.4, 5 or 6", frequency);
  cmd.AddValue("standard", "Set the standard (11a, 11b, 11g, 11n, 11ac, 11ax)", standard);
  cmd.AddValue("channelWidth",
               "Set the constant channel width in MHz (only for 11n/ac/ax)",
               channelWidth);
  cmd.AddValue("guardIntervalNs",
               "Set the the guard interval in nanoseconds (800 or 400 for 11n/ac, 800 or 1600 or "
               "3200 for 11 ax)",
               guardIntervalNs);

  cmd.Parse (argc, argv);

  return true;
}

void anomalyPrediction::ConfigConnect (){

    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback (&RxPacketInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback (&TxPacketInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDropInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDropInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WaveNetDevice/MacEntities/*/$ns3::OcbWifiMac/*/Queue/Dequeue", MakeCallback(&DequeueTrace));

    return;
}
void anomalyPrediction::Run (){

    //std::cout << "database connect: " << data.connect() << std::endl;

    RunRecord ();
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
    //YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    if (frequency == 6){
        // Reference Loss for Friss at 1 m with 6.0 GHz
        wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                       "Exponent",
                                       DoubleValue(2.0),
                                       "ReferenceDistance",
                                       DoubleValue(1.0),
                                       "ReferenceLoss",
                                       DoubleValue(49.013));
    } else if (frequency == 5){
        // Reference Loss for Friss at 1 m with 5.15 GHz
        wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                       "Exponent",
                                       DoubleValue(2.0),
                                       "ReferenceDistance",
                                       DoubleValue(1.0),
                                       "ReferenceLoss",
                                       DoubleValue(46.6777)
);

    } else {
        // Reference Loss for Friss at 1 m with 2.4 GHz
        wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                       "Exponent",
                                       DoubleValue(2.0),
                                       "ReferenceDistance",
                                       DoubleValue(1.0),
                                       "ReferenceLoss",
                                       DoubleValue(40.046));
    }

    wifiPhy.SetChannel (wifiChannel.Create ());
    wifiPhy.Set ("Antennas", UintegerValue (4));
    wifiPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
    wifiPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));


    WifiStandard wifiStandard;
    if (standard == "11a")
    {
        wifiStandard = WIFI_STANDARD_80211a;
        frequency = 5;
        channelWidth = 20;
    }
    else if (standard == "11b")
    {
        wifiStandard = WIFI_STANDARD_80211b;
        frequency = 2.4;
        channelWidth = 22;
    }
    else if (standard == "11g")
    {
        wifiStandard = WIFI_STANDARD_80211g;
        frequency = 2.4;
        channelWidth = 20;
    }
    else if (standard == "11n")
    {
        if (frequency == 2.4)
        {
            wifiStandard = WIFI_STANDARD_80211n;
        }
        else if (frequency == 5)
        {
            wifiStandard = WIFI_STANDARD_80211n;
        }
        else
        {
            NS_FATAL_ERROR("Unsupported frequency band " << frequency << " GHz for standard "
                                                         << standard);
        }
    }
    else if (standard == "11ac")
    {
        wifiStandard = WIFI_STANDARD_80211ac;
        frequency = 5;
    }
    else if (standard == "11ax")
    {
        if (frequency == 2.4)
        {
            wifiStandard = WIFI_STANDARD_80211ax;
        }
        else if (frequency == 5)
        {
            wifiStandard = WIFI_STANDARD_80211ax;
        }
        else if (frequency == 6)
        {
            wifiStandard = WIFI_STANDARD_80211ax;
        }
        else
        {
            NS_FATAL_ERROR("Unsupported frequency band " << frequency << " GHz for standard "
                                                         << standard);
        }
    }
    else
    {
        NS_FATAL_ERROR("Unsupported standard: " << standard);
    }


    WifiHelper wifi;
    wifi.SetStandard(wifiStandard);

    //Channel Settings
    //The primary20 index is the index of the primary 20 MHz channel within the operating channel
    std::string channelStr = "{" + std::to_string(ChannelNumber) + ", " + std::to_string(channelWidth) + ", BAND_" +
                             (frequency == 2.4 ? "2_4" : (frequency == 5 ? "5" : "6")) + "GHZ, 0}";
    Config::SetDefault("ns3::WifiPhy::ChannelSettings", StringValue(channelStr));

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (dataMode),
                                  "ControlMode", StringValue (controlMode )
                                );


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
