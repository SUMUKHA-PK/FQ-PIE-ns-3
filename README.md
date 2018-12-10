# FQ-PIE-ns-3

***

## Resources used:
1. RFC 8033, Proportional Integral Controller Enhanced (PIE)  https://tools.ietf.org/html/rfc8033

2. RFC 8290, Fq-Codel https://tools.ietf.org/html/rfc8290

3. NS3 documentation (Doxygen), https://www.nsnam.org/doxygen/

4. Algorithm for PIE followed, https://tools.ietf.org/html/rfc8033#appendix-A

***


## Implementation Details ([details](https://github.com/SUMUKHA-PK/FQ-PIE-ns-3/wiki/Implementation-progress-updates))

## Implementation of FQ-PIE was done in NS3

### Files

* **fq-pie-queue-disc.h** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/model/fq-pie-queue-disc.h)) <br/> 
This file contains the basic definition of the the FqPieFlow class and FqPieQueueDisc class.

* **fq-pie-queue-disc.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/model/fq-pie-queue-disc.cc)) <br/> 
This file contains all the declarations of the functions in **fq-pie-queue-disc.h**

* **fq-pie-example-tcp.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/examples/fq-pie-example-tcp.cc)) <br/>
This file creates an environment where only TCP packets are in the network

* **fq-pie-example-udp.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/examples/fq-pie-example-udp.cc)) <br/>
This file creates an environment where TCP and UDP packets are in the network

* **fq-pie-example-udp-7-flows.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/examples/fq-pie-example-udp-7-flows.cc)) <br/>
This file creates an environment where TCP and UDP packets are in the network. There are 5 TCP flows and 2 UDP flows. This is the example used for comparision.

### Algorithm (important functions)

* **doEnqueue :**<br/>
Whenever a packet comes into the node (maybe a router), this function is called.<br />
For each flow of packet a hash function is used to determine where the packet which flow (or slot) the packet will be added to, There might be hash collisions possible. <br />
If the hash value `h` is not present in the list of flows, a new flow is created with a new queue disc `qd`, else the flow which is already present is selected. <br />
If flow status is INACTIVE then the flow status is set to NEW_FLOW and a quantum deficit is added <br />
Then pie algorithm is applied as mentioned in the Resources.
* **doDequeue :**<br />
Here the DRR algorithm is used in order to determine which queue has to dequeue. While checking for flows to dequeue, new flows are checked first. A flow is rendered old right after its checked for a packet to DQ. If there are no appropriate packets in new flow, an old flow is considered.
* **calculatePFlow :**<br />
`Simulator::Schedule` is used to schedule this function every mt_Update unit of time. It calls another function `calculateP` which performs the PIE variant of calculation of probability except that this is done to every flow `ACTIVE`.

### Important variables

* **Pie variables for each flow :** <br />
        `bool m_inMeasurement     ` ( Indicates whether we are in a measurement cycle )<br />
        `double m_avgDqRate       ` ( Time averaged dequeue rate )<br />
        `double m_dqStart         ` ( Start timestamp of current measurement cycle )<br />
        `uint64_t m_dqCount       ` ( Number of bytes departed since current measurement cycle starts )<br />
        `double m_dropProb        ` ( Variable used in calculation of drop probability )<br />
        `uint32_t m_burstReset    ` ( Used to reset value of burst allowance )<br />
        `Time m_qDelayOld         ` ( Old value of queue delay )<br />
        `Time m_qDelay            ` ( Current value of queue delay )<br />
        `BurstStateT m_burstState ` ( Used to determine the current     state of burst )<br />
        `Time m_burstAllowance    ` ( Current max burst value in seconds that is allowed before random drops kick in )<br />

* **Fq-PIE variables :**<br />
        `Time m_sUpdate           `( Start time of the update timer )
        `Time m_tUpdate           `( Time period after which CalculateP () is called )
        `Time m_qDelayRef         `( Desired queue delay )
        `uint32_t m_meanPktSize   `( Average packet size in bytes )
        `Time m_maxBurst          `( Maximum burst allowed before random early dropping kicks in )
        `double m_a               `( Parameter to pie controller )
        `double m_b               `( Parameter to pie controller )
        `uint32_t m_dqThreshold   `( Minimum queue size in bytes before dequeue rate is measured )
        `uint32_t m_quantum       `( Deficit assigned to flows at each round )
        `uint32_t m_flows         `( Number of flow queues )
        `uint32_t m_perturbation  `( hash perturbation value )


***

### Weekly Progress ([details](https://github.com/SUMUKHA-PK/FQ-PIE-ns-3/wiki/Weekly-Progress))


***

### Result and Analysis ([details](https://github.com/SUMUKHA-PK/FQ-PIE-ns-3/wiki/Results-and-Further))
**FQ-PIE vs FQ-CoDel vs PIE vs CoDel**


***

### Conclusion
* Fq-PIE and Fq-CoDel have comparable throughputs
* PIE and CoDel are not fair when both TCP and UDP packets are present
* Fq-PIE is however better than Fq-CoDel in case of preventing buffer bloat as it reduces the number of drops significantly compared to Fq-CoDel.

*** 

### Further
* CAKE implementation and comparison with FqPIE
* Only Basic FQ-PIE has been implemented. A enhanced version can be used for better results. [Enhanced PIE](https://tools.ietf.org/html/rfc8033#appendix-B)

***
