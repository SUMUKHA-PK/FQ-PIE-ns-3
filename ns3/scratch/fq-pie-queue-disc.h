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


#ifndef FQ_PIE_QUEUE_DISC_H
#define FQ_PIE_QUEUE_DISC_H

#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"

#define BURST_RESET_TIMEOUT 1.5

namespace ns3 {

class TraceContainer;
class UniformRandomVariable;

class FqPieFlow : public QueueDiscClass {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  //static TypeId GetTypeId (void);
  /**
   * \brief FqPieFlow constructor
   */
  FqPieFlow ();

  virtual ~FqPieFlow ();

  /**
   * \enum FlowStatus
   * \brief Used to determine the status of this flow queue
   */
  enum FlowStatus
    {
      INACTIVE,
      NEW_FLOW,
      OLD_FLOW
    };

  /**
   * \brief Set the deficit for this flow
   * \param deficit the deficit for this flow
   */
  void SetDeficit (uint32_t deficit);
  /**
   * \brief Get the deficit for this flow
   * \return the deficit for this flow
   */
  int32_t GetDeficit (void) const;
  /**
   * \brief Increase the deficit for this flow
   * \param deficit the amount by which the deficit is to be increased
   */
  void IncreaseDeficit (int32_t deficit);
  /**
   * \brief Set the status for this flow
   * \param status the status for this flow
   */
  void SetStatus (FlowStatus status);
  /**
   * \brief Get the status of this flow
   * \return the status of this flow
   */
  FlowStatus GetStatus (void) const;

private:
  int32_t m_deficit;    //!< the deficit for this flow
  FlowStatus m_status;  //!< the status of this flow
};

/**
 * \ingroup traffic-control
 *
 * \brief Implements PIE Active Queue Management discipline
 */
class FqPieQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief FqPieQueueDisc Constructor
   */
  FqPieQueueDisc ();

  /**
   * \brief FqPieQueueDisc Destructor
   */
  virtual ~FqPieQueueDisc ();

  /**
   * \brief Burst types
   */
  enum BurstStateT
  {
    NO_BURST,
    IN_BURST,
    IN_BURST_PROTECTING,
  };

  /**
   * \brief Get queue delay.
   *
   * \returns The current queue delay.
   */
  Time GetQueueDelay (void);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

  // Reasons for dropping packets
  static constexpr const char* UNFORCED_DROP = "Unforced drop";  //!< Early probability drops: proactive
  static constexpr const char* FORCED_DROP = "Forced drop";      //!< Drops due to queue limit: reactive
  static constexpr const char* UNCLASSIFIED_DROP = "Unclassified drop";  //!< No packet filter able to classify packet
  

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual bool CheckConfig (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  /**
   * \brief Check if a packet needs to be dropped due to probability drop
   * \param item queue item
   * \param qSize queue size
   * \returns 0 for no drop, 1 for drop
   */
  bool DropEarly (Ptr<QueueDiscItem> item, uint32_t qSize);

  /**
   * Periodically update the drop probability based on the delay samples:
   * not only the current delay sample but also the trend where the delay
   * is going, up or down
   */
  void CalculateP ();

  static const uint64_t DQCOUNT_INVALID = std::numeric_limits<uint64_t>::max();  //!< Invalid dqCount value

  // ** Variables supplied by user
  Time m_sUpdate;                               //!< Start time of the update timer
  Time m_tUpdate;                               //!< Time period after which CalculateP () is called
  Time m_qDelayRef;                             //!< Desired queue delay
  uint32_t m_meanPktSize;                       //!< Average packet size in bytes
  Time m_maxBurst;                              //!< Maximum burst allowed before random early dropping kicks in
  double m_a;                                   //!< Parameter to pie controller
  double m_b;                                   //!< Parameter to pie controller
  uint32_t m_dqThreshold;                       //!< Minimum queue size in bytes before dequeue rate is measured
  uint32_t m_quantum;                          //!< Deficit assigned to flows at each round
  uint32_t m_flows;                            //!< Number of flow queues
  uint32_t m_dropBatchSize;                    //!< Max number of packets dropped from the fat flow
  uint32_t m_perturbation;                     //!< hash perturbation value

  std::list<Ptr<FqPieFlow> > m_newFlows;    //!< The list of new flows
  std::list<Ptr<FqPieFlow> > m_oldFlows;    //!< The list of old flows

  // ** Variables maintained by PIE
  double m_dropProb;                            //!< Variable used in calculation of drop probability
  Time m_qDelayOld;                             //!< Old value of queue delay
  Time m_qDelay;                                //!< Current value of queue delay
  Time m_burstAllowance;                        //!< Current max burst value in seconds that is allowed before random drops kick in
  uint32_t m_burstReset;                        //!< Used to reset value of burst allowance
  BurstStateT m_burstState;                     //!< Used to determine the current state of burst
  bool m_inMeasurement;                         //!< Indicates whether we are in a measurement cycle
  double m_avgDqRate;                           //!< Time averaged dequeue rate
  double m_dqStart;                             //!< Start timestamp of current measurement cycle
  uint64_t m_dqCount;                           //!< Number of bytes departed since current measurement cycle starts
  EventId m_rtrsEvent;                          //!< Event used to decide the decision of interval of drop probability calculation
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream
  ObjectFactory m_flowFactory;         //!< Factory to create a new flow
  ObjectFactory m_queueDiscFactory;    //!< Factory to create a new queue

  std::map<uint32_t, uint32_t> m_flowsIndices;    //!< Map with the index of class for each flow
};

};   // namespace ns3

#endif

