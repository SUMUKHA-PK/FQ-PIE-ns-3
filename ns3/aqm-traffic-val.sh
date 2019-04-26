 #!/bin/bash

mkdir ~/FqPie
counter=0
for var in 1 3 5 7 10 12 15 18 21 28 35 43 50 70 120 200 800 1200 3000 5000 7000 13000 17000 25000 35000
    do
    counter=$((counter+1))
(       NS_GLOBAL_VALUE="RngRun=$var" ./waf --run fq-pie-validation  	
) | tee -a ~/FQPIE/FQ-PIE-ns-3/ns3/Validation/FqPie/q.plotme

    mkdir ~/FqPie/$counter
    mkdir ~/FqPie/$counter/pcap
    mv ~/FQPIE/FQ-PIE-ns-3/ns3/Validation/FqPie/pcap ~/FqPie/$counter
    mv ~/FQPIE/FQ-PIE-ns-3/ns3/Validation/FqPie/q.plotme ~/FqPie/$counter
done


# counter=0
# for var in 1 3
#     do
#     counter=$((counter+1))
# (       NS_GLOBAL_VALUE="RngRun=$var" ./waf --run fq-pie-5-up  	
# ) | tee -a /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP10/q.plotme
#     mkdir /home/some_guyy/pk/FqPieTCP10/$counter
#     mkdir /home/some_guyy/pk/FqPieTCP10/$counter/pcap
#     mv /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP10/pcap /home/some_guyy/pk/FqPieTCP10/$counter
#     mv /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP10/q.plotme /home/some_guyy/pk/FqPieTCP10/$counter
# done


# counter=0
# for var in 1 3
#     do
#     counter=$((counter+1))
# (       NS_GLOBAL_VALUE="RngRun=$var" ./waf --run fq-pie-5-up  	
# ) | tee -a /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP50/q.plotme
#     mkdir /home/some_guyy/pk/FqPieTCP50/$counter
#     mkdir /home/some_guyy/pk/FqPieTCP50/$counter/pcap
#     mv /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP50/pcap /home/some_guyy/pk/FqPieTCP50/$counter
#     mv /home/some_guyy/pk/git/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP50/q.plotme /home/some_guyy/pk/FqPieTCP50/$counter
# done
