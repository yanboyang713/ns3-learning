/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This is an example script for AODV manet routing protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */
//./ns3 run "aodvExample --pcap=false --printRoutes=true --numOfUAVs=3 --time=5 --step=150"
//
#include <iostream>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-net-device.h"

#include "ns3/config-store-module.h"
#include "ns3/stats-module.h"

#include <fstream>


using namespace ns3;

class AodvExample{
  public:
  AodvExample ();
  /**
   * \brief Configure script parameters
   * \param argc is the command line argument count
   * \param argv is the command line arguments
   * \return true on successful configuration
  */
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /**
   * Report results
   * \param os the output stream
   */
  void Report (std::ostream & os);

private:

  // parameters
  /// Number of UAVs
  uint32_t numOfUAVs;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  //On-Off
  ////Transport layer payload size in bytes.
  uint32_t payloadSize = 1472;
  //Application layer datarate.
  std::string dataRate = "100Mbps";

  // UAVs network
  NodeContainer UAVs;
  NetDeviceContainer UAVdevices;
  Ipv4InterfaceContainer UAVinterfaces;

  //Ground Station
  NodeContainer groundStation;
  NetDeviceContainer groundStationDevice;
  Ipv4InterfaceContainer groundStationInterface;

  //Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
  //uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */

  bool enableFlowMonitor = true;
  uint16_t port = 9;   // Discard port (RFC 863)
  uint32_t bytesTotal;      //!< Total received bytes.
  uint32_t packetsReceived; //!< Total received packets.
                            //
private:
  /// Create the nodes
  void CreateNodes ();
  /// Create the devices
  void CreateDevices ();
  /// Create the network
  void InstallInternetStack ();
  /// Create the simulation applications
  void InstallApplications ();
  Ptr<Socket> SetupPacketReceive (Ipv4Address, Ptr<Node>);
  void ReceivePacket (Ptr<Socket> socket);
};

Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */

static void EnqueueTrace (std::string context, Ptr<const Packet> pkt){
  std::cout << context << "\tTime: " << Simulator::Now() << " packet of Size: " << pkt->GetSize() << " enqueued!" << std::endl;

}

static void RxPacketInfo(std::string context, Ptr <const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector,
                         MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId) {
  std::cout << context << std::endl;
  std::cout << "Recv. Packet of size " << packet->GetSize() << " Signal= " << signalNoise.signal << " Noise= " << signalNoise.noise << std::endl;
}

static void CalculateThroughput (ApplicationContainer &sinkApps)
//void CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         // Return the simulator's virtual time.
  //std::cout << now.GetSeconds () << std::endl;


  sink = StaticCast<PacketSink> (sinkApps.Get (0));
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     // Convert Application RX Packets to MBits.
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();

  //Simulator::Schedule (MilliSeconds (100), AodvExample::CalculateThroughput());
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput, sinkApps);

  return;
}
int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  numOfUAVs (3),
  step (50),
  totalTime (100),
  pcap (true),
  bytesTotal (0),
  packetsReceived (0),
  printRoutes (true)
{
}

Ptr<Socket> AodvExample::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node){
  std::cout << "SetupPacketReceive Start" << std::endl;

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&AodvExample::ReceivePacket, this));

  std::cout << "SetupPacketReceive End" << std::endl;
  return sink;
}

void AodvExample::ReceivePacket (Ptr<Socket> socket){
  Ptr<Packet> packet;
  Address senderAddress;
  std::cout << "bytesTotal: " << bytesTotal << std::endl;
  while ((packet = socket->RecvFrom (senderAddress))){
    bytesTotal += packet->GetSize ();
    std::cout << "bytesTotal: " << bytesTotal << std::endl;
    packetsReceived += 1;
    //NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
  }

  return;
}

bool AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
  bool tracing = true;
  SeedManager::SetSeed (12345);
  CommandLine cmd (__FILE__);

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("numOfUAVs", "Number of UAVs.", numOfUAVs);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data ate", dataRate);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.AddValue ("tracing", "Enable pcap tracing",tracing);
  cmd.Parse (argc, argv);
  return true;
}

void AodvExample::Run ()
{
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  /*
  // Use FileHelper to write out the packet byte count over time
  FileHelper fileHelper;

  // Configure the file to be written, and the formatting of output data.
  fileHelper.ConfigureFile("yby-test-packet-byte-count", FileAggregator::FORMATTED);

  // Set the labels for this formatted output file.
  fileHelper.Set2dFormat("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

  // Specify the probe type, trace source path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to write.
  std::string probeType = "ns3::Ipv4PacketProbe";
  std::string tracePath = "/NodeList/2/$ns3::Ipv4L3Protocol/Tx";
  fileHelper.WriteProbe(probeType, tracePath, "OutputBytes");

   */
  Simulator::Stop (Seconds (totalTime));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  

  /*
  // Output config store to txt format
  Config::SetDefault("ns3::ConfigStore::Filename", StringValue("output-attributes.txt"));
  Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("RawText"));
  Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults();
  outputConfig2.ConfigureAttributes();
   */


  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback (&RxPacketInfo));

  Simulator::Run ();

  //Config::Connect ("/NodeList/*/DeviceList/*/TxQueue/Enqueue", MakeCallback (&EnqueueTrace));

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

  Simulator::Destroy ();
}

void AodvExample::Report (std::ostream &)
{ 
}

void AodvExample::CreateNodes ()
{
  //Creating UAVs and Ground Station
  std::cout << "Creating " << (unsigned)numOfUAVs << " UAVs " << std::endl;
  UAVs.Create (numOfUAVs);
  std::cout << "Creating " << "Ground Station" << std::endl;
  groundStation.Create (1);
  // Name UAVs
  for (uint32_t i = 0; i < numOfUAVs; ++i)
    {
      std::ostringstream os;
      os << "UAV-" << i;
      Names::Add (os.str (), UAVs.Get (i));
    }
  //Name Ground Station
  std::string groundStationName = "Ground-Station";
  Names::Add (groundStationName, groundStation.Get (0));

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

void AodvExample::CreateDevices ()
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  //wifiMac.SetType ("ns3::ApWifiMac");

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


/*
  AsciiTraceHelper ascii;
  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifiPhy.tr"));
  UAVs.EnableAsciiAll(ascii.CreateFileStream("UAVs.tr"));
  groundStation.EnableAsciiAll(ascii.CreateFileStream("groundStation.tr"));
  
  if (tracing == true)
    {
       wifiPhy.EnablePcapAll("wifiPhyCap");
       UAVs.EnablePcapAll("UAVsCap");
       groundStation.EnablePcapAll("groundStationCap");
    
    }
    
    
*/

  return;
}

void AodvExample::InstallInternetStack () {
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
}

void AodvExample::InstallApplications ()
{
  /*
  V4PingHelper ping (UAVinterfaces.GetAddress (2));
  ping.SetAttribute ("Verbose", BooleanValue (true));

  ApplicationContainer p = ping.Install (UAVs.Get (1));
  p.Start (Seconds (0));
  p.Stop (Seconds (totalTime) - Seconds (0.001));
   */

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

  /*
  WifiPhy:
  .AddTraceSource("PhyTxDrop",
                            "Trace source indicating a packet "
                            "has been dropped by the device during transmission",
                            MakeTraceSourceAccessor(&WifiPhy::m_phyTxDropTrace),
                            "ns3::Packet::TracedCallback")


    .AddTraceSource("PhyRxDrop",
                            "Trace source indicating a packet "
                            "has been dropped by the device during reception",
                            MakeTraceSourceAccessor(&WifiPhy::m_phyRxDropTrace),
                            "ns3::Packet::TracedCallback")

    .AddTraceSource("MonitorSnifferRx",
                            "Trace source simulating a wifi device in monitor mode "
                            "sniffing all received frames",
                            MakeTraceSourceAccessor(&WifiPhy::m_phyMonitorSniffRxTrace),
                            "ns3::WifiPhy::MonitorSnifferRxTracedCallback")
    .AddTraceSource("MonitorSnifferTx",
                            "Trace source simulating the capability of a wifi device "
                            "in monitor mode to sniff all frames being transmitted",
                            MakeTraceSourceAccessor(&WifiPhy::m_phyMonitorSniffTxTrace),
                            "ns3::WifiPhy::MonitorSnifferTxTracedCallback");
   */

  //Simulator::Schedule (Seconds (1.1), &CalculateThroughput, sinkApps);
  //
  /*
  std::cout << UAVs.GetN() << std::endl;
  
  for (uint32_t i=0 ; i< UAVs.GetN() ; i++){
    Ptr<Node> node = UAVs.Get(i);
    for (uint32_t j=0 ; j<node->GetNDevices() ; j++){
      Ptr<NetDevice> device = node->GetDevice(j);

      std::cout << "GetInstanceTypeId: " << device->GetInstanceTypeId() << std::endl;

      if ( device->GetInstanceTypeId() == WifiNetDevice::GetTypeId() ){
        Ptr<WifiNetDevice> wifi_dev = DynamicCast<WifiNetDevice>(device);
        Ptr <WifiMac> mac = wifi_dev->GetMac ();


        std::cout << "GetAttribute: " << mac->GetAttribute() << std::endl;
        Ptr<RegularWifiMac> reg_mac = DynamicCast <RegularWifiMac> (mac);
        if (reg_mac){
          PointerValue val;
          reg_mac->GetAttribute ("Txop", val); //val is passed by reference
          Ptr <Txop> txop = DynamicCast <Txop> (val.GetObject ());
          Ptr <WifiMacQueue> txop_queue = txop->GetWifiMacQueue ();
          //txop_queue->SetMaxDelay ( MilliSeconds (250));
        }
      }
    }
  }
  */
  
 // set packet sink

  /*
  ApplicationContainer sinkApps;
  Ptr<Socket> sink = SetupPacketReceive (UAVinterfaces.GetAddress (1), UAVs.Get (1));
  sinkApps.Add(sink);
  sinkApps.Start (Seconds (0.1));
  sinkApps.Stop (Seconds (totalTime) - Seconds (0.001));
   */
 /*
 ApplicationContainer sinkApps;
 PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny (),port));
 sinkApps.Add(sink.Install(groundStation.Get(0)));
 for (uint32_t k = 0; k < numOfUAVs; k++){
    sinkApps.Add(sink.Install(UAVs.Get(k)));
 }
 sinkApps.Start (Seconds (0.1));
 sinkApps.Stop (Seconds (totalTime) - Seconds (0.001));
  */

  /*
  // Create a packet sink to receive these packets
  //Install TCP Receiver on the access point
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApp = sinkHelper.Install (groundStation.Get(0));
  sink = StaticCast<PacketSink> (sinkApp.Get (0));
  sinkApp.Start (Seconds (0.1));
  sinkApp.Stop (Seconds (totalTime) - Seconds (0.001));
  */

  //Simulator::Schedule (Seconds (1.1), &CalculateThroughput, sinkApps);
  return;
}

