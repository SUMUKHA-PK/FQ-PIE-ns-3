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
 *    100mB/s, 5ms |                    | 100mB/s, 5ms
 * n0--------------|TCP                 |---------------n00 TCP sink
 *                 |                    |
 *                 |                    |
 *    100mB/s, 5ms |                    | 100Mb/s, 5ms
 * n1--------------|TCP                 |---------------n10 TCP sink
 *                 |                    |
 *                 |                    |
 *    100Mb/s, 5ms |TCP                 | 100Mb/s, 5ms
 * n2--------------|                    |---------------n20 TCP sink
 *                 |    10Mbps, 32ms    |
 *                 n7------------------n8
 *    100Mb/s, 5ms |  QueueLimit = 100  |    
 *                 |                    |
 *                 |                    | 100Mb/s, 5ms
 * n3--------------|TCP                 |---------------n30 TCP sink
 *                 |                    |
 *                 |                    |
 *    100mB/s, 5ms |TCP                 | 100Mb/s, 5ms
 * n4--------------|                    |--------------- n40 TCP sink
 *                 |                    |
 *                 |                    | 
 *    100mB/s, 5ms |UDP                 | 100Mb/s, 5ms
 * n5--------------|                    |--------------- n50 UDP sink
 *                 |                    |
 *                 |                    |
 *    100mB/s, 5ms |UDP                 | 100Mb/s, 5ms
 * n6--------------|                    |--------------- n60 UDP sink
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FqCoDelExample");

uint32_t checkTimes;
double avgQueueDiscSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

// The Sources
NodeContainer n0n7;
NodeContainer n1n7;
NodeContainer n2n7;
NodeContainer n3n7;
NodeContainer n4n7;
NodeContainer n5n7;
NodeContainer n6n7;

// The bottleneck
NodeContainer n7n8;

// The sinks
NodeContainer n8n00;
NodeContainer n8n10;
NodeContainer n8n20;
NodeContainer n8n30;
NodeContainer n8n40;
NodeContainer n8n50;
NodeContainer n8n60;


// The sources
Ipv4InterfaceContainer i0i7;
Ipv4InterfaceContainer i1i7;
Ipv4InterfaceContainer i2i7;
Ipv4InterfaceContainer i3i7;
Ipv4InterfaceContainer i4i7;
Ipv4InterfaceContainer i5i7;
Ipv4InterfaceContainer i6i7;

// The bottlenecks
Ipv4InterfaceContainer i7i8;

// The sinks
Ipv4InterfaceContainer i8i00;
Ipv4InterfaceContainer i8i10;
Ipv4InterfaceContainer i8i20;
Ipv4InterfaceContainer i8i30;
Ipv4InterfaceContainer i8i40;
Ipv4InterfaceContainer i8i50;
Ipv4InterfaceContainer i8i60;

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
  //TCP sinks


  uint16_t port_tcp= 50000,port_udp=50001;

  Address sinkLocalAddress_tcp0 (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelperTCP0 ("ns3::TcpSocketFactory", sinkLocalAddress_tcp0);
  ApplicationContainer sinkAppTCP0 = sinkHelperTCP0.Install (n8n00.Get (1));
  sinkAppTCP0.Start (Seconds (sink_start_time));
  sinkAppTCP0.Stop (Seconds (sink_stop_time));

  Address sinkLocalAddress_tcp1 (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelperTCP1 ("ns3::TcpSocketFactory", sinkLocalAddress_tcp1);
  ApplicationContainer sinkAppTCP1 = sinkHelperTCP1.Install (n8n10.Get (1));
  sinkAppTCP1.Start (Seconds (sink_start_time));
  sinkAppTCP1.Stop (Seconds (sink_stop_time));
  
  Address sinkLocalAddress_tcp2 (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelperTCP2 ("ns3::TcpSocketFactory", sinkLocalAddress_tcp2);
  ApplicationContainer sinkAppTCP2 = sinkHelperTCP2.Install (n8n20.Get (1));
  sinkAppTCP2.Start (Seconds (sink_start_time));
  sinkAppTCP2.Stop (Seconds (sink_stop_time));

  Address sinkLocalAddress_tcp3 (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelperTCP3 ("ns3::TcpSocketFactory", sinkLocalAddress_tcp3);
  ApplicationContainer sinkAppTCP3 = sinkHelperTCP3.Install (n8n30.Get (1));
  sinkAppTCP3.Start (Seconds (sink_start_time));
  sinkAppTCP3.Stop (Seconds (sink_stop_time));

  Address sinkLocalAddress_tcp4 (InetSocketAddress (Ipv4Address::GetAny (), port_tcp));
  PacketSinkHelper sinkHelperTCP4 ("ns3::TcpSocketFactory", sinkLocalAddress_tcp4);
  ApplicationContainer sinkAppTCP4 = sinkHelperTCP4.Install (n8n40.Get (1));
  sinkAppTCP4.Start (Seconds (sink_start_time));
  sinkAppTCP4.Stop (Seconds (sink_stop_time));



  //UDP sinks

  Address sinkLocalAddress_udp5 (InetSocketAddress (Ipv4Address::GetAny (), port_udp));
  PacketSinkHelper sinkHelperUDP5 ("ns3::UdpSocketFactory", sinkLocalAddress_udp5);
  ApplicationContainer sinkAppUDP5 = sinkHelperUDP5.Install (n8n50.Get (1));
  sinkAppUDP5.Start (Seconds (sink_start_time));
  sinkAppUDP5.Stop (Seconds (sink_stop_time));

  Address sinkLocalAddress_udp6 (InetSocketAddress (Ipv4Address::GetAny (), port_udp));
  PacketSinkHelper sinkHelperUDP6 ("ns3::UdpSocketFactory", sinkLocalAddress_udp6);
  ApplicationContainer sinkAppUDP6 = sinkHelperUDP6.Install (n8n60.Get (1));
  sinkAppUDP6.Start (Seconds (sink_start_time));
  sinkAppUDP6.Stop (Seconds (sink_stop_time));



  //OnOffHelpers to send UDP and TCP data to respective destinations

  OnOffHelper clientHelperTCP0 ("ns3::TcpSocketFactory", Address ());
  clientHelperTCP0.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperTCP0.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperTCP0.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperTCP0.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));

  OnOffHelper clientHelperTCP1 ("ns3::TcpSocketFactory", Address ());
  clientHelperTCP1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperTCP1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperTCP1.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperTCP1.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));
 
  OnOffHelper clientHelperTCP2 ("ns3::TcpSocketFactory", Address ());
  clientHelperTCP2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperTCP2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperTCP2.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperTCP2.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));
 
  OnOffHelper clientHelperTCP3 ("ns3::TcpSocketFactory", Address ());
  clientHelperTCP3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperTCP3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperTCP3.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperTCP3.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));
  
  OnOffHelper clientHelperTCP4 ("ns3::TcpSocketFactory", Address ());
  clientHelperTCP4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperTCP4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperTCP4.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperTCP4.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));


// UDP Sinks

  OnOffHelper clientHelperUDP5 ("ns3::UdpSocketFactory", Address ());
  clientHelperUDP5.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperUDP5.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperUDP5.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperUDP5.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));

  OnOffHelper clientHelperUDP6 ("ns3::UdpSocketFactory", Address ());
  clientHelperUDP6.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelperUDP6.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelperUDP6.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelperUDP6.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));


  ApplicationContainer clientAppsTCP0,clientAppsTCP1,clientAppsTCP2,clientAppsTCP3,clientAppsTCP4,clientAppsUDP5,clientAppsUDP6;


  AddressValue remoteAddressTCP0 (InetSocketAddress (i8i00.GetAddress (1), port_tcp)); //Address to the tcp sink
  clientHelperTCP0.SetAttribute ("Remote", remoteAddressTCP0);
  
  AddressValue remoteAddressTCP1 (InetSocketAddress (i8i10.GetAddress (1), port_tcp)); //Address to the tcp sink
  clientHelperTCP1.SetAttribute ("Remote", remoteAddressTCP1);
  
  AddressValue remoteAddressTCP2 (InetSocketAddress (i8i20.GetAddress (1), port_tcp)); //Address to the tcp sink
  clientHelperTCP2.SetAttribute ("Remote", remoteAddressTCP2);
  
  AddressValue remoteAddressTCP3 (InetSocketAddress (i8i30.GetAddress (1), port_tcp)); //Address to the tcp sink
  clientHelperTCP3.SetAttribute ("Remote", remoteAddressTCP3);
  
  AddressValue remoteAddressTCP4 (InetSocketAddress (i8i40.GetAddress (1), port_tcp)); //Address to the tcp sink
  clientHelperTCP4.SetAttribute ("Remote", remoteAddressTCP4);
  
  //Installing TCP on nodes 0-4
  clientAppsTCP0.Add (clientHelperTCP0.Install (n0n7.Get (0)));
  clientAppsTCP1.Add (clientHelperTCP1.Install (n1n7.Get (0)));
  clientAppsTCP2.Add (clientHelperTCP2.Install (n2n7.Get (0)));
  clientAppsTCP3.Add (clientHelperTCP3.Install (n3n7.Get (0)));
  clientAppsTCP4.Add (clientHelperTCP4.Install (n4n7.Get (0)));

  clientAppsTCP0.Start (Seconds (client_start_time));
  clientAppsTCP0.Stop (Seconds (client_stop_time));

  clientAppsTCP1.Start (Seconds (client_start_time));
  clientAppsTCP1.Stop (Seconds (client_stop_time));

  clientAppsTCP2.Start (Seconds (client_start_time));
  clientAppsTCP2.Stop (Seconds (client_stop_time));

  clientAppsTCP3.Start (Seconds (client_start_time));
  clientAppsTCP3.Stop (Seconds (client_stop_time));

  clientAppsTCP4.Start (Seconds (client_start_time));
  clientAppsTCP4.Stop (Seconds (client_stop_time));


  AddressValue remoteAddressUDP5 (InetSocketAddress (i8i50.GetAddress (1), port_udp)); // Address to the udp sink
  clientHelperUDP5.SetAttribute ("Remote", remoteAddressUDP5);

  AddressValue remoteAddressUDP6 (InetSocketAddress (i8i60.GetAddress (1), port_udp)); // Address to the udp sink
  clientHelperUDP6.SetAttribute ("Remote", remoteAddressUDP6);
  
  //Installing UDP on nodes 5-6
  clientAppsUDP5.Add (clientHelperUDP5.Install (n5n7.Get (0)));
  clientAppsUDP6.Add (clientHelperUDP6.Install (n6n7.Get (0)));

  clientAppsUDP5.Start (Seconds (client_start_time));
  clientAppsUDP5.Stop (Seconds (client_stop_time));

  clientAppsUDP6.Start (Seconds (client_start_time));
  clientAppsUDP6.Stop (Seconds (client_stop_time));

}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("FqCoDelQueueDisc", LOG_LEVEL_INFO);

  std::string FqcodelLinkDataRate = "10Mbps";
  std::string FqcodelLinkDelay = "32ms";

  std::string pathOut;
  bool writeForPlot = true;
  bool writePcap = true;
  bool flowMonitor = false;

  bool printFqCoDelStats = true;

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

  // The sources
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  Names::Add ( "N6", c.Get (6));

  // The bottlenecks
  Names::Add ( "N7", c.Get (7));
  Names::Add ( "N8", c.Get (8));

  // The sinks
  Names::Add ( "N00", c.Get (9));
  Names::Add ( "N10", c.Get (10));
  Names::Add ( "N20", c.Get (11));
  Names::Add ( "N30", c.Get (12));
  Names::Add ( "N40", c.Get (13));
  Names::Add ( "N50", c.Get (14));
  Names::Add ( "N60", c.Get (15));

  // The sources

  n0n7 = NodeContainer (c.Get (0), c.Get (7));
  n1n7 = NodeContainer (c.Get (1), c.Get (7));
  n2n7 = NodeContainer (c.Get (2), c.Get (7));
  n3n7 = NodeContainer (c.Get (3), c.Get (7));
  n4n7 = NodeContainer (c.Get (4), c.Get (7));
  n5n7 = NodeContainer (c.Get (5), c.Get (7));
  n6n7 = NodeContainer (c.Get (6), c.Get (7));
  
  // THe bottleneck
  n7n8 = NodeContainer (c.Get (7), c.Get (8));
  
  // The sinks
  n8n00 = NodeContainer (c.Get (8), c.Get (9));
  n8n10 = NodeContainer (c.Get (8), c.Get (10));
  n8n20 = NodeContainer (c.Get (8), c.Get (11));
  n8n30 = NodeContainer (c.Get (8), c.Get (12));
  n8n40 = NodeContainer (c.Get (8), c.Get (13));
  n8n50 = NodeContainer (c.Get (8), c.Get (14));
  n8n60 = NodeContainer (c.Get (8), c.Get (15));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // 42 = headers size
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  // Codel params
  NS_LOG_INFO ("Set CODEL params in Fqcodelqueuedisc");
  Config::SetDefault ("ns3::FqCoDelQueueDisc::MaxSize", StringValue ("100p"));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  TrafficControlHelper tchFqCoDel;
  tchFqCoDel.SetRootQueueDisc ("ns3::FqCoDelQueueDisc");

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  // The souces
  NetDeviceContainer devn0n7;
  NetDeviceContainer devn1n7;
  NetDeviceContainer devn2n7;
  NetDeviceContainer devn3n7;
  NetDeviceContainer devn4n7;
  NetDeviceContainer devn5n7;
  NetDeviceContainer devn6n7;
  
  // The bottleneck
  NetDeviceContainer devn7n8;
  
  // The sinks
  NetDeviceContainer devn8n00;
  NetDeviceContainer devn8n10;
  NetDeviceContainer devn8n20;
  NetDeviceContainer devn8n30;
  NetDeviceContainer devn8n40;
  NetDeviceContainer devn8n50;
  NetDeviceContainer devn8n60;

  QueueDiscContainer queueDiscs;


  // The sources
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

  // The bottleneck
  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (FqcodelLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (FqcodelLinkDelay));
  devn7n8 = p2p.Install (n7n8);
  // only backbone link has FQ-CODEL queue disc
  queueDiscs = tchFqCoDel.Install (devn7n8); 


  // The sinks
  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n00 = p2p.Install (n8n00);
  tchPfifo.Install (devn8n00);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n10 = p2p.Install (n8n10);
  tchPfifo.Install (devn8n10);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n20 = p2p.Install (n8n20);
  tchPfifo.Install (devn8n20);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n30 = p2p.Install (n8n30);
  tchPfifo.Install (devn8n30);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n40 = p2p.Install (n8n40);
  tchPfifo.Install (devn8n40);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n50 = p2p.Install (n8n50);
  tchPfifo.Install (devn8n50);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  devn8n60 = p2p.Install (n8n60);
  tchPfifo.Install (devn8n60);
  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;


  // The sources
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


  // The bottleneck
  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  i7i8 = ipv4.Assign (devn7n8);


  // The sinks
  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  i8i00 = ipv4.Assign (devn8n00);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  i8i10 = ipv4.Assign (devn8n10);

   ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  i8i20 = ipv4.Assign (devn8n20);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  i8i30 = ipv4.Assign (devn8n30);

  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  i8i40 = ipv4.Assign (devn8n40);

  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  i8i50 = ipv4.Assign (devn8n50);

   ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  i8i60 = ipv4.Assign (devn8n60);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/fqcodel";
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
      filePlotQueueDisc << pathOut << "/" << "fq-codel-queue-disc.plotme";
      filePlotQueueDiscAvg << pathOut << "/" << "fq-codel-queue-disc_avg.plotme";

      remove (filePlotQueueDisc.str ().c_str ());
      remove (filePlotQueueDiscAvg.str ().c_str ());
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
      Simulator::ScheduleNow (&CheckQueueDiscSize, queue);
    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  QueueDisc::Stats st = queueDiscs.Get (0)->GetStats ();

  if (st.GetNDroppedPackets (FqCoDelQueueDisc::OVERLIMIT_DROP) != 0)
    {
      std::cout << "There should be no drops due to queue full." << std::endl;
    }

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/fqcodel.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

  if (printFqCoDelStats)
    {
      std::cout << "***FQ CoDel stats from Node 2 queue ***" << std::endl;
      std::cout << "\t " << st.GetNDroppedPackets (FqCoDelQueueDisc::UNCLASSIFIED_DROP)
                << " drops due to prob mark" << std::endl;
      std::cout << "\t " << st.GetNDroppedPackets (FqCoDelQueueDisc::OVERLIMIT_DROP)
                << " drops due to queue limits" << std::endl;
    }

  Simulator::Destroy ();

  return 0;
}
