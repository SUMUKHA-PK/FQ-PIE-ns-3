// /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// /*
//  * Copyright (c) 2018 NITK Surathkal
//  *
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License version 2 as
//  * published by the Free Software Foundation;
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  *
//  * Authors:  Sumukha PK <sumukhapk46@gmail.com>
//  *           Prajval M  <26prajval98@gmail.com>
//  *           Ishaan R D <ishaanrd6@gmail.com>
//  *           Mohit P. Tahiliani <tahiliani@nitk.edu.in>
//  */

// /** Network topology
//  *
//  * 
//  *    100mB/s, 5ms |                    | 100mB/s, 5ms
//  * n0--------------|TCP                 |---------------n00 TCP sink
//  *                 |                    |
//  *                 |                    |
//  *    100mB/s, 5ms |                    | 100Mb/s, 5ms
//  * n1--------------|TCP                 |---------------n10 TCP sink
//  *                 |                    |
//  *                 |                    |
//  *    100Mb/s, 5ms |TCP                 | 100Mb/s, 5ms
//  * n2--------------|                    |---------------n20 TCP sink
//  *                 |    10Mbps, 32ms    |
//  *                 n7------------------n8
//  *    100Mb/s, 5ms |  QueueLimit = 100  |    
//  *                 |                    |
//  *                 |                    | 100Mb/s, 5ms
//  * n3--------------|TCP                 |---------------n30 TCP sink
//  *                 |                    |
//  *                 |                    |
//  *    100mB/s, 5ms |TCP                 | 100Mb/s, 5ms
//  * n4--------------|                    |--------------- n40 TCP sink
//  *                 |                    |
//  *                 |                    | 
//  *    100mB/s, 5ms |UDP                 | 100Mb/s, 5ms
//  * n5--------------|                    |--------------- n50 UDP sink
//  *                 |                    |
//  *                 |                    |
//  *    100mB/s, 5ms |UDP                 | 100Mb/s, 5ms
//  * n6--------------|                    |--------------- n60 UDP sink
//  */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/tcp-header.h"
#include "ns3/traffic-control-module.h"
#include  <string>

using namespace ns3;

std::string dir ="FqCoDelTCP50/";

// int f=0;
// int x=0;

void
CheckQueueSize (Ptr<QueueDisc> queue,Ptr<FlowMonitor> monitor )
{
  double qSize = queue->GetCurrentSize ().GetValue ();
  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.1), &CheckQueueSize, queue,monitor);

  std::ofstream fPlotQueue (dir + "queueTraces/queue0.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();

   std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats (); 

  // int y ;
  // if(f==0){
  //   y = stats[1].delaySum.GetSeconds ();
  //   f=1;
  //   x = y;
  // }
  // else{
  //   y =stats[1].delaySum.GetSeconds () - x ;
  //   x = y;
  // }
  std::ofstream fPlotQueue1 (dir + "queueTraces/queue1.plotme", std::ios::out | std::ios::app);
  fPlotQueue1 << Simulator::Now ().GetSeconds () << " " << 1000* stats[1].delaySum.GetSeconds () / stats[1].rxPackets << std::endl;
  fPlotQueue1.close ();

  std::cout << "  Mean delay:   " << stats[1].delaySum.GetSeconds () / stats[1].rxPackets << std::endl;
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd / 1446.0 << std::endl;
}

static void
cwnd ()
{
  for(int i=0;i<50;i++)
  {
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + "cwndTraces/S1-" + std::to_string (i + 1) + ".plotme");
    
    Config::ConnectWithoutContext ("/NodeList/" + std::to_string (i) + "/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream));
  }
}

int main (int argc, char *argv[])
{
  int i = 0;
  float startTime = 0.0;
  float simDuration = 101;      // in seconds
  std::string  pathOut = ".";
  bool writeForPlot = true;
  // std::string EcnMode = "NoEcn";
  // bool useEcn = false;
  std::string queue_disc_type = "FqCoDelQueueDisc";
  float stopTime = startTime + simDuration;
  bool bql = true;

  CommandLine cmd;
  cmd.AddValue ("queue_disc_type", "Queue disc type for gateway (e.g. ns3::FqCoDelQueueDisc)", queue_disc_type);
  cmd.Parse (argc,argv);

  queue_disc_type = std::string ("ns3::") + queue_disc_type;

  TypeId qdTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (queue_disc_type, &qdTid), "TypeId " << queue_disc_type << " not found");

  std::string bottleneckBandwidth = "100Mbps";
  std::string bottleneckDelay = "50ms";

  std::string accessBandwidth = "100Mbps";
  std::string accessDelay = "5ms";

  NodeContainer source;
  source.Create (50);

  NodeContainer gateway;
  gateway.Create (2);

  NodeContainer sink;
  sink.Create (1);

  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (Seconds (0)));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
  Config::SetDefault ("ns3::TcpSocketBase::LimitedTransmit", BooleanValue (false));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1446));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));
  Config::SetDefault (queue_disc_type + "::MaxSize", QueueSizeValue (QueueSize ("1000p")));

  InternetStackHelper internet;
  internet.InstallAll ();

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize", StringValue ("1000p"));

  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queue_disc_type);

  if (bql)
     {
       tch.SetQueueLimits ("ns3::DynamicQueueLimits");
     }
     Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("100p"));


  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

// Create and configure access link and bottleneck link
  PointToPointHelper accessLink;
  accessLink.SetDeviceAttribute ("DataRate", StringValue (accessBandwidth));
  accessLink.SetChannelAttribute ("Delay", StringValue (accessDelay));

  NetDeviceContainer devices[50];
  for (i = 0; i < 50; i++)
    {
      devices[i] = accessLink.Install (source.Get (i), gateway.Get (0));
      tchPfifo.Install (devices[i]);
    }

  NetDeviceContainer devices_sink;
  devices_sink = accessLink.Install (gateway.Get (1), sink.Get (0));
  tchPfifo.Install (devices_sink);

  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute ("DataRate", StringValue (bottleneckBandwidth));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue (bottleneckDelay));

  NetDeviceContainer devices_gateway;
  devices_gateway = bottleneckLink.Install (gateway.Get (0), gateway.Get (1));
  QueueDiscContainer queueDiscs = tch.Install (devices_gateway);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  // Configure the source and sink net devices
  // and the channels between the source/sink and the gateway
  //Ipv4InterfaceContainer sink_Interfaces;
  Ipv4InterfaceContainer interfaces[50];
  Ipv4InterfaceContainer interfaces_sink;
  Ipv4InterfaceContainer interfaces_gateway;

  for (i = 0; i < 50; i++)
    {
      address.NewNetwork ();
      interfaces[i] = address.Assign (devices[i]);
    }

  address.NewNetwork ();
  interfaces_gateway = address.Assign (devices_gateway);

  address.NewNetwork ();
  interfaces_sink = address.Assign (devices_sink);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

  // Configure application
  AddressValue remoteAddress (InetSocketAddress (interfaces_sink.GetAddress (1), port));
  BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
  ftp.SetAttribute ("Remote", remoteAddress);
  ftp.SetAttribute ("SendSize", UintegerValue (1000));

  ApplicationContainer sourceApp = ftp.Install (source);
  sourceApp.Start (Seconds (0));
  sourceApp.Stop (Seconds (stopTime - 1));

  sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
  ApplicationContainer sinkApp = sinkHelper.Install (sink);
  sinkApp.Start (Seconds (0));
  sinkApp.Stop (Seconds (stopTime));

  if (writeForPlot)
    {
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
      Simulator::ScheduleNow (&CheckQueueSize, queue,monitor);
    }

  std::string dirToSave = "mkdir -p " + dir;
  system (dirToSave.c_str ());
  system ((dirToSave + "/pcap/").c_str ());
  system ((dirToSave + "/cwndTraces/").c_str ());
  system ((dirToSave + "/queueTraces/").c_str ());
  bottleneckLink.EnablePcapAll (dir + "pcap/N", true);
  Simulator::Schedule (Seconds (0.1), &cwnd);

  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
