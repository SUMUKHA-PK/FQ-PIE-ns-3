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

***

### Steps to reproduce the results :

* For validation of fq-pie, it is compared to pie :
	* fq-pie-validation and pie-validation are compared
* For evaluation of fq-pie, it is compared with fq-codel :
	* fq-pie-light-tcp and fq-codel-light-tcp
	* fq-pie-mix-traffic and fq-codel-mix-traffic
	* fq-pie-heavy-tcp and fq-codel-heavy-tcp

* List of example-files :
	* fq-pie-light-tcp 
	* fq-codel-light-tcp
	* fq-pie-mix-traffic 
	* fq-codel-mix-traffic
	* fq-pie-heavy-tcp 
	* fq-codel-heavy-tcp
	* fq-pie-validation 
	* pie-validation

#### Steps to run :
* Clone the repository
* Run the following initialising code :
        
        cd ns3/
        ./waf configure --enable-examples
        ./waf build

* To run an simulation for `example-file` (Check the list of example files above) :<br/>
        
	./waf --run example-file

* If a validation script is run you have a Validation folder in your ns3 which has your example-file's sub folder containing simulation documents (cwnd traces, pcap files and queue traces) which can be analysed


* If a evaluation script is run you have a Evaluation folder in your ns3 which has your example-file's sub folder containing simulation documents (cwnd traces, pcap files and queue traces) which can be analysed

(PS: For getting results in FQPIE, one must change the directory to desired examples directory in ```fq-pie-queuedisc.cc``` )
