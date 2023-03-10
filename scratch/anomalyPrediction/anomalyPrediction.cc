//CXX="clang++" ./ns3 configure --cxx-standard 17
//time ./ns3 run "anomalyPrediction --pcap=false --printRoutes=true --numOfUAVs=3 --time=3 --step=1 --EnableMonitor=true --hostname=test --dataRate=1Mbps"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include "ns3/timestamp.h"

#include "ns3/sha256.h"
#include "ns3/database.h"
#include "ns3/nodeInfo.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
//#include "ns3/wifi-mac-queue-item.h"
//#include "ns3/wifi-mac-queue.h"
//#include "ns3/wifi-mac-header.h"
//#include "ns3/wifi-mac.h"
//#include "ns3/wifi-mac-queue-item.h"

#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-mac-queue.h"

#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

using namespace ns3;

//global vars
//record all nodes info
nodeInfo nodesRecords;
//database
database dataOutput;
//run ID
std::string runID;
// Host Name
char hostname[1024];
//Create propagation loss matrix
Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel>();

// UAVs network
//NodeContainer UAVs;
NodeContainer UAVs;
NetDeviceContainer UAVdevices;
Ipv4InterfaceContainer UAVinterfaces;

//Ground Station
NodeContainer groundStation;
NetDeviceContainer groundStationDevice;
Ipv4InterfaceContainer groundStationInterface;

WifiMacHelper wifiMac;

YansWifiPhyHelper wifiPhy;

Ptr<YansWifiChannel> wifiChannel = CreateObject<YansWifiChannel>();

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
/**
 * Parse context strings of the form "/NodeList/x/DeviceList/x/..." to extract the NodeId integer
 *
 * \param context The context to parse.
 * \return the NodeId
 */
uint32_t contextToNodeId(std::string context) {
    std::string sub = context.substr(10);
    uint32_t pos = sub.find("/Device");

    uint32_t nodeId = std::stoi(sub.substr(0, pos));

    return nodeId;
}

struct packetInfo {
    uint32_t size;
    uint64_t UID;
};

packetInfo setPacketinfo (Ptr <const Packet> packet){
    packetInfo packetInfo;
    packetInfo.size = packet->GetSize();
    packetInfo.UID = packet->GetUid();

    return packetInfo;
}

static void PrintPacketInfo (Ptr <const Packet> packet){
    std::cout << "Recv. Packet of size " << packet->GetSize() << std::endl;
    std::cout << "UID: " << packet->GetUid() << std::endl;
    return;
}

struct wifiVector {
    uint8_t ness;
    uint8_t nss;
    uint8_t powerLevel;
};

wifiVector setWifiVector(ns3::WifiTxVector txVector) {
    wifiVector vector;

    //the number of Ness
    vector.ness = txVector.GetNess();
    //the number of Nss
    vector.nss = txVector.GetNss();
    //the transmission power level
    vector.powerLevel = txVector.GetTxPowerLevel();

    return vector;
}

static void RxPacketInfo(std::string context, Ptr <const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector,
                         MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId) {
    //std::cout << "RX Packet Info" << std::endl;
    std::string type = "RxPacketInfo";

    timestamp timestamp;

    //std::cout << "time string: " << timestamp.getTimeString() << std::endl;
    //std::cout << "context: " << context << std::endl;
    //std::cout << "node Name: " << nodesRecords.getName(contextToNodeId(context)) << std::endl;
    //std::cout << "node ID: " << contextToNodeId(context) << std::endl;
    //std::cout << "Signal= " << signalNoise.signal << " Noise= " << signalNoise.noise << std::endl;
    //std::cout << "channelFreqMhz: " << channelFreqMhz << std::endl;

    wifiVector vector;
    vector = setWifiVector(txVector);

    //std::cout << "ness: " << vector.ness << std::endl;
    //std::cout << "nss: " << vector.nss << std::endl;
    //std::cout << "powerLevel: " << vector.powerLevel << std::endl;

    packetInfo packetResult = setPacketinfo (packet);
    //std::cout << "Packet Size: " << packetResult.size  << std::endl;
    //std::cout << "Packet UID: " <<  packetResult.UID << std::endl;


    bool succ = dataOutput.RxPacketInfoRecord(runID, hostname, type, timestamp.getTimeString(), context, nodesRecords.getName(contextToNodeId(context)),
                                     contextToNodeId(context), signalNoise.signal, signalNoise.noise, channelFreqMhz,
                                     vector.ness, vector.nss, vector.powerLevel, packetResult.size, packetResult.UID);
    if (succ == false){
        std::cout << "RxPacketInfo Record fail!!!" << std::endl;
    }

    return;
}

static void TxPacketInfo(std::string context, Ptr <const Packet> packet, uint16_t channelFreqMhz,
                         WifiTxVector txVector, MpduInfo aMpdu, uint16_t staId){

    //std::cout << "TX Packet Info" << std::endl;
    std::string type = "TxPacketInfo";

    timestamp timestamp;

    //std::cout << "time string: " << timestamp.getTimeString() << std::endl;

    //std::cout << context << std::endl;
    //std::cout << "node Name: " << nodesRecords.getName(contextToNodeId(context)) << std::endl;
    //std::cout << "node ID: " << contextToNodeId(context) << std::endl;
    //std::cout << "channelFreqMhz: " << channelFreqMhz << std::endl;

    wifiVector vector;
    vector = setWifiVector(txVector);

    //std::cout << "ness: " << vector.ness << std::endl;
    //std::cout << "nss: " << vector.nss << std::endl;
    //std::cout << "powerLevel: " << vector.powerLevel << std::endl;

    packetInfo packetResult = setPacketinfo (packet);
    //std::cout << "Packet Size: " << packetResult.size  << std::endl;
    //std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    //database
    bool succ = dataOutput.TxPacketInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              channelFreqMhz, vector.ness, vector.nss, vector.powerLevel,
                                              packetResult.size, packetResult.UID);
    if (succ == false){
        std::cout << "TxPacketInfo Record fail!!!" << std::endl;
    }

    return;
}

static void DequeueTrace(std::string context, Ptr<const WifiMpdu> item){
    std::cout << "context: " << context << std::endl;
    std::cout << "time: " << Simulator::Now() << std::endl;

    packetInfo packetResult = setPacketinfo (item->GetPacket());
    std::cout << "Packet Size: " << packetResult.size  << std::endl;
    std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    return;
}

static void DropBeforeEnqueueTrace(std::string context, Ptr<const WifiMpdu> item){
    std::cout << "context: " << context << std::endl;
    std::cout << "time: " << Simulator::Now() << std::endl;

    packetInfo packetResult = setPacketinfo (item->GetPacket());
    std::cout << "Packet Size: " << packetResult.size  << std::endl;
    std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    return;
}

static void DropAfterDequeueTrace(std::string context, Ptr<const WifiMpdu> item){
    std::cout << "context: " << context << std::endl;
    std::cout << "time: " << Simulator::Now() << std::endl;

    packetInfo packetResult = setPacketinfo (item->GetPacket());
    std::cout << "Packet Size: " << packetResult.size  << std::endl;
    std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    return;
}

static void EnqueueTrace(std::string context, Ptr<const WifiMpdu> item){
    std::cout << "context: " << context << std::endl;
    std::cout << "time: " << Simulator::Now() << std::endl;

    packetInfo packetResult = setPacketinfo (item->GetPacket());
    std::cout << "Packet Size: " << packetResult.size  << std::endl;
    std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    return;
}

static void QueueDropTrace(std::string context, Ptr<const WifiMpdu> item){
    //std::cout << "Queue Drop Info" << std::endl;
    std::string type = "QueueDropInfo";

    timestamp timestamp;

    std::cout << "time string: " << timestamp.getTimeString() << std::endl;

    //std::cout << "context: " << context << std::endl;

    //std::cout << "node Name: " << nodesRecords.getName(contextToNodeId(context)) << std::endl;
    //std::cout << "node ID: " << contextToNodeId(context) << std::endl;

    packetInfo packetResult = setPacketinfo (item->GetPacket());
    //std::cout << "Packet Size: " << packetResult.size  << std::endl;
    //std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    //database
    bool succ = dataOutput.QueueDropInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              packetResult.size, packetResult.UID);

    if (succ == false){
        std::cout << "Queue Drop Info Record fail!!!" << std::endl;
    }

    return;
}

static void PhyTxDropInfo(std::string context, Ptr <const Packet> packet){
    //std::cout << "TX Drop Info" << std::endl;
    std::string type = " PhyTxDropInfo";

    timestamp timestamp;

    //std::cout << "time string: " << timestamp.getTimeString() << std::endl;

    //std::cout << "context: " << context << std::endl;
    //std::cout << "node Name: " << nodesRecords.getName(contextToNodeId(context)) << std::endl;
    //std::cout << "node ID: " << contextToNodeId(context) << std::endl;

    packetInfo packetResult = setPacketinfo (packet);
    //std::cout << "Packet Size: " << packetResult.size  << std::endl;
    //std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    //database
    bool succ = dataOutput.PhyTxDropInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              packetResult.size, packetResult.UID);
    if (succ == false){
        std::cout << " PhyTxDropInfo Record fail!!!" << std::endl;
    }

    return;
}

std::string WifiPhyRxfailureReasonEnumToString(WifiPhyRxfailureReason reason)
{
    switch (reason) {
        case UNKNOWN: return "UNKNOWN";
        case UNSUPPORTED_SETTINGS: return "UNSUPPORTED_SETTINGS";
        case CHANNEL_SWITCHING: return "CHANNEL_SWITCHING";
        case RXING: return "RXING";
        case TXING: return "TXING";
        case SLEEPING: return "SLEEPING";
        case BUSY_DECODING_PREAMBLE: return "BUSY_DECODING_PREAMBLE";
        case PREAMBLE_DETECT_FAILURE: return "PREAMBLE_DETECT_FAILURE";
        case RECEPTION_ABORTED_BY_TX: return "RECEPTION_ABORTED_BY_TX";
        case L_SIG_FAILURE: return "L_SIG_FAILURE";
        case HT_SIG_FAILURE: return "HT_SIG_FAILURE";
        case SIG_A_FAILURE: return "SIG_A_FAILURE";
        case SIG_B_FAILURE: return "SIG_B_FAILURE";
        case PREAMBLE_DETECTION_PACKET_SWITCH: return "PREAMBLE_DETECTION_PACKET_SWITCH";
        case FRAME_CAPTURE_PACKET_SWITCH: return "FRAME_CAPTURE_PACKET_SWITCH";
        case OBSS_PD_CCA_RESET: return "OBSS_PD_CCA_RESET";
        case HE_TB_PPDU_TOO_LATE: return "HE_TB_PPDU_TOO_LATE";
        case FILTERED: return "FILTERED";
        default: return "UNKNOWN";
    }
}

static void PhyRxDropInfo(std::string context, Ptr <const Packet> packet, WifiPhyRxfailureReason reason){
    //std::cout << "RX Drop Info" << std::endl;
    std::string type = " PhyRxDropInfo";

    timestamp timestamp;

    //std::cout << "time string: " << timestamp.getTimeString() << std::endl;

    //std::cout << "context: " << context << std::endl;
    //std::cout << "node Name: " << nodesRecords.getName(contextToNodeId(context)) << std::endl;
    //std::cout << "node ID: " << contextToNodeId(context) << std::endl;

    packetInfo packetResult = setPacketinfo (packet);
    //std::cout << "Packet Size: " << packetResult.size  << std::endl;
    //std::cout << "Packet UID: " <<  packetResult.UID << std::endl;

    //std::cout << "RX Drop Reason: " <<  WifiPhyRxfailureReasonEnumToString(reason) << std::endl;

    //database
    bool succ = dataOutput.PhyRxDropInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              packetResult.size, packetResult.UID, WifiPhyRxfailureReasonEnumToString(reason));
    if (succ == false){
        std::cout << "PhyRxDropInfo fail!!!" << std::endl;
    }



    return;
}

int main (int argc, char **argv){

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

    runID = generateRunID();

    /*
    //Print Run Record out
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
    */

    bool succ = dataOutput.runRecord(runID, hostname, dataMode, controlMode, numOfUAVs, totalTime, payloadSize, dataRate,
                                     ChannelNumber, frequency, standard, channelWidth, guardIntervalNs);
    if (succ == true){
        std::cout << "Run Record Success!!!" << std::endl;
    }else{
        std::cout << "Run Record fail!!!" << std::endl;
    }

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

    //connect database
    pqxx::connection * conn =  dataOutput.getConnection();

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

    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback (&RxPacketInfo));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferTx", MakeCallback (&TxPacketInfo));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback (&PhyTxDropInfo));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback (&PhyRxDropInfo));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/*/Queue/Dequeue", MakeCallback(&DequeueTrace));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/*/Queue/Enqueue", MakeCallback(&EnqueueTrace));
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/*/Queue/Drop", MakeCallback(&QueueDropTrace));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/*/Queue/DropBeforeEnqueue", MakeCallback(&DropBeforeEnqueueTrace));
    //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/*/Queue/DropAfterDequeue", MakeCallback(&DropAfterDequeueTrace));

    return;
}

static void setLoss() {
    std::cout << "Function called at " << Simulator::Now().GetSeconds() << " seconds" << std::endl;
/*
    //ns-3 examples/wireless/power-adaptation-distance.cc
    Ptr<NetDevice> device = wifiNetDevices.Get(i);
    Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
    Ptr<YansWifiPhy> phy = DynamicCast<YansWifiPhy>(wifiDevice->GetPhy());
    phy->SetTxPowerStart(20);
    phy->SetTxPowerEnd(20);

 */
    // set symmetric loss 0 <-> 1 to 200 dB (no link)
    lossModel->SetLoss(UAVs.Get(0)->GetObject<MobilityModel>(),
                       UAVs.Get(1)->GetObject<MobilityModel>(),
                       200, true);

    wifiChannel->SetPropagationLossModel(lossModel);

    wifiPhy.SetChannel(wifiChannel);

    return;
}

void anomalyPrediction::Run (){

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

    Simulator::Schedule(Seconds(2.0), &setLoss);

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

        nodesRecords.insertName(UAVs.Get(i)->GetId(), os.str());
        std::cout << "name: " << os.str() << " ID: " << UAVs.Get(i)->GetId() << std::endl;
    }

    std::cout << "Creating " << "Ground Station" << std::endl;
    groundStation.Create (1);

    //Name Ground Station
    std::string groundStationName = "Ground-Station";
    Names::Add (groundStationName, groundStation.Get(0));

    nodesRecords.insertName(groundStation.Get(0)->GetId(), groundStationName);
    std::cout << "name: " <<  groundStationName << " ID: " << groundStation.Get(0)->GetId() << std::endl;

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
    wifiMac.SetType ("ns3::AdhocWifiMac");


    lossModel->SetDefaultLoss(0); // set default loss to 200 dB (no link)

    wifiChannel->SetPropagationLossModel(lossModel);
    wifiChannel->SetPropagationDelayModel(CreateObject<ConstantSpeedPropagationDelayModel>());

    wifiPhy.SetChannel(wifiChannel);
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
