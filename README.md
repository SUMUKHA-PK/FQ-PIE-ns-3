# FQ-PIE-ns-3

***

## Resources used:
1. RFC 8033, Proportional Integral Controller Enhanced (PIE  https://tools.ietf.org/html/rfc8033

2. RFC 8290, Fq-Codel https://tools.ietf.org/html/rfc8290

3. NS3 documentation (Doxygen), https://www.nsnam.org/doxygen/


***


## Implementation Details ([detailed Wiki](https://github.com/SUMUKHA-PK/FQ-PIE-ns-3/wiki/Implementation-progress-updates))

## Implementation of FQ-PIE was done in NS3

### Files

* **fq-pie-queue-disc.h** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/model/fq-pie-queue-disc.h)) <br/> 
This file contains the basic definition of the the FqPieFlow class and FqPieQueueDisc class.

* **fq-pie-queue-disc.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/model/fq-pie-queue-disc.cc)) <br/> 
This file contains all the declarations of the functions in **fq-pie-queue-disc.h**

* **fq-pie-example-tcp.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/examples/fq-pie-example-tcp.cc)) 
<br/>
This file creates an environment where only TCP packets are in the network

* **fq-pie-example-udp.cc** ([Click here](https://raw.githubusercontent.com/SUMUKHA-PK/FQ-PIE-ns-3/master/ns3/src/traffic-control/examples/fq-pie-example-udp.cc)) 
<br/>
This file creates an environment where only TCP and UDP packets are in the network

### Algorithm (important functions)

* **doenqueue :**<br/>
Whenever a packet comes into the node (maybe a router), this function is called.<br />
For each flow of packet a hash function is used to determine where the packet which flow (or slot) the packet will be added to, There might be hash collisions possible. <br />
If the hash value `h` is not present in the list of flows, a new flow is created with a new queue disc `qd`, else the flow which is already present is selected. <br />
If flow status is INACTIVE then the flow status is set to NEW_FLOW and a quantum deficit is added <br />
Then pie algorithm is applied as mentioned in the Resources.
* **dodequeue: ** <br />
Here the DRR algorithm is used in order to determine which queue has to dequeue. While checking for flows to dequeue, new flows are checked first. A flow is rendered old right after its checked for a packet to DQ. If there are no appropriate packets in new flow, an old flow is considered.
* **calculatePFlow: ** <br />
`Simulator::Schedule` is used to schedule this function every mt_Update unit of time. It calls another function `calculateP` which performs the PIE variant of calculation of probability except that this is done to every flow `ACTIVE`.
