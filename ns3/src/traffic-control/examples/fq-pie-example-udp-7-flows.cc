/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
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
 * Authors:  Sumukha PK <sumukhapk46@gmail.com>
 *           Prajval M  <26prajval98@gmail.com>
 *           Ishaan R D <ishaanrd6@gmail.com>
 *           Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

/** Network topology
 *
 * 
 *    100mB/s, 5ms |
 * n0--------------|TCP
 *                 |
 *                 |
 *    100mB/s, 5ms |
 * n1--------------|TCP
 *                 |
 *                 |
 *    100Mb/s, 5ms |TCP                      100Mb/s, 5ms
 * n2--------------|                    |---------------n9 TCP sink
 *                 |    10Mbps, 32ms    |
 *                 n7------------------n8
 *    100Mb/s, 5ms |  QueueLimit = 100  |    100Mb/s, 5ms
 *                 |                    |
 *                 |                    |
 * n3--------------|TCP                 |---------------n10 UDP sink
 *                 |
 *                 |
 *    100mB/s, 5ms |TCP
 * n4--------------|
 *                 |
 *                 |
 *    100mB/s, 5ms |UDP
 * n5--------------|
 *                 |
 *                 |
 *    100mB/s, 5ms |UDP
 * n6--------------|
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FqPieExample");

uint32_t checkTimes;
double avgQueueDiscSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n0n7;
NodeContainer n1n7;
NodeContainer n2n7;
NodeContainer n3n7;
NodeContainer n4n7;
NodeContainer n5n7;
NodeContainer n6n7;
NodeContainer n7n8;
NodeContainer n8n9;
NodeContainer n8n10;

Ipv4InterfaceContainer i0i7;
Ipv4InterfaceContainer i1i7;
Ipv4InterfaceContainer i2i7;
Ipv4InterfaceContainer i3i7;
Ipv4InterfaceContainer i4i7;
Ipv4InterfaceContainer i5i7;
Ipv4InterfaceContainer i6i7;
Ipv4InterfaceContainer i7i8;
Ipv4InterfaceContainer i8i9;
Ipv4InterfaceContainer i8i10;

std::stringstream filePlotQueueDisc;
std::stringstream filePlotQueueDiscAvg;

void
CheckQueueDiscSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();

  avgQueueDiscSize += qSize;
  checkTimes++;

  // check queue disc size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueDiscSize, queue);

  std::ofstream fPlotQueueDisc (filePlotQueueDisc.str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueueDisc << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueueDisc.close ();

  std::ofstream fPlotQueueDiscAvg (filePlotQueueDiscAvg.str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueueDiscAvg << Simulator::Now ().GetSeconds () << " " << avgQueueDiscSize / checkTimes << std::endl;
  fPlotQueueDiscAvg.close ();
}

void
BuildAppsTest ()
{
  //TCP sink
  uint16_t port_tcp= 50000,port_udp = 50001;
  Address sinkLocalAddress_tcp (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelper_tcp ("ns3::TcpSocketFactory", sinkLocalAddress_tcp);
  ApplicationContainer sinkApp_tcp = sinkHelper_tcp.Install (n8n9.Get (1));
  sinkApp_tcp.Start (Seconds (sink_start_time));
  sinkApp_tcp.Stop (Seconds (sink_stop_time));

  //UDP sink
  Address sinkLocalAddress_udp (InetSocketAddress (Ipv4Address::GetAny (), port_udp));
  PacketSinkHelper sinkHelper_udp ("ns3::UdpSocketFactory", sinkLocalAddress_udp);
  ApplicationContainer sinkApp_udp = sinkHelper_udp.Install (n8n10.Get (1));
  sinkApp_udp.Start (Seconds (sink_start_time));
  sinkApp_udp.Stop (Seconds (sink_stop_time));

 
  //OnOffHelpers to send UDP and TCP data to respective destinations
  OnOffHelper clientHelper_tcp ("ns3::TcpSocketFactory", Address ());
  clientHelper_tcp.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper_tcp.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper_tcp.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper_tcp.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));

  OnOffHelper clientHelper_udp ("ns3::UdpSocketFactory", Address ());
  clientHelper_udp.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper_udp.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper_udp.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper_udp.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));

  ApplicationContainer clientApps_tcp,clientApps_udp;

  AddressValue remoteAddress (InetSocketAddress (i8i9.GetAddress (1), port_tcp)); //Address to the sink
  clientHelper_tcp.SetAttribute ("Remote", remoteAddress);
  //Installing TCP on nodes 0-4
  clientApps_tcp.Add (clientHelper_tcp.Install (n0n7.Get (0)));
  clientApps_tcp.Add (clientHelper_tcp.Install (n1n7.Get (0)));
  clientApps_tcp.Add (clientHelper_tcp.Install (n2n7.Get (0)));
  clientApps_tcp.Add (clientHelper_tcp.Install (n3n7.Get (0)));
  clientApps_tcp.Add (clientHelper_tcp.Install (n4n7.Get (0)));

  clientApps_tcp.Start (Seconds (client_start_time));
  clientApps_tcp.Stop (Seconds (client_stop_time));



  AddressValue remoteAddress1 (InetSocketAddress (i8i10.GetAddress (1), port_udp));
  clientHelper_udp.SetAttribute ("Remote", remoteAddress);
  //Installing UDP on nodes 5-6
  clientApps_udp.Add (clientHelper_tcp.Install (n5n7.Get (0)));
  clientApps_udp.Add (clientHelper_tcp.Install (n6n7.Get (0)));

  clientApps_udp.Start (Seconds (client_start_time));
  clientApps_udp.Stop (Seconds (client_stop_time));

}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("FqPieQueueDisc", LOG_LEVEL_INFO);

  std::string FqpieLinkDataRate = "10Mbps";
  std::string FqpieLinkDelay = "32ms";

  std::string pathOut;
  bool writeForPlot = true;
  bool writePcap = true;
  bool flowMonitor = true;

  bool printFqPieStats = true;

  global_start_time = 0.0;
  sink_start_time = global_start_time;
  client_start_time = global_start_time + 1.5;
  global_stop_time = 7.0;
  sink_stop_time = global_stop_time + 3.0;
  client_stop_time = global_stop_time - 2.0;

  // Configuration and command line parameter parsing
  // Will only save in the directory if enable opts below
  pathOut = "."; // Current directory
  CommandLine cmd;
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  c.Create (11);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  Names::Add ( "N6", c.Get (6));
  Names::Add ( "N7", c.Get (7));
  Names::Add ( "N8", c.Get (8));
  Names::Add ( "N9", c.Get (9));
  Names::Add ( "N10", c.Get (10));

  n0n7 = NodeContainer (c.Get (0), c.Get (7));
  n1n7 = NodeContainer (c.Get (1), c.Get (7));
  n2n7 = NodeContainer (c.Get (2), c.Get (7));
  n3n7 = NodeContainer (c.Get (3), c.Get (7));
  n4n7 = NodeContainer (c.Get (4), c.Get (7));
  n5n7 = NodeContainer (c.Get (5), c.Get (7));
  n6n7 = NodeContainer (c.Get (6), c.Get (7));
  n7n8 = NodeContainer (c.Get (7), c.Get (8));
  n8n9 = NodeContainer (c.Get (8), c.Get (9));
  n8n10 = NodeContainer (c.Get (8), c.Get (10));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  uint32_t meanPktSize = 1000;

  // PIE params
  NS_LOG_INFO ("Set PIE params in Fqpiequeuedisc");
  Config::SetDefault ("ns3::FqPieQueueDisc::MaxSize", StringValue ("100p"));
  Config::SetDefault ("ns3::FqPieQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
  Config::SetDefault ("ns3::FqPieQueueDisc::DequeueThreshold", UintegerValue (10000));
  Config::SetDefault ("ns3::FqPieQueueDisc::QueueDelayReference", TimeValue (Seconds (0.02)));
  Config::SetDefault ("ns3::FqPieQueueDisc::MaxBurstAllowance", TimeValue (Seconds (0.1)));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  TrafficControlHelper tchFqPie;
  tchFqPie.SetRootQueueDisc ("ns3::FqPieQueueDisc");

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  NetDeviceContainer devn0n7;
  NetDeviceContainer devn1n7;
  NetDeviceContainer devn2n7;
  NetDeviceContainer devn3n7;
  NetDeviceContainer devn4n7;
  NetDeviceContainer devn5n7;
  NetDeviceContainer devn6n7;
  NetDeviceContainer devn7n8;
  NetDeviceContainer devn8n9;
  NetDeviceContainer devn8n10;

  QueueDiscContainer queueDiscs;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn0n7 = p2p.Install (n0n7);
  tchPfifo.Install (devn0n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn1n7 = p2p.Install (n1n7);
  tchPfifo.Install (devn1n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn2n7 = p2p.Install (n2n7);
  tchPfifo.Install (devn2n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn3n7 = p2p.Install (n3n7);
  tchPfifo.Install (devn3n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn4n7 = p2p.Install (n4n7);
  tchPfifo.Install (devn4n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn5n7 = p2p.Install (n5n7);
  tchPfifo.Install (devn5n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn6n7 = p2p.Install (n6n7);
  tchPfifo.Install (devn6n7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (FqpieLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (FqpieLinkDelay));
  devn7n8 = p2p.Install (n7n8);
  // only backbone link has FQ-PIE queue disc
  queueDiscs = tchFqPie.Install (devn7n8); 

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n9 = p2p.Install (n8n9);
  tchPfifo.Install (devn8n9);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n10 = p2p.Install (n8n10);
  tchPfifo.Install (devn8n10);

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i0i7 = ipv4.Assign (devn0n7);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i1i7 = ipv4.Assign (devn1n7);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i2i7 = ipv4.Assign (devn2n7);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i3i7 = ipv4.Assign (devn3n7);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i4i7 = ipv4.Assign (devn4n7);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  i5i7 = ipv4.Assign (devn5n7);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  i6i7 = ipv4.Assign (devn6n7);

  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  i7i8 = ipv4.Assign (devn7n8);

  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  i8i9 = ipv4.Assign (devn8n9);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  i8i10 = ipv4.Assign (devn8n10);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/fqpie";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  Ptr<FlowMonitor> flowmon;
  if (flowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  if (writeForPlot)
    {
      filePlotQueueDisc << pathOut << "/" << "fq-pie-queue-disc.plotme";
      filePlotQueueDiscAvg << pathOut << "/" << "fq-pie-queue-disc_avg.plotme";

      remove (filePlotQueueDisc.str ().c_str ());
      remove (filePlotQueueDiscAvg.str ().c_str ());
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
      Simulator::ScheduleNow (&CheckQueueDiscSize, queue);
    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();

  if (st.GetNDroppedPackets (FqPieQueueDisc::FORCED_DROP) != 0)
    {
      std::cout << "There should be no drops due to queue full." << std::endl;
      exit (1);
    }

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/fqpie.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

  if (printFqPieStats)
    {
      std::cout << "***FQ PIE stats from Node 2 queue ***" << std::endl;
      std::cout << "\t " << st.GetNDroppedPackets (FqPieQueueDisc::UNFORCED_DROP)
                << " drops due to prob mark" << std::endl;
      std::cout << "\t " << st.GetNDroppedPackets (FqPieQueueDisc::FORCED_DROP)
                << " drops due to queue limits" << std::endl;
    }

  Simulator::Destroy ();

  return 0;
}
