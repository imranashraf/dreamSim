#!/home/iashraf/bin/bash

rootdir=`pwd`
summaryfile=$rootdir/summary.txt

runlists=`ls *_PartialConfig.txt`

# Write the header
# print MAX_NODE_CONFIGS "\t" total_tasks_generated "\t" total_PEs "\t" total_configurations "\t" total_simulation_time "\t" total_tasks_completed "\t" total_tasks_discarded 		"\t" total_wasted_area "\t" average_wasted_area_per_task "\t" total_scheduling_steps "\t" average_scheduling_steps_per_task "\t" total_tasks_waiting_time "\t" average_task_waiting_time "\t" total_tasks_running_time "\t" average_task_running_time "\t" total_configuration_time "\t" average_configuration_time_per_task "\t" Total_reconfiguration_count "\t" average_reconfiguration_count_per_node "\t" Total_Simulation_Workload;
echo -ne MAX_NODE_CONFIGS  "\t" total_tasks_generated "\t" total_PEs "\t" total_configurations "\t" total_simulation_time "\t" total_tasks_completed "\t" total_tasks_discarded "\t" total_wasted_area "\t" average_wasted_area_per_task "\t" total_scheduling_steps "\t" average_scheduling_steps_per_task "\t" total_tasks_waiting_time "\t" average_task_waiting_time "\t" total_tasks_running_time "\t" average_task_running_time "\t" total_configuration_time "\t" average_configuration_time_per_task "\t" Total_reconfiguration_count "\t" average_reconfiguration_count_per_node "\t" Total_Simulation_Workload "\n\n" > $summaryfile

for runlist in $runlists
do
	awk -f ./awkprog.awk $runlist >> $summaryfile
done

