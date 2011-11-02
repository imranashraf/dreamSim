#!/usr/bin/gnuplot -persist

set autoscale
set xlabel "Nodes"
set ylabel "Wasted Area"
#set xrange [0:50]
#set yrange [0:100]
#set style data histogram
#set style histogram cluster gap 1
#set style fill solid border -1
#set boxwidth 0.9
set title "Plot of Wasted Area" 

plot 	"summary.txt" using 3:8 with linespoints



#title "A"
#replot	"summary.txt" using 1:3 title "B" 

#plot 	"data.txt" using 2:1 with boxes title "A" 
#replot	"data.txt" using 3:1 with boxes title "B" 

#plot 	"data.txt" using 1:2 with boxes title "A" fs pattern 1 , \
#	"data.txt" using 1:3 with boxes title "B" fs pattern 2

