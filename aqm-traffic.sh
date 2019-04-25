 #!/bin/bash

#for scenario in LbeTransportSender UnresponsiveTransport UnresponsiveWithFriendly TCPFriendlyDifferentInitCwnd TCPFriendlySameInitCwnd HeavyCongestion MediumCongestion MildCongestion VaryingCongestion AggressiveTransportSender
#LbeTransportSender UnresponsiveTransport UnresponsiveWithFriendly TCPFriendlyDifferentInitCwnd TCPFriendlySameInitCwnd HeavyCongestion MediumCongestion MildCongestion VaryingCongestion

for scenario in UnresponsiveTransport UnresponsiveWithFriendly HeavyCongestion MediumCongestion MildCongestion VaryingCongestion
do
	for var in 1 3 5 7 10 12 15 18 21 28 35 43 50 70 120 200 800 1200 3000 5000 7000 13000 17000 25000 35000
	    do
	       NS_GLOBAL_VALUE="RngRun=$var" ./waf --run "aqm-eval-suite-runner --name=$scenario"
	       mkdir  "./aqm-eval-output/$scenario/rng$var/"  	
	       mv /home/sdp/Documents/ns-3-dev-git-master/aqm-eval-output/$scenario/data/ /home/sdp/Documents/ns-3-dev-git-master/aqm-eval-output/$scenario/rng$var/ 
	done

topdir=/home/sdp/Documents/ns-3-dev-git-master/aqm-eval-output/$scenario
dir1=rng1
for f in $topdir/$dir1/MinstrelPieQueueDisc-result.dat
do
    outf=$topdir/`basename $f .dat`
    cp $f $outf
for dir in rng1 rng3 rng5 rng7 rng10 rng12 rng15 rng18 rng21 rng28 rng35 rng43 rng50 rng70 rng120 rng200 rng800 rng1200 rng3000 rng5000 rng7000 rng13000 rng17000 rng25000 rng35000
do	 
    sed -e '' $topdir/$dir/`basename $f` >> $outf
done
done
for f in $topdir/$dir1/PieQueueDisc-result.dat
do
    outf=$topdir/`basename $f .dat`
    cp $f $outf
for dir in rng1 rng3 rng5 rng7 rng10 rng12 rng15 rng18 rng21 rng28 rng35 rng43 rng50 rng70 rng120 rng200 rng800 rng1200 rng3000 rng5000 rng7000 rng13000 rng17000 rng25000 rng35000
do	 
    sed -e '' $topdir/$dir/`basename $f` >> $outf
done
done

echo "set terminal postscript eps color font 20 
set output \"$topdir/$scenario.eps\"
set xlabel \"Queue Delay (ms)\"
set ylabel \"Goodput (Mbps)\"
set xrange[] reverse
set yrange[]
set grid
show grid
set key left top horizontal" >> $topdir/plot-shell

for aqm in Pie MinstrelPie
do
./ellipsemaker $scenario $aqm
done
echo "plot \"$topdir/PieQueueDisc-ellipse.dat\" title 'PIE' with lines lw 2,\"$topdir/MinstrelPieQueueDisc-ellipse.dat\" title 'MinstrelPIE' with lines lw 2 lc 'magenta'" >> $topdir/plot-shell
gnuplot $topdir/plot-shell
done

