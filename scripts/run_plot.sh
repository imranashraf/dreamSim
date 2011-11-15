#! /bin/bash

currDir=`pwd`
srcDir=../src
MaxNodeConfigs=10

#####################################
# echo "Entering source dir"
# cd $srcDir
# 
# echo "Running DreamSim with Full Configuration (MAX_NODE_CONFIG = 1 ) ..."
# make cleanall clean all "MNC=1"
# make run
# 
# echo "Running DreamSim with Partial Configuration (MAX_NODE_CONFIG = $MaxNodeConfigs ) ..."
# make clean all "MNC=$MaxNodeConfigs"
# make run
# 
# echo "Making Plot ...."
# echo "Entering script dir"
# cd $currDir
######################################

tempfile=temp.txt
FullData=full.txt
PartialData=partial.txt
interval=3
xaxis=total_tasks_generated
#change the entry below for the required metric
yaxis=average_reconfiguration_count_per_node

# total_wasted_area
# average_wasted_area_per_task
# total_tasks_waiting_time
# average_task_waiting_time
# total_tasks_discarded
# total_scheduling_steps
# average_scheduling_steps_per_task
# Total_Simulation_Workload
# Total_reconfiguration_count
# average_reconfiguration_count_per_node
# total_configuration_time
# average_configuration_time_per_task

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

# 	while true; do 
#         echo pause $interval; 
#         echo reread;
#         echo replot; 
#         sleep $interval; 
#     done

	) | gnuplot -persist


