Update 2-Feb-2011
Version 2.0

extensions:

a) Total scheduler workload added

Which accounts for the scheduler workload during one simulation run.

Bascially, a counter Total_Scheduler_Workload is incremented when scheduler performs an operation during the scheduling of task.

It is different from 'Total_Search_Length_Scheduler' which only accounts for the search steps taken to accommodate a task to an appropriate node.

