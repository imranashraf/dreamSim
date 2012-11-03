#! /bin/bash

currDir=`pwd`
srcDir=./.
MaxNodeConfigs=10
MAKEandPLOT=$1

#assign yes to the following to send the result by email
SendEmail=yes

tempfile=temp.txt
FullData=full.txt
PartialData=partial.txt
interval=3
xaxis=total_PEs

#change the entry below for the required metric
Metrics="average_wasted_area_per_task average_task_waiting_time total_tasks_discarded average_scheduling_steps_per_task Total_Simulation_Workload average_reconfiguration_count_per_node average_configuration_time_per_task"


function getLabel ()
{
case "$1" in		
'average_wasted_area_per_task')
	echo "Average wasted area per task"
	;;
"average_task_waiting_time")
	echo "Average task waiting time"
	;;
"total_tasks_discarded")
	echo "Total tasks discarded"
	;;
"average_scheduling_steps_per_task")
	echo "Average scheduling steps per task"
	;;
"Total_Simulation_Workload")
	echo "Total scheduler workload"
	;;
"average_reconfiguration_count_per_node")
	echo "Average reconfiguration count per node"
	;;
"average_configuration_time_per_task")
	echo "Average configuration time per task"
	;;
"total_PEs")
	echo "Total nodes"
	;;
"total_tasks_generated")
	echo "Total tasks generated"
	;;
"total_configurations")
	echo "Total configurations"
	;;
*)
	echo "Default"
	;;
esac
}
	
if [ "$MAKEandPLOT" == "all" ] 
then
	#echo "Entering source dir"
	#	cd $srcDir

	echo "Running DreamSim with Full Configuration (MAX_NODE_CONFIG = 1 ) ..."
	make delete all "MNC=1"
	make run

	echo "Running DreamSim with Partial Configuration (MAX_NODE_CONFIG = $MaxNodeConfigs ) ..."
	make clean all "MNC=$MaxNodeConfigs"
	make run

	echo "Making Plot ...."
	#echo "Entering script dir"
	#cd $currDir
fi

#remove old files
#rm -f *.dsim *.eps *.pdf

#copy new files
#cp ../src/*.dsim .

#remove the old file
rm -f $FullData $PartialData $tempfile

for metric in $Metrics
do
	yaxis=$metric

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
	
	xlabel=`getLabel $xaxis`
	ylabel=`getLabel $yaxis`	
		
		(
		echo "set term postscript eps enhanced color"
		echo "set output '$yaxis.eps'"
		echo "set xlabel '$xlabel' font 'Helvetica,20' "
		echo "set ylabel '$ylabel' font 'Helvetica,20' "
 		echo "set pointsize 2"
		echo "set grid linewidth 0.5"
		echo "set xtics font 'Helvetica, 18' "
		echo "set ytics font 'Helvetica, 18' "
		echo "plot '$FullData' using 1:2  with linespoints linetype  1 linewidth 3 pointtype 3 linecolor rgb 'red' title 'Without partial configuration' , '$PartialData' using 1:2  with linespoints linetype 2 linewidth 3 pointtype 5 linecolor rgb 'blue' title 'With partial configuration' "
		) | gnuplot -persist
done



# comment it if you dont need pdf files
for epsfile in *.eps
do
	epstopdf $epsfile
done

# comment the following if you dont want to pack all the eps graphs in .tar.gz
tar -czf allepsgraphs.tar.gz *.eps

# comment the following if you dont want to pack all the pdf graphs in .tar.gz
tar -czf allpdfgraphs.tar.gz *.pdf

# comment the following if you dont want to pack all the dsim result files in .tar.gz
tar -czf alldsimfiles.tar.gz *.dsim

# comment it if you dont want to delete eps files
rm -f *.eps

# comment it if you dont want to delete pdf files
rm -f *.pdf

# Following is responsible to send the result files, eps and pdf by email
if [ "$SendEmail" == "yes" ] 
then
	echo "Sending Email ..."
	
	echo -e "Contains \n 	\n \
			1) 	dsim files \n   \
			2) 	eps graphs \n    \
			3)	pdf graphs \n " | \
		mail \
 		-a alldsimfiles.tar.gz \
 		-a allepsgraphs.tar.gz \
 		-a allpdfgraphs.tar.gz \
		-s "Dreamsim Simulation Automatic Email ABCD Policy Variable Nodes [20-1000] step size 20" \
		iimran.aashraf@gmail.com \
 		faisal.jaan@gmail.com
fi

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


	# 	while true; do 
	#         echo pause $interval; 
	#         echo reread;
	#         echo replot; 
	#         sleep $interval; 
	#     done

	# case "$1" in		
	# 'average_wasted_area_per_task')
		# xlabel="Average Wasted Area Per Task"
		# ;;
	# "average_task_waiting_time")
		# xlabel="Average Task Waiting Time"
		# ;;
	# "total_tasks_discarded")
		# xlabel="total_tasks_discarded"
		# ;;
	# "average_scheduling_steps_per_task")
		# xlabel="average_scheduling_steps_per_task"
		# ;;
	# "Total_Simulation_Workload")
		# xlabel="Total_Simulation_Workload"
		# ;;
	# "average_reconfiguration_count_per_node")
		# xlabel="average_reconfiguration_count_per_node"
		# ;;
	# "average_configuration_time_per_task")
		# xlabel="average_configuration_time_per_task"
		# ;;
		
	# "total_PEs")
		# xlabel="Total Nodes"
		# echo "Total Nodes"
		# ;;
	# "total_tasks_discarded")
		# xlabel="total_tasks_discarded"
		# echo "total_tasks_discarded"
		# ;;
	# "total_tasks_discarded")
		# xlabel="total_tasks_discarded"
		# echo "total_tasks_discarded"
		# ;;
	# *)
		# xlabel="Default"
		# echo "Default"
		# ;;
	# esac
