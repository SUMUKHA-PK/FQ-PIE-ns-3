.. include:: replace.txt
.. highlight:: cpp
.. highlight:: bash

FqPie queue disc
------------------

This chapter describes the FqPie queue disc implementation in |ns3|.

The FlowQueue-PIE (FQ-Pie) algorithm is a combined packet scheduler and
Active Queue Management (AQM) algorithm developed as part of the
bufferbloat-fighting community effort ([Buf16]_).
FqPie classifies incoming packets into different queues (by default, 1024
queues are created), which are served according to a modified Deficit Round
Robin (DRR) queue scheduluer. Each queue is managed by the PIE AQM algorithm.
FqPie distinguishes between "new" queues (which don't build up a standing
queue) and "old" queues, that have queued enough data to be around for more
than one iteration of the round-robin scheduler.


Model Description
*****************

The source code for the FqPie queue disc is located in the directory
``src/traffic-control/model`` and consists of 2 files `fq-pie-queue-disc.h`
and `fq-pie-queue-disc.cc` defining a FqPieQueueDisc class and a helper
FqPieFlow class. The code was ported to |ns3| based on Linux kernel code.


* class :cpp:class:`FqPieQueueDisc`: This class implements the main FqPie algorithm:

  * ``FqPieQueueDisc::DoEnqueue ()``: If no packet filter has been configured, this routine calls the QueueDiscItem::Hash() method to classify the given packet into an appropriate queue. Otherwise, the configured filters are used to classify the packet. If the filters are unable to classify the packet, the packet is dropped. Otherwise, it is handed over to the Pie algorithm where all the variables of the queue except m_dqCount are initialized to zero. Then, if the queue is not currently active (i.e., if it is not in either the list of new or the list of old queues), it is added to the end of the list of new queues, and its deficit is initiated to the configured quantum. Otherwise, the queue is left in its current queue list. Next, the current size of that queue is checked with the maximum size and, if the sum of the current size and the new item to be added to the queue is greater than the maximum size of the queue, then the incoming packet is dropped before enqueuing. Otherwise, the DropEarly method of Pie algorithm is called to determine whether to drop the incoming packet based on random probability or not. If the packet is not being dropped, it is enqueued.
  
  * ``FqPieQueueDisc::DropEarly ()``: If there is still burst allowance left, then random early drop is skipped. If no burst allowance, then the following conditions are checked for which random dropping is bypassed - If the queue's latency sample is smaller than half of it's target latency value when the drop probability is not too high (less than 0.2) or if the queue has 2 or less packets, the packet is not dropped. Otherwise, the incoming packet is randomly dropped.
  
  * ``FqPieQueueDisc::CalculateP ()``: The PIE algorithm periodically updates the probability based on the latency samples. The drop probability is updated as per the algorithm given in RFC 8033. /* TODO line 461 to 526*/
  
  * ``FqPieQueueDisc::DoDequeue ()``: The first task performed by this routine is selecting a queue from which to dequeue a packet. To this end, the scheduler first looks at the list of new queues; for the queue at the head of that list, if that queue has a negative deficit (i.e., it has already dequeued at least a quantum of bytes), it is given an additional amount of deficit, the queue is put onto the end of the list of old queues, and the routine selects the next queue and starts again. Otherwise, that queue is selected for dequeue. If the list of new queues is empty, the scheduler proceeds down the list of old queues in the same fashion (checking the deficit, and either selecting the queue for dequeuing, or increasing deficit and putting the queue back at the end of the list). If no queue is found for dequeuing, the method returns a message stating that. Otherwise, a packet is chosen from the selected queue to be dequeued and the deficit of the queue is updated.
  
* class :cpp:class:`FqPieFlow`: This class implements a flow queue, by keeping its current status (whether it is in the list of new queues, in the list of old queues or inactive) and its current deficit.

In Linux, by default, packet classification is done by hashing (using a Jenkins
hash function) on the 5-tuple of IP protocol, and source and destination IP
addresses and port numbers (if they exist), and taking the hash value modulo
the number of queues. The hash is salted by modulo addition of a random value
selected at initialisation time, to prevent possible DoS attacks if the hash
is predictable ahead of time. Alternatively, any other packet filter can be
configured.
In |ns3|, packet classification is performed in the same way as in Linux.


References
==========


.. [Buf16] Bufferbloat.net.  Available online at `<http://www.bufferbloat.net/>`_.


Attributes
==========

The key attributes that the FqCoDelQueue class holds include the following:

* ``MeanPktSize:`` Average of packet size.
* ``A:`` Value of alpha.
* ``B:`` Value of beta.
* ``Tupdate:`` Time period to calculate drop probability.
* ``Supdate:`` Start time of the update timer.
* ``MaxSize:`` The maximum number of packets accepted by this queue disc.
* ``DequeueThreshold:`` Minimum queue size in bytes before dequeue rate is measured.
* ``QueueDelayReference:`` Desired queue delay.
* ``MaxBurstAllowance:`` Current max burst allowance in seconds before random drop.
* ``Flows:`` The number of queues into which the incoming packets are classified.

Note that the quantum, i.e., the number of bytes each queue gets to dequeue on
each round of the scheduling algorithm, is set by default to the MTU size of the
device (at initialisation time). The ``FqPieQueueDisc::SetQuantum ()`` method
can be used (at any time) to configure a different value.

Examples
========


Validation
**********

The FqPie model is tested using :cpp:class:`FqPieQueueDiscTestSuite` class defined in `src/test/ns3tc/fq-pie-queue-disc-test-suite.cc`.  The suite includes 5 test cases:

The test suite can be run using the following commands::

  $ ./waf configure --enable-examples --enable-tests
  $ ./waf build
  $ ./test.py -s fq-pie-queue-disc

or::

  $ NS_LOG="FqPieQueueDisc" ./waf --run "test-runner --suite=fq-pie-queue-disc"
