# FQ-PIE-ns-3


Resources used:
  1. RFC 8033, Proportional Integral Controller Enhanced (PIE  https://tools.ietf.org/html/rfc8033 
  2. IEEE paper on PIE http://ieeexplore.ieee.org/document/6602305/
  3. RFC 8290, Fq-Codel https://tools.ietf.org/html/rfc8290
  4. Paper related  to FQ-PIE http://caia.swin.edu.au/reports/160418A/CAIA-TR-160418A.pdf
Design Goals of PIE (As in RFC 8033):

  *  First, queuing latency, instead of queue length, is controlled.
      Queue sizes change with queue draining rates and various flows'
      round-trip times.  Latency bloat is the real issue that needs to
      be addressed, as it impairs real-time applications.  If latency
      can be controlled, bufferbloat is not an issue.  In fact, once
      latency is under control, it frees up buffers for sporadic bursts.

   *  Secondly, PIE aims to attain high link utilization.  The goal of
      low latency shall be achieved without suffering link
      underutilization or losing network efficiency.  An early
      congestion signal could cause TCP to back off and avoid queue
      buildup.  On the other hand, however, TCP's rate reduction could
      result in link underutilization.  There is a delicate balance
      between achieving high link utilization and low latency.

   *  Furthermore, the scheme should be simple to implement and easily
      scalable in both hardware and software.  PIE strives to maintain
      design simplicity similar to that of RED, which has been
      implemented in a wide variety of network devices.

   *  Finally, the scheme should ensure system stability for various
      network topologies and scale well across an arbitrary number of
      streams.  Design parameters shall be set automatically.  Users
      only need to set performance-related parameters such as target
      queue latency, not design parameters.
      
About the protocol PIE: 
  1. FQ-PIE is FlowQueue combined with Proportional Integral controller Enhanced (PIE) algorithm to address the problem of bufferbloat in the network. 
  2. Bufferbloat is a phenomenon in which excess buffers in the network cause high latency and latency variation. 
  3. PIE,Proportional Integrated controller Enhanced can effectively control the average queuing delay to a target value.
  4. PIE, aims to keep the benefits of RED(Random Early Detection) and also include easy implementation and scalability to high speeds(Both are the reasons why RED failed).
  5. PIE drops random packets at the onset of congestion (like RED, for congestion control) but congestion detection is based on queuing latency rather than the queuing length as with RED. PIE uses a derivative of queuing latency to help determine congestion levels and an appropriate response. The control parameters of PIE are chosen by control theory but can also be made self tuning based on circumstances.
  6. The structure of PIE
  
  <pre>
  
       Random Drop
              /               --------------
      -------/  -------------->    | | | | | -------------->
             /|\                   | | | | |
              |               --------------
              |             Queue Buffer   \
              |                     |       \
              |                     |Queue   \
              |                     |Length   \
              |                     |          \
              |                    \|/         \/
              |          -----------------    -------------------
              |          |     Drop      |    |                 |
              -----<-----|  Probability  |<---| Latency         |
                         |  Calculation  |    | Calculation     |
                         -----------------    -------------------
  </pre>
  
  When a packet arrives, a random decision is made regarding whether to drop thepacket.  The drop probability is updated periodically based on how far the current latency is away from the target value and whether thequeuing latency is currently trending up or down.  The queuing latency can be obtained using direct measurements or using estimations calculated from the queue length and the dequeue rate.
  7. **Random dropping:** Dropping of an incoming packet is decided based upon a certain _drop probability_ which is obtained from the _drop-probability-calculation-component_ of PIE. The random drop is bypassed if the older queue delay sample is less than half the target latency value when the drop probability is <0.2 or queue has less than a couple of packets.
        In conclusion,on packet arrival,
        <pre>
        if ( (PIE->qdelay_old_ < QDELAY_REF/2 && PIE->drop_prob_ < 0.2)
            || (queue_.byte_length() <= 2 * MEAN_PKTSIZE) )
                return ENQUE;
      else
         randomly drop the packet with a probability of
         PIE->drop_prob_.
         </pre>
  8. **Drop probability calculation:** The drop probability is periodically updated based on the latency samples collected over a period of time. But when the congestion period ends, we might end up with high drop probabilty values, so PIE algorithm has a mechanism by which the drop probability decays exonentially when congestion doesn't exist. The update interval(T_UPDATE) of the drop-probability is defaulted to 15ms(MAY be reduced in high speed links for smoother response). 
      The control parameters _alpha_ and _beta_(in Hz) are designed using feedback loop analysis. If T_UPDATE is cut in half, alpha must also be cut in half and beta must be increased by 0.25*alpha. 
   9. **Latency calculation:**
