#! /bin/awk -f

/^MAX_NODE_CONFIGS/ {MAX_NODE_CONFIGS = $2}
/^total_tasks_generated/ {total_tasks_generated = $2}
/^total_PEs/ {total_PEs = $2}
/^total_configurations/ {total_configurations = $2}
/^total_simulation_time/ {total_simulation_time = $2}
/^total_tasks_completed/ {total_tasks_completed = $2}
/^total_tasks_discarded/ {total_tasks_discarded = $2}
/^total_wasted_area/ {total_wasted_area = $2}
/^average_wasted_area_per_task/ {average_wasted_area_per_task = $2}
/^total_scheduling_steps/ {total_scheduling_steps = $2}
/^average_scheduling_steps_per_task/ {average_scheduling_steps_per_task = $2}
/^total_tasks_waiting_time/ {total_tasks_waiting_time = $2}
/^average_task_waiting_time/ {average_task_waiting_time = $2}
/^total_tasks_running_time/ {total_tasks_running_time = $2}
/^average_task_running_time/ {average_task_running_time = $2}
/^total_configuration_time/ {total_configuration_time = $2}
/^average_configuration_time_per_task/ {average_configuration_time_per_task = $2}
/^Total_reconfiguration_count/ {Total_reconfiguration_count = $2}
/^average_reconfiguration_count_per_node/ {average_reconfiguration_count_per_node = $2}
/^Total_Simulation_Workload/ {Total_Simulation_Workload = $2}

END	{
	print MAX_NODE_CONFIGS "\t" total_tasks_generated "\t" total_PEs "\t" total_configurations "\t" total_simulation_time "\t" total_tasks_completed "\t" total_tasks_discarded 		"\t" total_wasted_area "\t" average_wasted_area_per_task "\t" total_scheduling_steps "\t" average_scheduling_steps_per_task "\t" total_tasks_waiting_time "\t" average_task_waiting_time "\t" total_tasks_running_time "\t" average_task_running_time "\t" total_configuration_time "\t" average_configuration_time_per_task "\t" Total_reconfiguration_count "\t" average_reconfiguration_count_per_node "\t" Total_Simulation_Workload;
	}
