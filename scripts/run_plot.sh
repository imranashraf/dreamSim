#! /bin/bash

tempfile=temp.txt
FullData=full.txt
PartialData=partial.txt

xaxis=total_tasks_generated
#change the entry below for the required metric
yaxis=average_task_waiting_time   

# average_task_waiting_time
# total_tasks_discarded
# total_wasted_area
# average_wasted_area_per_task
# total_scheduling_steps
# average_scheduling_steps_per_task
# total_tasks_waiting_time
# average_task_waiting_time
# Total_reconfiguration_count
# Total_Simulation_Workload

#remove old files
rm -f *.dsim 

#copy new files
cp ../src/*.dsim .

#remove the old file
rm -f $FullData $PartialData $tempfile

for file in *FullConfig.dsim
do
	a=`cat $file | grep $xaxis | awk '{print $2}'`
	b=`cat $file | grep $yaxis | awk '{print $2}'`
	echo -e "$a \t $b" >> $tempfile 
done

sort -n $tempfile > $FullData
rm -f $tempfile

for file in *PartialConfig.dsim
do
	a=`cat $file | grep $xaxis | awk '{print $2}'`
	b=`cat $file | grep $yaxis | awk '{print $2}'`
	echo -e "$a \t $b" >> $tempfile 
done

sort -n $tempfile > $PartialData
rm -f $tempfile

	(
	echo "set title '$yaxis' "
	echo "set xlabel '$xaxis' "
	echo "set ylabel '$yaxis' "
	echo "plot '$FullData' using 1:2  with linespoints title 'Full' , '$PartialData' using 1:2  with linespoints title 'Partial' "
	) | gnuplot -persist

