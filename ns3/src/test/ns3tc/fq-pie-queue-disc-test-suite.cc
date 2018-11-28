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


#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/fq-pie-queue-disc.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-packet-filter.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-packet-filter.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

using namespace ns3;

/**
 * Simple test packet filter able to classify IPv4 packets
 *
 */
class Ipv4TestPacketFilter : public Ipv4PacketFilter {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  Ipv4TestPacketFilter ();
  virtual ~Ipv4TestPacketFilter ();

private:
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;
};

TypeId
Ipv4TestPacketFilter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4TestPacketFilter")
    .SetParent<Ipv4PacketFilter> ()
    .SetGroupName ("Internet")
    .AddConstructor<Ipv4TestPacketFilter> ()
  ;
  return tid;
}

Ipv4TestPacketFilter::Ipv4TestPacketFilter ()
{
}

Ipv4TestPacketFilter::~Ipv4TestPacketFilter ()
{
}

int32_t
Ipv4TestPacketFilter::DoClassify (Ptr<QueueDiscItem> item) const
{
  return 0;
}

/**
 * This class tests packets for which there is no suitable filter
 */
class FqPieQueueDiscNoSuitableFilter : public TestCase
{
public:
  FqPieQueueDiscNoSuitableFilter ();
  virtual ~FqPieQueueDiscNoSuitableFilter ();

private:
  virtual void DoRun (void);
};

FqPieQueueDiscNoSuitableFilter::FqPieQueueDiscNoSuitableFilter ()
  : TestCase ("Test packets that are not classified by any filter")
{
}

FqPieQueueDiscNoSuitableFilter::~FqPieQueueDiscNoSuitableFilter ()
{
}

void
FqPieQueueDiscNoSuitableFilter::DoRun (void)
{
  // Packets that cannot be classified by the available filters should be dropped
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc> ("MaxSize", StringValue ("4p"));
  Ptr<Ipv4TestPacketFilter> filter = CreateObject<Ipv4TestPacketFilter> ();
  queueDisc->AddPacketFilter (filter);

  queueDisc->SetQuantum (1500);
  queueDisc->Initialize ();

  Ptr<Packet> p;
  p = Create<Packet> ();
  Ptr<Ipv6QueueDiscItem> item;
  Ipv6Header ipv6Header;
  Address dest;
  item = Create<Ipv6QueueDiscItem> (p, dest, 0, ipv6Header);
  queueDisc->Enqueue (item);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetNQueueDiscClasses (), 0, "no flow queue should have been created");

  p = Create<Packet> (reinterpret_cast<const uint8_t*> ("hello, world"), 12);
  item = Create<Ipv6QueueDiscItem> (p, dest, 0, ipv6Header);
  queueDisc->Enqueue (item);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetNQueueDiscClasses (), 0, "no flow queue should have been created");

  Simulator::Destroy ();
}

/**
 * This class tests the IP flows separation and the packet limit
 */
class FqPieQueueDiscIPFlowsSeparationAndPacketLimit : public TestCase
{
public:
  FqPieQueueDiscIPFlowsSeparationAndPacketLimit ();
  virtual ~FqPieQueueDiscIPFlowsSeparationAndPacketLimit ();

private:
  virtual void DoRun (void);
  void AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header hdr);
};

FqPieQueueDiscIPFlowsSeparationAndPacketLimit::FqPieQueueDiscIPFlowsSeparationAndPacketLimit ()
  : TestCase ("Test IP flows separation and packet limit")
{
}

FqPieQueueDiscIPFlowsSeparationAndPacketLimit::~FqPieQueueDiscIPFlowsSeparationAndPacketLimit ()
{
}

void
FqPieQueueDiscIPFlowsSeparationAndPacketLimit::AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header hdr)
{
  Ptr<Packet> p = Create<Packet> (100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, hdr);
  queue->Enqueue (item);
}

void
FqPieQueueDiscIPFlowsSeparationAndPacketLimit::DoRun (void)
{
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc> ("MaxSize", StringValue ("4p"));

  queueDisc->SetQuantum (1500);
  queueDisc->Initialize ();

  Ipv4Header hdr;
  hdr.SetPayloadSize (100);
  hdr.SetSource (Ipv4Address ("10.10.1.1"));
  hdr.SetDestination (Ipv4Address ("10.10.1.2"));
  hdr.SetProtocol (7);

  // Add three packets from the first flow
  AddPacket (queueDisc, hdr);
  AddPacket (queueDisc, hdr);
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 3, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the flow queue");

  // Add two packets from the second flow
  hdr.SetDestination (Ipv4Address ("10.10.1.7"));
  // Add the first packet
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 4, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the flow queue");
  // Add the second packet that causes two packets to be dropped from the fat flow (max backlog = 300, threshold = 150)
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 3, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the flow queue");

  Simulator::Destroy ();
}

/**
 * This class tests the deficit per flow
 */
class FqPieQueueDiscDeficit : public TestCase
{
public:
  FqPieQueueDiscDeficit ();
  virtual ~FqPieQueueDiscDeficit ();

private:
  virtual void DoRun (void);
  void AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header hdr);
};

FqPieQueueDiscDeficit::FqPieQueueDiscDeficit ()
  : TestCase ("Test credits and flows status")
{
}

FqPieQueueDiscDeficit::~FqPieQueueDiscDeficit ()
{
}

void
FqPieQueueDiscDeficit::AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header hdr)
{
  Ptr<Packet> p = Create<Packet> (100);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, hdr);
  queue->Enqueue (item);
}

void
FqPieQueueDiscDeficit::DoRun (void)
{
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc> ();

  queueDisc->SetQuantum (90);
  queueDisc->Initialize ();

  Ipv4Header hdr;
  hdr.SetPayloadSize (100);
  hdr.SetSource (Ipv4Address ("10.10.1.1"));
  hdr.SetDestination (Ipv4Address ("10.10.1.2"));
  hdr.SetProtocol (7);

  // Add a packet from the first flow
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 1, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the first flow queue");
  Ptr<FqPieFlow> flow1 = StaticCast<FqPieFlow> (queueDisc->GetQueueDiscClass (0));
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), static_cast<int32_t> (queueDisc->GetQuantum ()), "the deficit of the first flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::NEW_FLOW, "the first flow must be in the list of new queues");
  // Dequeue a packet
  queueDisc->Dequeue ();
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 0, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 0, "unexpected number of packets in the first flow queue");
  // the deficit for the first flow becomes 90 - (100+20) = -30
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), -30, "unexpected deficit for the first flow");

  // Add two packets from the first flow
  AddPacket (queueDisc, hdr);
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 2, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::NEW_FLOW, "the first flow must still be in the list of new queues");

  // Add two packets from the second flow
  hdr.SetDestination (Ipv4Address ("10.10.1.10"));
  AddPacket (queueDisc, hdr);
  AddPacket (queueDisc, hdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 4, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the second flow queue");
  Ptr<FqPieFlow> flow2 = StaticCast<FqPieFlow> (queueDisc->GetQueueDiscClass (1));
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), static_cast<int32_t> (queueDisc->GetQuantum ()), "the deficit of the second flow must equal the quantum");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::NEW_FLOW, "the second flow must be in the list of new queues");

  // Dequeue a packet (from the second flow, as the first flow has a negative deficit)
  queueDisc->Dequeue ();
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 3, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  // the first flow got a quantum of deficit (-30+90=60) and has been moved to the end of the list of old queues
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), 60, "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::OLD_FLOW, "the first flow must be in the list of old queues");
  // the second flow has a negative deficit (-30) and is still in the list of new queues
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), -30, "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::NEW_FLOW, "the second flow must be in the list of new queues");

  // Dequeue a packet (from the first flow, as the second flow has a negative deficit)
  queueDisc->Dequeue ();
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 2, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  // the first flow has a negative deficit (60-(100+20)= -60) and stays in the list of old queues
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), -60, "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::OLD_FLOW, "the first flow must be in the list of old queues");
  // the second flow got a quantum of deficit (-30+90=60) and has been moved to the end of the list of old queues
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), 60, "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::OLD_FLOW, "the second flow must be in the list of new queues");

  // Dequeue a packet (from the second flow, as the first flow has a negative deficit)
  queueDisc->Dequeue ();
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 1, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 0, "unexpected number of packets in the second flow queue");
  // the first flow got a quantum of deficit (-60+90=30) and has been moved to the end of the list of old queues
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), 30, "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::OLD_FLOW, "the first flow must be in the list of old queues");
  // the second flow has a negative deficit (60-(100+20)= -60)
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), -60, "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::OLD_FLOW, "the second flow must be in the list of new queues");

  // Dequeue a packet (from the first flow, as the second flow has a negative deficit)
  queueDisc->Dequeue ();
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 0, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 0, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 0, "unexpected number of packets in the second flow queue");
  // the first flow has a negative deficit (30-(100+20)= -90)
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), -90, "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::OLD_FLOW, "the first flow must be in the list of old queues");
  // the second flow got a quantum of deficit (-60+90=30) and has been moved to the end of the list of old queues
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), 30, "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::OLD_FLOW, "the second flow must be in the list of new queues");

  // Dequeue a packet
  queueDisc->Dequeue ();
  // the first flow is at the head of the list of old queues but has a negative deficit, thus it gets a quantun
  // of deficit (-90+90=0) and is moved to the end of the list of old queues. Then, the second flow (which has a
  // positive deficit) is selected, but the second flow is empty and thus it is set to inactive. The first flow is
  // reconsidered, but it has a null deficit, hence it gets another quantum of deficit (0+90=90). Then, the first
  // flow is reconsidered again, now it has a positive deficit and hence it is selected. But, it is empty and
  // therefore is set to inactive, too.
  NS_TEST_ASSERT_MSG_EQ (flow1->GetDeficit (), 90, "unexpected deficit for the first flow");
  NS_TEST_ASSERT_MSG_EQ (flow1->GetStatus (), FqPieFlow::INACTIVE, "the first flow must be inactive");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetDeficit (), 30, "unexpected deficit for the second flow");
  NS_TEST_ASSERT_MSG_EQ (flow2->GetStatus (), FqPieFlow::INACTIVE, "the second flow must be inactive");

  Simulator::Destroy ();
}

/**
 * This class tests the TCP flows separation
 */
class FqPieQueueDiscTCPFlowsSeparation : public TestCase
{
public:
  FqPieQueueDiscTCPFlowsSeparation ();
  virtual ~FqPieQueueDiscTCPFlowsSeparation ();

private:
  virtual void DoRun (void);
  void AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, TcpHeader tcpHdr);
};

FqPieQueueDiscTCPFlowsSeparation::FqPieQueueDiscTCPFlowsSeparation ()
  : TestCase ("Test TCP flows separation")
{
}

FqPieQueueDiscTCPFlowsSeparation::~FqPieQueueDiscTCPFlowsSeparation ()
{
}

void
FqPieQueueDiscTCPFlowsSeparation::AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, TcpHeader tcpHdr)
{
  Ptr<Packet> p = Create<Packet> (100);
  p->AddHeader (tcpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, ipHdr);
  queue->Enqueue (item);
}

void
FqPieQueueDiscTCPFlowsSeparation::DoRun (void)
{
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc> ("MaxSize", StringValue ("10p"));

  queueDisc->SetQuantum (1500);
  queueDisc->Initialize ();

  Ipv4Header hdr;
  hdr.SetPayloadSize (100);
  hdr.SetSource (Ipv4Address ("10.10.1.1"));
  hdr.SetDestination (Ipv4Address ("10.10.1.2"));
  hdr.SetProtocol (6);

  TcpHeader tcpHdr;
  tcpHdr.SetSourcePort (7);
  tcpHdr.SetDestinationPort (27);

  // Add three packets from the first flow
  AddPacket (queueDisc, hdr, tcpHdr);
  AddPacket (queueDisc, hdr, tcpHdr);
  AddPacket (queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 3, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");

  // Add a packet from the second flow
  tcpHdr.SetSourcePort (8);
  AddPacket (queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 4, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");

  // Add a packet from the third flow
  tcpHdr.SetDestinationPort (28);
  AddPacket (queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 5, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (2)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the third flow queue");

  // Add two packets from the fourth flow
  tcpHdr.SetSourcePort (7);
  AddPacket (queueDisc, hdr, tcpHdr);
  AddPacket (queueDisc, hdr, tcpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 7, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (2)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the third flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (3)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the third flow queue");

  Simulator::Destroy ();
}

/**
 * This class tests the UDP flows separation
 */
class FqPieQueueDiscUDPFlowsSeparation : public TestCase
{
public:
  FqPieQueueDiscUDPFlowsSeparation ();
  virtual ~FqPieQueueDiscUDPFlowsSeparation ();

private:
  virtual void DoRun (void);
  void AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, UdpHeader udpHdr);
};

FqPieQueueDiscUDPFlowsSeparation::FqPieQueueDiscUDPFlowsSeparation ()
  : TestCase ("Test UDP flows separation")
{
}

FqPieQueueDiscUDPFlowsSeparation::~FqPieQueueDiscUDPFlowsSeparation ()
{
}

void
FqPieQueueDiscUDPFlowsSeparation::AddPacket (Ptr<FqPieQueueDisc> queue, Ipv4Header ipHdr, UdpHeader udpHdr)
{
  Ptr<Packet> p = Create<Packet> (100);
  p->AddHeader (udpHdr);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, ipHdr);
  queue->Enqueue (item);
}

void
FqPieQueueDiscUDPFlowsSeparation::DoRun (void)
{
  Ptr<FqPieQueueDisc> queueDisc = CreateObjectWithAttributes<FqPieQueueDisc> ("MaxSize", StringValue ("10p"));

  queueDisc->SetQuantum (1500);
  queueDisc->Initialize ();

  Ipv4Header hdr;
  hdr.SetPayloadSize (100);
  hdr.SetSource (Ipv4Address ("10.10.1.1"));
  hdr.SetDestination (Ipv4Address ("10.10.1.2"));
  hdr.SetProtocol (17);

  UdpHeader udpHdr;
  udpHdr.SetSourcePort (7);
  udpHdr.SetDestinationPort (27);

  // Add three packets from the first flow
  AddPacket (queueDisc, hdr, udpHdr);
  AddPacket (queueDisc, hdr, udpHdr);
  AddPacket (queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 3, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");

  // Add a packet from the second flow
  udpHdr.SetSourcePort (8);
  AddPacket (queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 4, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");

  // Add a packet from the third flow
  udpHdr.SetDestinationPort (28);
  AddPacket (queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 5, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (2)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the third flow queue");

  // Add two packets from the fourth flow
  udpHdr.SetSourcePort (7);
  AddPacket (queueDisc, hdr, udpHdr);
  AddPacket (queueDisc, hdr, udpHdr);
  NS_TEST_ASSERT_MSG_EQ (queueDisc->QueueDisc::GetNPackets (), 7, "unexpected number of packets in the queue disc");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (0)->GetQueueDisc ()->GetNPackets (), 3, "unexpected number of packets in the first flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (1)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the second flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (2)->GetQueueDisc ()->GetNPackets (), 1, "unexpected number of packets in the third flow queue");
  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetQueueDiscClass (3)->GetQueueDisc ()->GetNPackets (), 2, "unexpected number of packets in the third flow queue");

  Simulator::Destroy ();
}

class FqPieQueueDiscTestSuite : public TestSuite
{
public:
  FqPieQueueDiscTestSuite ();
};

FqPieQueueDiscTestSuite::FqPieQueueDiscTestSuite ()
  : TestSuite ("fq-pie-queue-disc", UNIT)
{
  AddTestCase (new FqPieQueueDiscNoSuitableFilter, TestCase::QUICK);
  AddTestCase (new FqPieQueueDiscIPFlowsSeparationAndPacketLimit, TestCase::QUICK);
  AddTestCase (new FqPieQueueDiscDeficit, TestCase::QUICK);
  AddTestCase (new FqPieQueueDiscTCPFlowsSeparation, TestCase::QUICK);
  AddTestCase (new FqPieQueueDiscUDPFlowsSeparation, TestCase::QUICK);
}

static FqPieQueueDiscTestSuite fqPieQueueDiscTestSuite;
