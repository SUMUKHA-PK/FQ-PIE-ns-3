set terminal eps enhanced color size 800,600 font 'Arial-Bold'
set output "TP-Flow.eps"
set xlabel "Time (sec)"
set ylabel "Queue Delay (Mbps)"
set xrange [2:50]
set xrange [0:50]
set key outside;
set key right top;
plot "queue0.plotme" title "S1-0" with lines lw 1.5

