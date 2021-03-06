#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/switched-ethernet-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/pcap-file.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SwitchedEthernetPacketSocketExample");

std::ofstream g_os;

static void
SinkRx (std::string path, Ptr<const Packet> p, const Address &address)
{
//  NS_LOG_UNCOND("Accessing receiver file");
  g_os << p->GetSize () << std::endl;
}

int
main (int argc, char *argv[])
{
#if 0
  LogComponentEnable ("SwitchedEthernetPacketSocketExample", LOG_LEVEL_INFO);
#endif

  Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(1500));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1434));
  CommandLine cmd;
  cmd.Parse (argc, argv);

  
  NS_LOG_INFO ("Create nodes.");

  //Create two nodes - both can send and receive simultaneously
  NodeContainer nodes;
  nodes.Create (2);

  InternetStackHelper internet;
  internet.Install (nodes);

  // create the two full-duplex channels that are used to send packets to node 0 from nodes 1 and 2
  NS_LOG_INFO ("Create channels.");
  Ptr<SwitchedEthernetChannel> channel = CreateObjectWithAttributes<SwitchedEthernetChannel> (
      "DataRate", DataRateValue (DataRate (1000000000)),
      "Delay", TimeValue (NanoSeconds (10)));
  Time startTime = Seconds (0);
  Time stopTime = Seconds (70);

  // use a helper function to connect our nodes to the full-duplex channel.
  NS_LOG_INFO ("Build Topology.");
  SwitchedEthernetHelper switchedEth;
  switchedEth.SetDeviceAttribute("EncapsulationMode", StringValue ("Llc"));
  switchedEth.SetDeviceAttribute("GoToSleepTime", TimeValue(MicroSeconds(182.0)));
  switchedEth.SetDeviceAttribute("QueueTimer", TimeValue(MicroSeconds(1000)));
  switchedEth.SetDeviceAttribute("MaxQueueLength", UintegerValue(1000));
  switchedEth.SetDeviceAttribute("DynamicCoalescing", BooleanValue (true));
  switchedEth.SetDeviceAttribute("Additive", BooleanValue (true));
  switchedEth.SetDeviceAttribute("Multiplicative", BooleanValue (true));
  switchedEth.SetDeviceAttribute("TimerStepAdditive", UintegerValue (1000));   //Time is given in microseconds
  switchedEth.SetDeviceAttribute("TimerStepMultiplicative", UintegerValue (10));   //The step is given in percentage eg. 5 10 etc
  switchedEth.SetDeviceAttribute("QueueStepAdditive", UintegerValue (0));   //Queue step is given in number of packets
  switchedEth.SetDeviceAttribute("QueueStepMultiplicative", UintegerValue (0));   //The step is given in percentage eg. 5 10 etc
  switchedEth.SetDeviceAttribute("MaxTimeout", TimeValue (MicroSeconds(200000)));
  switchedEth.SetDeviceAttribute("MaxAverageDelay", DoubleValue (1.0)); //we set up the delay like 1.0 for 1ms, 0.5 for 500microseconds etc.

  NetDeviceContainer devs = switchedEth.Install (nodes, channel);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses");
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs);
  NS_LOG_INFO ("Create Applications.");

  // Create a packet sink on the star "hub" to receive these packets
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (nodes.Get(1));
  sinkApp.Start (startTime);
  sinkApp.Stop (stopTime);

  PacketSinkHelper sinkHelper2 ("ns3::UdpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (nodes.Get(0));
  sinkApp2.Start (startTime);
  sinkApp2.Stop (stopTime);

//FromDouble
  TraceAppHelper client = TraceAppHelper ("ns3::UdpSocketFactory", i.GetAddress (1));
  client.SetAttribute ("TraceFile", StringValue("/home/angelos/ns3-AdaptiveTimer/current_trace2_0"));
  ApplicationContainer clientApps;
  AddressValue remoteAddress(InetSocketAddress (i.GetAddress (1), port));
  client.SetAttribute ("Remote", remoteAddress);
  clientApps = client.Install (nodes.Get(0));
  clientApps.Start (startTime);
  clientApps.Stop (stopTime);

  TraceAppHelper client2 = TraceAppHelper ("ns3::UdpSocketFactory", i.GetAddress (0));
  client2.SetAttribute ("TraceFile", StringValue("/home/angelos/ns3-AdaptiveTimer/current_trace2_1"));
  client2.SetAttribute ("MaxPacketSize", UintegerValue(1460));
  ApplicationContainer clientApps2;
  AddressValue remoteAddress2(InetSocketAddress (i.GetAddress (0), port));
  client2.SetAttribute ("Remote", remoteAddress2);
  clientApps2 = client2.Install (nodes.Get(1));
  clientApps2.Start (startTime);
  clientApps2.Stop (stopTime);

  // While the below trace sink is hooked to all nodes (the wildcard "*")
  // only one packet sink (on node 0) is actually added above, so
  // only the receive events on node 0 will be traced
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                   MakeCallback (&SinkRx));

  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the switched-ethernet-MyScenario2.tr file
  NS_LOG_INFO ("Configure Tracing.");

  // For more detailed information on what is going on, enable these two log functions
  // in debug mode. Note that this logging will generate a large amount of output.
  //LogComponentEnable ("SwitchedEthernetNetDevice", LOG_LEVEL_ALL);
  //LogComponentEnable ("SwitchedEthernetChannel", LOG_LEVEL_ALL);

  AsciiTraceHelper ascii;
//  switchedEth.EnableAsciiAll (ascii.CreateFileStream ("switched-ethernet-MyScenario1.tr"));
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("scenarioFile.tr");
  stream->GetStream()->precision(10);
  switchedEth.EnableAsciiAll(stream);
//  switchedEth.EnablePcapAll ("scenarioFile", false);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(stopTime + Seconds(1.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  g_os.close ();
  return 0;
}
