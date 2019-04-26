 #!/bin/bash

mkdir ~/FqPieTCP5
counter=0
for var in 1 3 5 7 10 12 15 18 21 28 35 43 50 70 120 200 800 1200 3000 5000 7000 13000 17000 25000 35000
    do
    counter=$((counter+1))
(       NS_GLOBAL_VALUE="RngRun=$var" ./waf --run fq-pie-5-up  	
) | tee -a ~/FQPIE/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP5/q.plotme
    mkdir ~/FqPieTCP5/$counter
    mkdir ~/FqPieTCP5/$counter/pcap
    mv ~/FQPIE/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP5/pcap ~/FqPieTCP5/$counter
    mv ~/FQPIE/FQ-PIE-ns-3/ns3/Evaluation/FqPieTCP5/q.plotme ~/FqPieTCP5/$counter
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
