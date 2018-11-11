# FQ-PIE-ns-3


### Resources used:
1. RFC 8033, Proportional Integral Controller Enhanced (PIE  https://tools.ietf.org/html/rfc8033

2. IEEE paper on PIE http://ieeexplore.ieee.org/document/6602305/

3. RFC 8290, Fq-Codel https://tools.ietf.org/html/rfc8290

4. Paper related  to FQ-PIE http://caia.swin.edu.au/reports/160418A/CAIA-TR-160418A.pdf

### Design Goals of PIE (As in RFC 8033):

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
      
      
