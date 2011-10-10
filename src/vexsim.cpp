// VexSim v1.0
// This is software simulator for task scheduling on Vex multiprocessor systems
// written by Arash Ostadzadeh
// ostadzadeh@gmail.com
#include <stdio.h>
#include "vexsim.h"

using namespace std;

VexSim::VexSim(unsigned int TN,unsigned int TC, unsigned long int TT, unsigned int NextTaskMaxInterval,
			   unsigned int NlowA, unsigned int NhighA,
			   unsigned int TlowA, unsigned int ThighA,
		       unsigned int TRTlow, unsigned int TRThigh,
			   unsigned int ConfTmL , unsigned int ConfTmH,
			   unsigned int NWDH, unsigned int NWDL)
{
	TotalNodes=TN;
	TotalConfigs=TC;
	TotalTasks=TT;

	NodelowA=NlowA;
	NodehighA=NhighA;

	TasklowA=TlowA;
	TaskhighA=ThighA;

	TaskReqTimelow=TRTlow;
	TaskReqTimehigh=TRThigh;

	VexSim::NextTaskMaxInterval=NextTaskMaxInterval;
	
	TotalCompletedTasks=0;
	TotalCurGenTasks=0;
	TotalCurSusTasks=0;
	TotalDiscardedTasks=0;
	
	suspendedlist=NULL; // no task initially in the suspension list

	// for report
	Total_Wasted_Area=0;
	Total_Search_Length_Scheduler=0; 	// It accounts only for the steps taken by the scheduler to accommodate tasks
										// (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
	Total_Scheduler_Workload=0; 		//Scheduler workload during one simulation run
	Total_Task_Wait_Time=0;	
	Total_Tasks_Running_Time=0;
	Total_Configuration_Time=0;
	
	ConfigTimeHigh 	= ConfTmH;
	ConfigTimeLow 	= ConfTmL;
	NWDHigh = NWDH;
	NWDLow = NWDL;
	
	TimeTick=0;
	
	InitNodes();
	InitConfigs();
	
}

void VexSim::InitNodes()
{
	int i;
	Node *node;
	
	blanklist=new Node*[TotalNodes];
	if(!blanklist) { cerr<<"\nError in memory allocation.\n"; exit(1);}
	
	CurBlankNodeIndex=0;
	//nodesList=NULL; // no node initially in the already configured linked list
	
	for(unsigned int i=0;i<TotalNodes;i++)
    {

		node=new Node;
		if(!node) { cerr<<"\nError in memory allocation.\n"; exit(1);}
		
		node->ConfigCount=0; //counter for the no of configurations for this node
		blanklist[i]=node;
		node->NodeNo=i+1;  // The Node numbers are starting from 1.
		node->TotalArea=(x.rand_int31()%(NodehighA-NodelowA+1))+NodelowA;
		node->AvailableArea=node->TotalArea;
		node->NetworkDelay = (x.rand_int31()%(NWDHigh-NWDLow+1))+NWDLow;
		node->ConfigNo=0;  // The node initial is blank
		node->Bnext=NULL;
		node->Inext=NULL;
		
		node->NodeTasks=0;	//initially there are no current tasks assigned to this node
		// node->Tasks=NULL; 
		// node->Tasks->next=NULL;
		
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
}

void VexSim::InitConfigs()
{
	int i;
	
	configs=new Config[TotalConfigs+1];
	if(!configs) { cerr<<"\nError in memory allocation.\n"; exit(1);}

	for(unsigned int i=0;i<TotalConfigs+1;i++)
	{
		configs[i].ConfigNo=i;  // ConfigNo are beginning from 1.
		
		configs[i].ConfigTime = (x.rand_int31()%(ConfigTimeHigh-ConfigTimeLow+1))+ConfigTimeLow;
		
    	configs[i].idle=configs[i].busy=NULL;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
    }
}

void VexSim::AddNodeToIdleList(Node *node)
{
	// update the idle list for the current node/config
	// add the node to the current idle list at the starting point

	node->Inext=configs[node->ConfigNo].idle;
	configs[node->ConfigNo].idle=node;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::AddNodeToBusyList(Node *node)
{
	// update the busy list for the current node/config
	// add the node to the current busy list at the starting point

	node->Bnext=configs[node->ConfigNo].busy;
	configs[node->ConfigNo].busy=node;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::RemoveNodeFromIdleList(Node *node) 
{
	Node * temp;
	
	if(configs[node->ConfigNo].idle)
		temp=configs[node->ConfigNo].idle;
	else
		return;
	
	cout<<"\n Removing node "<<node->NodeNo<<" from idle list "<<endl;
	
	if (temp==node) 
		configs[node->ConfigNo].idle=temp->Inext;
	else
	{
		while(temp->Inext != NULL) 
			if(temp->Inext!=node)	
				temp=temp->Inext;
			
		if(temp->Inext->Inext)
			temp->Inext=temp->Inext->Inext;
		else
			cout<<"\n temp->Inext->Inext is NULL"<<endl;
			
	}
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::RemoveNodeFromBusyList(Node *node)
{
	Node * temp;
	
	if(configs[node->ConfigNo].busy)
		temp=configs[node->ConfigNo].busy;
	else
		return;
	
	cout<<"\n Removing node "<<node->NodeNo<<" from busy list "<<endl;	
	
	if (temp==node) configs[node->ConfigNo].busy=temp->Bnext;
	else
	{
		while(temp->Bnext && temp->Bnext!=node)	temp=temp->Bnext;
		temp->Bnext=temp->Bnext->Bnext;
	}
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	


void VexSim::TaskCompletionProc(Node* curNode , Task *task)
{
	//print task report summary
	cout<<"\n Inside Task Completion Proc"<<endl;
	
	cout<<"Node # "<< curNode->NodeNo << " finished executing Task # "<< task->TaskNo <<endl;
	
	cout<<"Starting time: "<< task->StartTime <<"     CompletionTime= "<< task->CompletionTime <<endl;
	
	cout<<"elapsed time ticks: " << task->RequiredTime <<endl;
	
	if (task->SusRetry) 
		cout<<"The task had been initially put in the suspension queue. "
		    << task->SusRetry << " retries have been carried out before accommodation\n";
	
	//update report statistics
	TotalCompletedTasks++;
	Total_Tasks_Running_Time+=task->RequiredTime;
	
	if(curNode->NodeTasks == 0)
	{
		// remove the node from current busy list, if no task is running on this node
		RemoveNodeFromBusyList(curNode);
		// add the node to the current idle list
		AddNodeToIdleList(curNode);
	}
	
	// delete curNode->Tasks;
	// curNode->Tasks=NULL;// current node becomes available	
	removeTaskFromNode(curNode,task);
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::SendTaskToNode(Task *task, Node *node)
{
	unsigned int conftime;
	
	cout<<"\n Sending Task to node"<<endl;
	
	cout<<"Node # "<< node->NodeNo << " has started executing task # "<< task->TaskNo << endl;
	cout<<"Task creation time: "<< task->CreateTime <<endl;
	task->StartTime=TimeTick;
	task->CompletionTime= TimeTick + task->RequiredTime;
	
	cout<<"start time: "<< task->StartTime <<"     completion time: "<< task->CompletionTime << endl;
	
	if( addTaskToNode(node,task) == false )
	{
		cout<<"\n Cannot assign task to node "<<endl;
		exit(1);
	}
	
	// remove the node from current idle list
	RemoveNodeFromIdleList(node);
	// add the node to the current busy list
	AddNodeToBusyList(node);
	
	conftime = configs[task->AssignedConfig].ConfigTime;
	
	// update some report statistics
	Total_Configuration_Time+= conftime;
	Total_Wasted_Area+= node->TotalArea - node->AvailableArea;
	Total_Task_Wait_Time += ( task->StartTime - task->CreateTime) + conftime; 
						// basically start time is the time for starting the configuration process, 
						// i.e we have identified optimal/preffered/... node and we want to configure it
						// so configuration will also take some time depending upon different factors
						// this mean that actual waiting time will be affected by config time as well
						// thats why its added here
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

Task * VexSim::CreateTask()
{
	Task *task;
	
	TotalCurGenTasks++; // new task is going to be created
	task=new Task;
	if (!task) { cerr<<"\nError in memory allocation for Task # " << TotalCurGenTasks <<"\n"; exit(1);}
	
	task->TaskNo=TotalCurGenTasks;
	task->NeededArea=(x.rand_int31()%(TaskhighA-TasklowA+1))+TasklowA;
	
	// we will assume about 10% of the created tasks preferring a configuration which is not available in the system
	// the criterion can be changed later
	task->PrefConfig=1 + ( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) ); 
	
	task->CreateTime=TimeTick;
	task->RequiredTime=(x.rand_int31()%(TaskReqTimehigh-TaskReqTimelow+1))+TaskReqTimelow;
	task->SusRetry=0;
	
	cout<<"Task # " << TotalCurGenTasks << " submitted to the system at time # " << TimeTick << endl;
	// can print other information about the task here
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	
	return task;
}

Task * VexSim::CheckSuspensionQueue(Node *curNode)
{
	SusList* temp=suspendedlist;
	// the search for a match on the suspension queue will terminate as soon as a matched node with the same config and available area found
	// could be a point for further improvement (a more intelligent match making to optimize waste area ,...)
	
	SusList* prev_temp=NULL; // this always precedes temp
	
	while(temp) 
	{
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		if (curNode->ConfigNo==temp->item->AssignedConfig && curNode->AvailableArea >= temp->item->NeededArea) 
		 // a match is found
		{
			Task * task=temp->item;
			
			//update the suspended tasks list
			if (!prev_temp) // first node to be removed from suspension queue
				suspendedlist=temp->next;
			else 
				prev_temp->next=temp->next;
			
			delete temp;
			Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.			
			return task;
			
		}
		
		temp->item->SusRetry++; // this task was agained tested to be assigned to a node without success
								// the info looks stupid for now but can be useful later. for now the list is checked on every time tick (crazy)!!!!
		prev_temp=temp;
		temp=temp->next;
	}
	
	return NULL; // no suitable task found the recently released node
}

Config* VexSim::findPreferredConfig(Task *task)
{
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	
	// for now just a stupid straightforward check!	
	if (task->PrefConfig <= TotalConfigs) 
		return &configs[task->PrefConfig];
		
	return NULL; // no exact match
}


Config* VexSim::findClosestConfig(Task *task)
{
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	// for now just a random config # will be picked up as the closest match
	return &configs[(x.rand_int31()%TotalConfigs)+1];
}
	
	
Node* VexSim::findAnyIdleNode(Task* task,unsigned long long int& SL)
{
 	unsigned long int c=0;
 	// trying to find any (first) available idle node with any kind of configuration
 	
	while( c < CurBlankNodeIndex)
 	{
 		if ( (blanklist[c]->NodeTasks < MAX_NODE_TASKS) && blanklist[c]->AvailableArea >= task->NeededArea) 
			return blanklist[c];
			
  		c++;
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
 	}
	return NULL;
}

Node* VexSim::findBestBlankNodeMatch(Task* task,unsigned long long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff;
 	signed long int tempArea;
 	signed long int counter=CurBlankNodeIndex;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(counter<TotalNodes)			// find the first suitable node
 	{
 		if (blanklist[counter]->TotalArea >= task->NeededArea) 
 		{
 			bestMatchindex=counter;
 			mindiff=blanklist[counter]->TotalArea - task->NeededArea;
 			break;
 		}
  		counter++;
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
 	}
	
	if(bestMatchindex==-1) return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		tempArea=blanklist[counter]->TotalArea - task->NeededArea;
		
 		if ( (tempArea>=0) && ( mindiff > tempArea ) ) 
 		{
 			bestMatchindex=counter;
 			mindiff=tempArea;
 		}
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}	
	// swap the node found at bestmatch and the node residing on the CurBlankNodeIndex to keep the already assigned nodes and blank nodes in two
	// separate parts

	Node * tempNode=blanklist[CurBlankNodeIndex];
	blanklist[CurBlankNodeIndex]=blanklist[bestMatchindex];
	blanklist[bestMatchindex]=tempNode;
	
	CurBlankNodeIndex++;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	return blanklist[CurBlankNodeIndex-1];
}

	
Node* VexSim::findBestNodeMatch(Task* task,Node *idlelist,unsigned long long int& SL)
{
	Node * bestMatch=NULL;
 	unsigned long int mindiff;
 	signed long int temp;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(idlelist)			// find the first suitable node
 	{
 		if (idlelist->TotalArea >= task->NeededArea) 
 		{
 			bestMatch=idlelist;
 			mindiff=idlelist->TotalArea - task->NeededArea;
 			break;
 		}
  		idlelist=idlelist->Inext;
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
 	}
	
	if(!bestMatch) return NULL;
	
	while(idlelist)	// check the remaining nodes to find a better match
	{
		temp=idlelist->TotalArea - task->NeededArea;
		
 		if ( (temp>=0) && ( mindiff > temp ) ) 
 		{
 			bestMatch=idlelist;
 			mindiff=temp;
 		}
  		idlelist=idlelist->Inext;
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}	

	return bestMatch;
}



void VexSim::makeNodeBlank(Node *node)
{
	cout<<"making Node # " << node->NodeNo <<" blank"<<endl;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}


void VexSim::sendBitstream(Node *node)
{
	(node->ConfigCount)++;
	cout<<"sending bitstream for configuration # " << node->ConfigNo 
		<< " to the Node # " <<node->NodeNo
		<< " reconfiguration count for this node = "<<node->ConfigCount<<endl;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

bool VexSim::queryBusyListforPotentialCandidate(Task *task, unsigned long long int& SL )
{
	Node *tempBusyNode=configs[task->AssignedConfig].busy;
	
	while(tempBusyNode)
	{
		if( tempBusyNode->TotalArea >= task->NeededArea ) return true;
		tempBusyNode=tempBusyNode->Bnext;
		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
	
	return false;
}

void VexSim::PutInSuspensionQueue(Task *task)
{
	SusList* cur;
	
	cur=new SusList;
	if(!cur) { cerr<<"\nError in memory allocation for suspended task list.\n"; exit(1);}
	
	cout<<"puting the task # "<<task->TaskNo<<" on the suspended tasks queue\n";
	cur->item=task;
	cur->next=suspendedlist;
	suspendedlist=cur;
	TotalCurSusTasks++;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::DiscardTask(Task *task)
{
	cout<<"Task # "<<task->TaskNo<<" can not be accommodated by the scheduler. Discarding the task!"<<endl;
	TotalDiscardedTasks++;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::RunVexScheduler(Task *task)
{
	Config * Cmatch,* Closestmatch;
	bool found=false;
	unsigned long long int SL=0;
	Node *node;
	
	Cmatch=findPreferredConfig(task);
	
	if (Cmatch) // required configuration was found in the list (exact match)
	{
		task->AssignedConfig = Cmatch->ConfigNo; // set the assigned config for the current task
		
		if(Cmatch->idle) // there are idle nodes available with the preferred config
		{
			node=findBestNodeMatch(task,Cmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
			Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		
			if (node) 
			{	
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(task,node);  // found a suitable node
				found=true;
			}
		}
		if(!found) // no suitable idle node found at this point
		{
			if(CurBlankNodeIndex < TotalNodes) // blank node(s) still available
			{
				node=findBestBlankNodeMatch(task,SL);
				if (node)
				{
					Total_Search_Length_Scheduler+=SL;
					node->ConfigNo=Cmatch->ConfigNo; // change the ConfigNo of the blank node from 0 to the configuration match
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(node);
					
					AddNodeToIdleList(node); // this is something stupid!!! extra here because the node was initially blank we first add it to the idle
									  // list of corresponding config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(task,node);
					found=true;					
				}
			}// end of blank node(s) still available
			
			if(!found) // no blank node available or there is no suitable blank node available!!!
			{			// try reconfiguring one of the idle nodes!
				node=findAnyIdleNode(task,SL);
				if (node) // An idle node is found for reconfiguration
				{
					Total_Search_Length_Scheduler+=SL;
					RemoveNodeFromIdleList(node);
					makeNodeBlank(node);
					node->ConfigNo=Cmatch->ConfigNo; // change the ConfigNo of the idle node to the configuration match
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(node);
					
					AddNodeToIdleList(node); // this is something stupid!!! extra here because the node is not in the idle
									  // list of new config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(task,node);
					found=true;
				}
			}// end of doing reconfiguration for one of the idle nodes
		}// exact match found / but the was no idle nodes available
		//--------------------------------------------------------------------------------------------------------
		// if you want to wait for a busy node with exact match to become available this is the point to add the rountine!!!
		//---------------------------------------------------------------------------------------------------------
	
	}// end of exact match for the configuration

	if(!found) // no exact match! or we have exact match but can not accommodate in any way the current task at this moment!
	{
		Closestmatch=findClosestConfig(task); // we assume that there is always a closest config match
											// for now because we actually do not look for closest match the SL is not increased,
											// in reality SL is also increased correspondingly to indicate the search effort
		task->AssignedConfig=Closestmatch->ConfigNo; // set the assigned config for the current task
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		
		if(Closestmatch->idle) // there are idle nodes available with the closest config
		{
			node=findBestNodeMatch(task,Closestmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
		
			if (node) 
			{	
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(task,node);  // found a suitable node
				found=true;
			}
		}
		if(!found) // no (suitable) idle nodes available
		{
			// in case we are coming from an exact match configuration part, we have already searched the blank list and no need for a re-search
			if( !Cmatch	&& CurBlankNodeIndex<TotalNodes) // blank node(s) still available
			{
				node=findBestBlankNodeMatch(task,SL);
				if (node)
				{
					Total_Search_Length_Scheduler+=SL;
					node->ConfigNo=Closestmatch->ConfigNo; // change the ConfigNo of the blank node from 0 to the closest configuration match
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(node);
					
					AddNodeToIdleList(node); // this is something stupid!!! extra here because the node was initially blank we first add it to the idle
									  // list of corresponding config and then next function will add it to the busy list of that config, just
									  // because it has a function which initially tries to remove the node from idle list!
					SendTaskToNode(task,node);
					found=true;
				}
			}// end of blank node(s) still available
		}
		
		if(!found) // no blank node available or there is no suitable blank node available!!!
			     	   // we are going to wait for a busy node to become idle!
						// The last solution!
		{
				if(Cmatch) 
					task->AssignedConfig=Cmatch->ConfigNo; // adjust the assigned config field again if needed, in case there was previously an exact match
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
				
				found=queryBusyListforPotentialCandidate(task,SL); // determine whether or not the current busy list has any potential candidate for 
																// accommodating the task regarding the Area restriction
				Total_Search_Length_Scheduler+=SL;
																				
				if( found )   // if yes
					PutInSuspensionQueue(task);

				// we can also go for another check with all busy nodes and do a reconfig later if needed, not considered in this scheduler!!!
				else DiscardTask(task); // bad task, throw it away! 
		} // end of waiting!
	}// end of no exact match or we had exact match but were not able to accommodate on exact config list
}

void VexSim::MakeReport()
{
	ofstream outfile;
	sprintf (fileName, "%d", TotalTasks);
	strcat(fileName, ".txt");
	outfile.open(fileName);
	if (outfile.fail()) { cout<<"\n failed opening the simulation report file.\n"; exit(1); }
	  
	
	outfile<<"total_tasks_generated\t"<<TotalTasks<<endl;
	outfile<<"total_PEs\t"<<TotalNodes<<endl;
	outfile<<"total_configurations\t"<<TotalConfigs<<endl;
	outfile<<"total_used_PEs\t"<<CurBlankNodeIndex<<endl;
	outfile<<"total_simulation_time\t"<<TimeTick<<endl;


	outfile<<"task_generation_interval\t[ 1 ... "<<NextTaskMaxInterval<<" ]"<<endl;
	outfile<<"PE_available_area_range\t[ "<<NodelowA<<" ... "<<NodehighA<<" ]"<<endl;
	outfile<<"task_required_area_range\t[ "<<TasklowA<<" ... "<<TaskhighA<<" ]"<<endl;
	outfile<<"task_required_timeslice_range\t[ "<<TaskReqTimelow<<" ... "<<TaskReqTimehigh<<" ]"<<endl;
	
	outfile<<"total_tasks_completed\t"<<TotalCompletedTasks<<endl;
	outfile<<"total_tasks_discarded\t"<<TotalDiscardedTasks<<endl;
	outfile<<"total_wasted_area\t"<<Total_Wasted_Area<<endl;
	outfile<<"average_wasted_area_per_task\t"<<(Total_Wasted_Area)/(double)(TotalCompletedTasks)<<endl;
	
	outfile<<"total_scheduling_steps\t"<<Total_Search_Length_Scheduler<<endl;
	outfile<<"average_scheduling_steps_per_task\t"<<(Total_Search_Length_Scheduler)/(double)(TotalTasks)<<endl;
	
	outfile<<"total_tasks_waiting_time\t"<<Total_Task_Wait_Time<<endl;
	outfile<<"average_task_waiting_time\t"<<(Total_Task_Wait_Time)/(double)(TotalCompletedTasks)<<endl;
	
	outfile<<"total_tasks_running_time\t"<<Total_Tasks_Running_Time<<endl;
	outfile<<"average_task_running_time\t"<<(Total_Tasks_Running_Time)/(double)(TotalCompletedTasks)<<endl;
	
	outfile<<"total_configuration_time\t"<<Total_Configuration_Time<<endl;
	outfile<<"average_configuration_time_per_task\t"<<(Total_Configuration_Time)/(double)(TotalCompletedTasks)<<endl;
	outfile<<"Total_reconfiguration_count\t"<<TotalConfigCount()<<endl;
	outfile<<"average_reconfiguration_count_per_node\t"<<TotalConfigCount()/(double)TotalNodes<<endl;
	outfile<<"Total_Scheduler_Workload\t"<<Total_Scheduler_Workload<<endl;
	
	
	outfile.close();
}

void VexSim::Start()
{
	unsigned long long int nextIncomTaskTimeTick=0;
	Task* task;
	
	while( ( TotalCompletedTasks + TotalDiscardedTasks ) < TotalTasks )  // still there are tasks which are not finished
	{
		cout<<"\n TotalCompletedTasks "<<TotalCompletedTasks<<endl;
		cout<<"\n TotalDiscardedTasks "<<TotalDiscardedTasks<<endl;
		cout<<"\n TotalTasks "<<TotalTasks<<endl;
		
		nextIncomTaskTimeTick=TimeTick + 1 + (x.rand_int31() % NextTaskMaxInterval);
		IncreaseTimeTick();  // advance one time tick
		
		// can be improved later because there is actually no need to check on every time tick
		while(TimeTick<=nextIncomTaskTimeTick) // check for completed tasks first then check whether or not suspended tasks could be accomodated
		{
			for(unsigned int i=0;i<CurBlankNodeIndex;i++,Total_Scheduler_Workload++) 
				if(blanklist[i]->NodeTasks > 0) // there is a task running at this node
				{
					for(int k = 0 ; k < blanklist[i]->NodeTasks ; k++) 	//we will check all the tasks running on this node
																		//as there can be multiple tasks
					{
						cout<<"\n reached here in k, k = "<<k<<endl;
						cout<<"\n blanklist[i]->NodeTasks = "<<blanklist[i]->NodeTasks<<endl;
						cout<<"\n TimeTick = "<<TimeTick<<endl;
						cout<<"\n (blanklist[i]->Tasks[k]).CompletionTime = "<<(blanklist[i]->Tasks[k]).CompletionTime<<endl;
						if( (blanklist[i]->Tasks[k]).CompletionTime == TimeTick) 	//if this task is going to complete at this moment
																				//then perform task termination housekeeping for this task
						{
							Task *tsq;
							cout<<"\n reached here inside if before taskcompletionproc "<<endl;
							TaskCompletionProc( blanklist[i], &(blanklist[i]->Tasks[k]) );
							tsq=CheckSuspensionQueue(blanklist[i]);
							if (tsq) // found a task in suspension queue to accomodate
							{
								cout<<"removing task # "<< tsq->TaskNo <<" from suspension queue\n";
								// send the task to the recently released node
								SendTaskToNode(tsq,blanklist[i]);
								TotalCurSusTasks--; 
							}
						}
						// cout<<endl<<"press any key to continue "<<endl;
						// getchar();
					}
				}
			IncreaseTimeTick();  // advance one time tick
		}// end of while ( TimeTick<=nextIncomTaskTimeTick )
		
		DecreaseTimeTick();  // the time needs to be adjusted just for the last unsuccessful increase in the TimeTick, otherwise we miss
							//  one of the ticks here! Note that the current TimeTick at this point is one unit ahead of the actual value

		cout<<"\n (in Start) Reached here "<<endl;
							
		//create the new scheduled task
		if ( TotalCurGenTasks < TotalTasks ) // still we need to generate more tasks!
		{
			task=CreateTask();
			//send the created task to the scheduler 
			RunVexScheduler(task);
		}
	}// main loop of the simulation
	
	MakeReport(); 	// end of the simulation, make the final report
}

unsigned long VexSim::TotalConfigCount()
{
	unsigned int i;
	unsigned long Total=0;
	
	for(i=0;i<TotalNodes;i++)
    {
		Total += blanklist[i]->ConfigCount; //each node has the info of its reconfig count
											//secondly, when a node is created, its registered in blanklist,
											//so a loop traversal on this blanklist for the total no
											//of nodes is done to the total configuration count
		//just for testing
// 		cout<<endl<<"Network delay of node ["<<i<<"] = "<<blanklist[i]->NetworkDelay;
											
	}
	return Total;
}

bool addTaskToNode(Node *node, Task *task)
{
	cout<<"\n Adding task "<<task->TaskNo<<" to node "<<node->NodeNo<<endl;
	if( (node->NodeTasks) < MAX_NODE_TASKS )
	{
		node->Tasks[node->NodeTasks] = *task;
		node->AvailableArea -= task->NeededArea;
		(node->NodeTasks)++;
		return true;
	}
	else
		return false;
}

bool removeTaskFromNode(Node *node, Task *task)
{
	cout<<"\n Removing task "<<task->TaskNo<<" to node "<<node->NodeNo<<endl;
	for(int i =0 ; i < (node->NodeTasks); i++ )
	{
		if(node->Tasks[i].TaskNo == task->TaskNo)
		{
			(node->NodeTasks)--;
			
			for(int j = i; j < (node->NodeTasks); j++ )
				node->Tasks[i] = node->Tasks[i+1];
			
			return true;
		}
	}
	return false;
}

