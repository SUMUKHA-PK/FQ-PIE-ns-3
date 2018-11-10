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
 *           Ishaan R D <ishaanrd6@gmail.com>
 *           Prajval M  <26prajval98@gmail.com>
 *           Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "fq-pie-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/net-device-queue-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FqPieQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (FqPieQueueDisc);

TypeId FqPieQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FqPieQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<FqPieQueueDisc> ()
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&FqPieQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("A",
                   "Value of alpha",
                   DoubleValue (0.125),
                   MakeDoubleAccessor (&FqPieQueueDisc::m_a),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Value of beta",
                   DoubleValue (1.25),
                   MakeDoubleAccessor (&FqPieQueueDisc::m_b),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Tupdate",
                   "Time period to calculate drop probability",
                   TimeValue (Seconds (0.03)),
                   MakeTimeAccessor (&FqPieQueueDisc::m_tUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("Supdate",
                   "Start time of the update timer",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&FqPieQueueDisc::m_sUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("25p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("DequeueThreshold",
                   "Minimum queue size in bytes before dequeue rate is measured",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&FqPieQueueDisc::m_dqThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("QueueDelayReference",
                   "Desired queue delay",
                   TimeValue (Seconds (0.02)),
                   MakeTimeAccessor (&FqPieQueueDisc::m_qDelayRef),
                   MakeTimeChecker ())
    .AddAttribute ("MaxBurstAllowance",
                   "Current max burst allowance in seconds before random drop",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&FqPieQueueDisc::m_maxBurst),
                   MakeTimeChecker ())
  ;

  return tid;
}
// TypeId FqPieFlow::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::FqPieFlow")
//     .SetParent<QueueDiscClass> ()
//     .SetGroupName ("TrafficControl")
//     .AddConstructor<FqPieFlow> ()
//   ;
//   return tid;
// }


FqPieQueueDisc::FqPieQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES, QueueSizeUnit::PACKETS),
    m_quantum(0)
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
  m_rtrsEvent = Simulator::Schedule (m_sUpdate, &FqPieQueueDisc::CalculateP, this);
}

FqPieQueueDisc::~FqPieQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
FqPieFlow::SetDeficit (uint32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit = deficit;
}

int32_t
FqPieFlow::GetDeficit (void) const
{
  NS_LOG_FUNCTION (this);
  return m_deficit;
}

void
FqPieFlow::IncreaseDeficit (int32_t deficit)
{
  NS_LOG_FUNCTION (this << deficit);
  m_deficit += deficit;
}

void
FqPieFlow::SetStatus (FlowStatus status)
{
  NS_LOG_FUNCTION (this);
  m_status = status;
}

FqPieFlow::FlowStatus
FqPieFlow::GetStatus (void) const
{
  NS_LOG_FUNCTION (this);
  return m_status;
}

void
FqPieQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  Simulator::Remove (m_rtrsEvent);
  QueueDisc::DoDispose ();
}

Time
FqPieQueueDisc::GetQueueDelay (void)
{
  NS_LOG_FUNCTION (this);
  return m_qDelay;
}


//changes needed/
int64_t
FqPieQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}


//changes needed/
bool
FqPieQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);


  //hashing to the right queue
  uint32_t h = 0;

  if (GetNPacketFilters () == 0)
    {
      h = item->Hash (m_perturbation) % m_flows;
    }
  else
    {
      int32_t ret = Classify (item);

      if (ret != PacketFilter::PF_NO_MATCH)
        {
          h = ret % m_flows;
        }
      else
        {
          NS_LOG_ERROR ("No filter has been able to classify this packet, drop it.");
          DropBeforeEnqueue (item, UNCLASSIFIED_DROP);
          return false;
        }
    }

  Ptr<FqPieFlow> flow;
  Ptr<QueueDisc>  qd;
  if (m_flowsIndices.find (h) == m_flowsIndices.end ())
    {
      NS_LOG_DEBUG ("Creating a new flow queue with index " << h);
      flow = m_flowFactory.Create<FqPieFlow> ();
      qd = m_queueDiscFactory.Create<QueueDisc> ();
      qd->Initialize ();
      flow->SetQueueDisc (qd);
      AddQueueDiscClass (flow);

      m_flowsIndices[h] = GetNQueueDiscClasses () - 1;
    }
  else
    {
      flow = StaticCast<FqPieFlow> (GetQueueDiscClass (m_flowsIndices[h]));
    }

  if (flow->GetStatus () == FqPieFlow::INACTIVE)
    {
      flow->SetStatus (FqPieFlow::NEW_FLOW);
      flow->SetDeficit (m_quantum);
      m_newFlows.push_back (flow);
    }

  QueueSize nQueued = qd->GetCurrentSize (); //getting the size of the current qd

  if (nQueued + item > qd->GetMaxSize ())
    {
      // Drops due to queue limit: reactive
      DropBeforeEnqueue (item, FORCED_DROP);
      return false;
    }
  else if (DropEarly (item, nQueued.GetValue ()))
    {
      // Early probability drop: proactive
      DropBeforeEnqueue (item, UNFORCED_DROP);
      return false;
    }

  // No drop
  bool retval = qd->GetInternalQueue (0)->Enqueue (item); //The queue selected gets the packet

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("\t bytesInQueue  " << qd->GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << qd->GetInternalQueue (0)->GetNPackets ());

  return retval;
}

//changes needed/
void
FqPieQueueDisc::InitializeParams (void)
{
  // Initially queue is empty so variables are initialize to zero except m_dqCount
  m_inMeasurement = false;
  m_dqCount = DQCOUNT_INVALID;
  m_dropProb = 0;
  m_avgDqRate = 0.0;
  m_dqStart = 0;
  m_burstState = NO_BURST;
  m_qDelayOld = Time (Seconds (0));
}


//changes needed/
bool FqPieQueueDisc::DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize)
{
  NS_LOG_FUNCTION (this << item << qSize);
  if (m_burstAllowance.GetSeconds () > 0)
    {
      // If there is still burst_allowance left, skip random early drop.
      return false;
    }

  if (m_burstState == NO_BURST)
    {
      m_burstState = IN_BURST_PROTECTING;
      m_burstAllowance = m_maxBurst;
    }

  double p = m_dropProb;

  uint32_t packetSize = item->GetSize ();

  if (GetMaxSize ().GetUnit () == QueueSizeUnit::BYTES)
    {
      p = p * packetSize / m_meanPktSize;
    }
  bool earlyDrop = true;
  double u =  m_uv->GetValue ();

// Following conditions are where the packet must not be dropped 
  if ((m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb < 0.2)) //as mentioned in the rfc explicitly
    {
      return false;
    }
  else if (GetMaxSize ().GetUnit () == QueueSizeUnit::BYTES && qSize <= 2 * m_meanPktSize)  // if there are less than 2 packets or bytes then do not drop(like it just)
    {
      return false;
    }
  else if (GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS && qSize <= 2) 
    {
      return false;
    }

  if (u > p)
    {
      earlyDrop = false;
    }
  if (!earlyDrop)
    {
      return false;
    }

  return true;
}

//changes needed/
void FqPieQueueDisc::CalculateP ()
{
  NS_LOG_FUNCTION (this);
  Time qDelay;
  double p = 0.0;
  bool missingInitFlag = false;
  if (m_avgDqRate > 0)
    {
      qDelay = Time (Seconds (GetInternalQueue (0)->GetNBytes () / m_avgDqRate));
    }
  else
    {
      qDelay = Time (Seconds (0));
      missingInitFlag = true;
    }

  m_qDelay = qDelay;

  if (m_burstAllowance.GetSeconds () > 0)
    {
      m_dropProb = 0;
    }
  else
    {
      p = m_a * (qDelay.GetSeconds () - m_qDelayRef.GetSeconds ()) + m_b * (qDelay.GetSeconds () - m_qDelayOld.GetSeconds ());
      if (m_dropProb < 0.001)
        {
          p /= 32;
        }
      else if (m_dropProb < 0.01)
        {
          p /= 8;
        }
      else if (m_dropProb < 0.1)
        {
          p /= 2;
        }
      else if (m_dropProb < 1)
        {
          p /= 0.5;
        }
      else if (m_dropProb < 10)
        {
          p /= 0.125;
        }
      else
        {
          p /= 0.03125;
        }
      if ((m_dropProb >= 0.1) && (p > 0.02))
        {
          p = 0.02;
        }
    }

  p += m_dropProb;

  // For non-linear drop in prob

  if (qDelay.GetSeconds () == 0 && m_qDelayOld.GetSeconds () == 0)
    {
      p *= 0.98;
    }
  else if (qDelay.GetSeconds () > 0.2)
    {
      p += 0.02;
    }

  m_dropProb = (p > 0) ? p : 0;
  if (m_burstAllowance < m_tUpdate)
    {
      m_burstAllowance =  Time (Seconds (0));
    }
  else
    {
      m_burstAllowance -= m_tUpdate;
    }

  uint32_t burstResetLimit = static_cast<uint32_t>(BURST_RESET_TIMEOUT / m_tUpdate.GetSeconds ());
  if ( (qDelay.GetSeconds () < 0.5 * m_qDelayRef.GetSeconds ()) && (m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb == 0) && !missingInitFlag )
    {
      m_dqCount = DQCOUNT_INVALID;
      m_avgDqRate = 0.0;
    }
  if ( (qDelay.GetSeconds () < 0.5 * m_qDelayRef.GetSeconds ()) && (m_qDelayOld.GetSeconds () < (0.5 * m_qDelayRef.GetSeconds ())) && (m_dropProb == 0) && (m_burstAllowance.GetSeconds () == 0))
    {
      if (m_burstState == IN_BURST_PROTECTING)
        {
          m_burstState = IN_BURST;
          m_burstReset = 0;
        }
      else if (m_burstState == IN_BURST)
        {
          m_burstReset++;
          if (m_burstReset > burstResetLimit)
            {
              m_burstReset = 0;
              m_burstState = NO_BURST;
            }
        }
    }
  else if (m_burstState == IN_BURST)
    {
      m_burstReset = 0;
    }

  m_qDelayOld = qDelay;
  m_rtrsEvent = Simulator::Schedule (m_tUpdate, &FqPieQueueDisc::CalculateP, this);
}

Ptr<QueueDiscItem>
FqPieQueueDisc::DoDequeue ()  //this is an internal function of queue disc, this makes the queue behave the following way
{
  NS_LOG_FUNCTION (this);

  Ptr<FqPieFlow> flow;
  Ptr<QueueDiscItem> item;

  do
    {
      bool found = false;

      while (!found && !m_newFlows.empty ())
        {
          flow = m_newFlows.front ();

          if (flow->GetDeficit () <= 0)
            {
              flow->IncreaseDeficit (m_quantum);
              flow->SetStatus (FqPieFlow::OLD_FLOW);
              m_oldFlows.push_back (flow);
              m_newFlows.pop_front ();
            }
          else
            {
              NS_LOG_DEBUG ("Found a new flow with positive deficit");
              found = true;
            }
        }

      while (!found && !m_oldFlows.empty ())
        {
          flow = m_oldFlows.front ();

          if (flow->GetDeficit () <= 0)
            {
              flow->IncreaseDeficit (m_quantum);
              m_oldFlows.push_back (flow);
              m_oldFlows.pop_front ();
            }
          else
            {
              NS_LOG_DEBUG ("Found an old flow with positive deficit");
              found = true;
            }
        }

      if (!found)
        {
          NS_LOG_DEBUG ("No flow found to dequeue a packet");
          return 0;
        }

      item = flow->GetQueueDisc ()->Dequeue ();

      if (!item)
        {
          NS_LOG_DEBUG ("Could not get a packet from the selected flow queue");
          if (!m_newFlows.empty ())
            {
              flow->SetStatus (FqPieFlow::OLD_FLOW);
              m_oldFlows.push_back (flow);
              m_newFlows.pop_front ();
            }
          else
            {
              flow->SetStatus (FqPieFlow::INACTIVE);
              m_oldFlows.pop_front ();
            }
        }
      else
        {
          NS_LOG_DEBUG ("Dequeued packet " << item->GetPacket ());
        }
    } while (item == 0);

  flow->IncreaseDeficit (item->GetSize () * -1);

  return item;
}

bool
FqPieQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FqPieQueueDisc cannot have classes");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add  a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("FqPieQueueDisc needs 1 internal queue");
      return false;
    }

  if (!m_quantum)
    {
      Ptr<NetDeviceQueueInterface> ndqi = GetNetDeviceQueueInterface ();
      Ptr<NetDevice> dev;
      // if the NetDeviceQueueInterface object is aggregated to a
      // NetDevice, get the MTU of such NetDevice
      if (ndqi && (dev = ndqi->GetObject<NetDevice> ()))
        {
          m_quantum = dev->GetMtu ();
          NS_LOG_DEBUG ("Setting the quantum to the MTU of the device: " << m_quantum);
        }

      if (!m_quantum)
        {
          NS_LOG_ERROR ("The quantum parameter cannot be null");
          return false;
        }
    }

  return true;
}

} //namespace ns3
