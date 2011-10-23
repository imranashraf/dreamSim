// VexSim v1.0
// This is software simulator for task scheduling on Vex multiprocessor systems
// written by Arash Ostadzadeh
// ostadzadeh@gmail.com

#include "vexsim.h"
int schduledTasks=0;
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
	Total_Search_Length_Scheduler=0; // It accounts only for the steps taken by the scheduler to accommodate tasks (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
	Total_Scheduler_Workload=0; //Scheduler workload during one simulation run
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
	unsigned int i,j;
	Node *n;
	
	nodelist=new Node*[TotalNodes];
	if(!nodelist) { cerr<<"\nError in memory allocation.\n"; exit(1);}
	
	CurBlankNodeIndex=0;
	//nodesList=NULL; // no node initially in the already configured linked list
	
	for(i=0;i<TotalNodes;i++)
    {
		n=new Node;
		if(!n) { cerr<<"\nError in memory allocation.\n"; exit(1);}
		
		n->ReConfigCount=0; //counter for the no of configurations for this node
		n->NodeNo=i;  // The Node numbers are starting from 0.
		n->TotalArea=(x.rand_int31()%(NodehighA-NodelowA+1))+NodelowA;
		n->AvailableArea = n->TotalArea;
		n->Config_Task_Entries=0;
		
		n->NetworkDelay = (x.rand_int31()%(NWDHigh-NWDLow+1))+NWDLow;
		
		for (j=0; j < TotalConfigs ;j++) //MAX_NODE_CONFIGS
		{
			n->Config_Task_List[j].task = NULL;
			n->Config_Task_List[j].config = NULL;
			n->Inext[j]=NULL;
			n->Bnext[j]=NULL;			
		}
		
		nodelist[i]=n;
		
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
}

void VexSim::InitConfigs()
{
	int i;
	
	configs=new Config[TotalConfigs];
	if(!configs) { cerr<<"\nError in memory allocation.\n"; exit(1);}

	for(i=0;i<TotalConfigs;i++)
	{
		configs[i].ConfigNo=i;  // ConfigNo are beginning from 1.
		
		configs[i].ConfigTime = (x.rand_int31()%(ConfigTimeHigh-ConfigTimeLow+1))+ConfigTimeLow;
		
		configs[i].RequiredArea = (x.rand_int31()%(TaskhighA-TasklowA+1))+TasklowA;
		
    	configs[i].idle=NULL;
		configs[i].busy=NULL;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
    }
}

void VexSim::AddNodeToIdleList(Node *n, Config *conf)
{
	// update the idle list for the current node/config
	// add the node to the current idle list at the starting point

	n->Inext[conf->ConfigNo] = configs[conf->ConfigNo].idle;
	configs[conf->ConfigNo].idle=n;

	cout<<"\n Adding node "<<n->NodeNo<<" to idle list of config "<<conf->ConfigNo<<endl;
	//getchar();
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::RemoveNodeFromIdleList(Node *node, Config *conf)
{
	cout<<"\n In Remove Node from Idle list"<<endl;
	Node * n;
	if(conf == NULL)
		{cout<<"\n conf is null "<<endl;}
		
	if(configs[conf->ConfigNo].idle == NULL )
		{cout<<"\n No Idle list "<<endl; return;}
	else
		n=configs[conf->ConfigNo].idle;
	
	if (n->NodeNo == node->NodeNo) 
		configs[conf->ConfigNo].idle=n->Inext[conf->ConfigNo];
	else
	{
		while(n->Inext[conf->ConfigNo] && n->Inext[conf->ConfigNo] != node)	
			n=n->Inext[conf->ConfigNo];
		
		n->Inext[conf->ConfigNo] = n->Inext[conf->ConfigNo]->Inext[conf->ConfigNo];
	}
	cout<<"\n Removing node "<<node->NodeNo<<" from idle list of config "<<conf->ConfigNo<<endl;
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::AddNodeToBusyList(Node *n , Config *conf)
{
	// update the busy list for the current node/config
	// add the node to the current busy list at the starting point

	n->Bnext[conf->ConfigNo] = configs[conf->ConfigNo].busy;
	
	configs[conf->ConfigNo].busy=n;
	
	cout<<"\n Adding node "<<n->NodeNo<<" to busy list of config "<<conf->ConfigNo<<endl;

	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

void VexSim::RemoveNodeFromBusyList(Node *node , Config *conf)
{
	Node * n;  //busylist
	if(conf == NULL) 
		{cout<<"\n conf is null Press "<<endl; getchar();}

	if(configs[conf->ConfigNo].busy == NULL )
		{cout<<"\n No busy list Press "<<endl; getchar();}
	else
		n=configs[conf->ConfigNo].busy;
	
	if (n == node) 
		configs[conf->ConfigNo].busy=n->Bnext[conf->ConfigNo];
	else
	{
		while(n->Bnext[conf->ConfigNo] && n->Bnext[conf->ConfigNo] != node)	
			n=n->Bnext[conf->ConfigNo];
		
		n->Bnext[conf->ConfigNo]=n->Bnext[conf->ConfigNo]->Bnext[conf->ConfigNo];
	}
	cout<<"\n Removing node "<<node->NodeNo<<" from busy list of config "<<conf->ConfigNo<<endl;
		
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}	

bool VexSim::GiveEntryNo(Node *n, Task * t, unsigned int * EntryNo)
{
	for (int i=0; i < n->Config_Task_Entries ; i++)
	{
		if(n->Config_Task_List[i].task->TaskNo == t->TaskNo)
		{
			*EntryNo = i;
			return true;			
		}
	}
		
	return false;
}

void VexSim::TaskCompletionProc(Node* n, Task * t)
{
	unsigned int EntryNo;
	//print task report summary
	cout<<"Node # "<< n->NodeNo << " finished executing Task # "<< t->TaskNo <<endl;
	
	cout<<"Starting time: "<< t->StartTime <<"     CompletionTime= "<< t->CompletionTime <<endl;
	cout<<"elapsed time ticks: " << t->RequiredTime <<endl;
	
	//update report statistics
	TotalCompletedTasks++;
	Total_Tasks_Running_Time+=t->RequiredTime;
	
	if( GiveEntryNo(n, t, &EntryNo) == true )
	{	
		if (t->SusRetry) 
			cout<<"The task had been initially put in the suspension queue. "
				<< n->Config_Task_List[EntryNo].task->SusRetry << " retries have been carried out before accommodation\n";

		if(n->Config_Task_List[EntryNo].config == NULL)
		{	
			cout<<"\n t->TaskNo = "<<t->TaskNo<<endl;
			cout<<"\n t->NodeNo = "<<n->NodeNo<<endl;
			cout<<"\n n->Config_Task_List[EntryNo].config is null before going to removeNodefromBusyList"<<endl;
			cout<<"\n EntryNo = "<<EntryNo<<endl;
			getchar();
		}
		RemoveNodeFromBusyList(n,n->Config_Task_List[EntryNo].config);
		cout<<"\n calling AddNodeToIdleList from TaskCompletionProc"<<endl;
		AddNodeToIdleList(n,n->Config_Task_List[EntryNo].config);
	}
	else
	{
		cout<<"\n Task (TaskCompletion) "<<t->TaskNo<<" is not running on node "<<n->NodeNo<<endl;
		getchar();
	}
	if( RemoveTaskFromNode(n,t) == false )
	{
		cout<<"\n could not remove task "<<t->TaskNo <<" from node "<<n->NodeNo<<endl;
		getchar();
	}
		
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

bool VexSim::RemoveTaskFromNode(Node *node, Task *task)
{
	cout<<"\n Removing task "<<task->TaskNo<<" from node "<<node->NodeNo<<endl;
	unsigned int EntryNo;
	
	if( GiveEntryNo(node, task, &EntryNo) == true )
	{	
		node->Config_Task_List[EntryNo].task = NULL;
		return true;
	}
	else
	{
		cout<<"\n Task (RemoveTaskFromNode) "<<task->TaskNo<<" is not running on node "<<node->NodeNo<<endl;
		getchar();
	}
	
	getchar();
	
	return false;
}

void VexSim::SendTaskToNode(Task *t, Node *n)
{
	unsigned int conftime,EntryNo=0;
	Config * conf;
	
	t->StartTime=TimeTick;
	t->CompletionTime=TimeTick + t->RequiredTime;
	
	if( addTaskToNode(n,t) == false)
	{
		cout<<"\n Could not add task "<<t->TaskNo <<" to node "<<n->NodeNo<<endl;
		getchar();
	}
	else
	{
		if( GiveEntryNo(n, t, &EntryNo) == true )
		{	
			conf = n->Config_Task_List[EntryNo].config;
			// cout<<"\n Entry No found is "<<EntryNo<<endl;
			// cout<<"\n Configuration at this Entry No is "
				// <<n->Config_Task_List[EntryNo].config->ConfigNo
				// <<endl;
		}
		else
		{
			cout<<"\n (in SendTaskToNode) Task "<<t->TaskNo<<" is not running on node "<<n->NodeNo<<endl;
			getchar();
		}

		// remove the node from current idle list
		RemoveNodeFromIdleList(n,conf);
		// add the node to the current busy list
		AddNodeToBusyList(n,conf);
		
		conftime = configs[t->AssignedConfig].ConfigTime;
		//this need to be modified later, as conftime can be zero in the case when no configuration is required
		
		// update some report statistics
		Total_Configuration_Time+= conftime;
		Total_Wasted_Area += n->AvailableArea - conf->RequiredArea;
		Total_Task_Wait_Time += ( t->StartTime - t->CreateTime) + conftime; 
							// basically start time is the time for starting the configuration process, 
							// i.e we have identified optimal/preffered/... node and we want to configure it
							// so configuration will also take some time depending upon different factors
							// this mean that actual waiting time will be affected by config time as well
							// thats why its added here
							
		cout<<"Node # "<< n->NodeNo << " has started executing task # "<< t->TaskNo << endl;
		cout<<"Task creation time: "<< t->CreateTime <<endl;		
		cout<<"start time: "<< t->StartTime <<"  completion time: "<< t->CompletionTime << endl;		
	}
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

bool VexSim::addTaskToNode(Node *node, Task *task)
{
	for(int i=0;i<node->Config_Task_Entries;i++)
		if	(
			( ( (node->Config_Task_List[i]).config->ConfigNo) ) == (task->AssignedConfig) ||
			( ( (node->Config_Task_List[i]).config->ConfigNo) ) == (task->PrefConfig)
			)
		{
			node->Config_Task_List[i].task = task;
			(node->AvailableArea) -= (task->NeededArea);
			return true;
		}

	cout<<"\n Node "<<node->NodeNo<<" does not have configuration "<<task->AssignedConfig<<endl;
	getchar();
	
	return false;
	
}

Task * VexSim::CreateTask()
{
	Task *t;
	
	t=new Task;
	if (!t) { cerr<<"\nError in memory allocation for Task # " << TotalCurGenTasks <<"\n"; exit(1);}
	
	t->TaskNo=TotalCurGenTasks;
	t->NeededArea=(x.rand_int31()%(TaskhighA-TasklowA+1))+TasklowA;
	
	// we will assume about 10% of the created tasks preferring a configuration which is not available in the system
	// the criterion can be changed later
	//t->PrefConfig=1 + ( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) ); 
	t->PrefConfig=( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) ); 
	
	t->CreateTime=TimeTick;
	t->RequiredTime=(x.rand_int31()%(TaskReqTimehigh-TaskReqTimelow+1))+TaskReqTimelow;
	t->SusRetry=0;

	cout<<"Task # " << TotalCurGenTasks << " submitted to the system at time # " << TimeTick << endl;
	// can print other information about the task here
	
	TotalCurGenTasks++; // new task is going to be created
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.

	return t;
}

//this function checks if certain configuration is available on node 'n' 
// if yes then its EntryNo is returned 
bool VexSim::GiveEntryNo(Node * n, unsigned int confNo, unsigned int * EntryNo)
{
	for (int i=0; i < n->Config_Task_Entries; i++)
	{
		if( (n->Config_Task_List[i]).config !=NULL )
		{
			if( ( (n->Config_Task_List[i]).config)->ConfigNo == confNo)
			{
				*EntryNo = i;
				return true;
			}
		}
	}
	
	return false;
}

// a node is idle if it has some configuration but this configuration is 
// currently not being used by any task
bool VexSim::IsNodeIdle(Node * n)
{
	for (int i=0; i < n->Config_Task_Entries; i++)
		if(n->Config_Task_List[i].task == NULL)
		{
			return true;
		}
		
	return false;
}

Task * VexSim::CheckSuspensionQueue(Node *n)
{
	unsigned int EntryNo=0;
	SusList* temp=suspendedlist;
	// the search for a match on the suspension queue will terminate as soon as a matched node with the same config and available area found
	// could be a point for further improvement (a more intelligent match making to optimize waste area ,...)
	
	SusList* prev_temp=NULL; // this always precedes temp

	while(temp) 
	{
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		
		//Here we are checking the tasks in the suspended list one by one, if the configuration required by this task 
		//is already available on this node, if a match is found then the task is removed from the suspending list
		//and returned
		
		if	( GiveEntryNo(n, temp->item->AssignedConfig , &EntryNo)	== true ) 	// a configuration is found on this node
																				//EntryNo is not needed here
		{
			Task * t=temp->item;

			//update the suspended tasks list
			if (!prev_temp) // first node to be removed from suspension queue
				suspendedlist=temp->next;
			else 
				prev_temp->next=temp->next;
			
			delete temp;
			Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.			
			return t;
		}
		
		temp->item->SusRetry++; // this task was agained tested to be assigned to a node without success
								// the info looks stupid for now but can be useful later. for now the list is checked on every time tick (crazy)!!!!
		prev_temp=temp;
		temp=temp->next;
	}
	
	return NULL; // no suitable task found for the recently released node
}

// this function will return the first task in the suspension queue
Task * VexSim::GetAnyTaskFromSuspensionQueue()
{
	SusList* temp=suspendedlist;

	if(temp) 
	{
		Task * t=temp->item;
		suspendedlist=temp->next;
			
		delete temp;
		TotalCurSusTasks--; //as a task has been removed
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.			
		return t;
	}
		
	return NULL; // no task in suspension queue
}

// a node is blank if it has no configuration at all
bool VexSim::IsNodeBlank(Node * n)
{
	return ( n->Config_Task_Entries == 0);
}

Config* VexSim::findPreferredConfig(Task *t)
{
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	
	// for now just a stupid straightforward check!	
	if (t->PrefConfig < TotalConfigs) 
		return &configs[t->PrefConfig];
	
	return NULL; // no exact match
}

Config* VexSim::findClosestConfig(Task *t)
{
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	
	// for now just a random config # will be picked up as the closest match
	return &configs[(x.rand_int31()%TotalConfigs)];
}
	
	
Node* VexSim::findAnyIdleNode(Task* t,unsigned long long int& SL)
{
 	unsigned long int i;
 	// trying to find any (first) available idle node with any kind of configuration
 	
	for ( i=0; i < TotalNodes; i++)
 	{
 		if 	(
			(IsNodeIdle(nodelist[i])) && 
			( nodelist[i]->AvailableArea >= (configs[t->PrefConfig].RequiredArea)  ) &&
			(nodelist[i]->Config_Task_Entries < MAX_NODE_CONFIGS)
			)
			return nodelist[i];
		
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
 	}
	return NULL;
}

/*
Node* VexSim::findBestBlankNodeMatch(Task* t,unsigned long long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff, nodearea,taskarea;
 	signed long int temp;
 	signed long int counter=0;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(counter<TotalNodes)			// find the first suitable node
 	{
		if( IsNodeBlank(nodelist[counter]) )
		{
			nodearea = nodelist[counter]->TotalArea;
			taskarea = configs[t->PrefConfig].RequiredArea;
			if ( nodearea >= taskarea && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS)) 
			{
				bestMatchindex=counter;
				mindiff= nodearea - taskarea;
				break;
			}
		}			
		counter++;
		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
	
	if(bestMatchindex==-1) 
		return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		if( IsNodeBlank(nodelist[counter]) )
		{
			nodearea = nodelist[counter]->TotalArea;
			taskarea = configs[t->PrefConfig].RequiredArea;
			temp=nodearea - taskarea;
			if ( (temp>=0) && ( mindiff > temp ) && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
			{
				bestMatchindex=counter;
				mindiff=temp;
			}
		}
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	return nodelist[bestMatchindex];
}
*/

/////////////////////////////////////
// this is the modified version of the above function which will check 
// if partial reconfiguration is possible or node
Node* VexSim::findBestBlankNodeMatch(Task* t,unsigned long long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff, nodearea,taskarea;
 	signed long int temp;
 	signed long int counter=0;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(counter<TotalNodes)			// find the first suitable node
 	{
		// if( IsNodeBlank(nodelist[counter]) )
		// {
			nodearea = nodelist[counter]->AvailableArea;
			taskarea = configs[t->PrefConfig].RequiredArea;
			if ( nodearea >= taskarea && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS)) 
			{
				bestMatchindex=counter;
				mindiff= nodearea - taskarea;
				break;
			}
		// }			
		counter++;
		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
	
	if(bestMatchindex==-1) 
		return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		// if( IsNodeBlank(nodelist[counter]) )
		// {
			nodearea = nodelist[counter]->AvailableArea;
			taskarea = configs[t->PrefConfig].RequiredArea;
			temp=nodearea - taskarea;
			if ( (temp>=0) && ( mindiff > temp ) && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
			{
				bestMatchindex=counter;
				mindiff=temp;
			}
		// }
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	return nodelist[bestMatchindex];
}

/////////////////////////////////////
	
Node* VexSim::findBestNodeMatch(Task* t,Node *idlelist,unsigned long long int& SL)
{
	Node * bestMatch=NULL;
 	unsigned long int mindiff,nodearea,taskarea,EntryNo;
 	signed long int temp; 
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(idlelist)			// find the first suitable node
 	{
		nodearea = idlelist->TotalArea;
		taskarea = configs[t->AssignedConfig].RequiredArea;
 		if (nodearea >= taskarea && (idlelist->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
 		{
 			bestMatch=idlelist;
 			mindiff=nodearea - taskarea;
 			break;
 		}
		
		idlelist = idlelist->Inext[t->AssignedConfig];
		
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
 	}
	
	if(!bestMatch) return NULL;
	
	while(idlelist)	// check the remaining nodes to find a better match
	{
		nodearea = idlelist->TotalArea;
		taskarea = configs[t->AssignedConfig].RequiredArea;
		temp=nodearea - taskarea;
		
 		if ( (temp>=0) && ( mindiff > temp ) && (idlelist->Config_Task_Entries < MAX_NODE_CONFIGS)) 
 		{
 			bestMatch=idlelist;
 			mindiff=temp;
 		}
  		idlelist=idlelist->Inext[t->AssignedConfig];
  		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}	

	return bestMatch;
}



void VexSim::makeNodeBlank(Node *n)
{
	cout<<"making Node # " << n->NodeNo <<" blank"<<endl;
	
	n->Config_Task_Entries = 0;
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::sendBitstream(Node *n, Config *conf)
{
	(n->ReConfigCount)++;
	
	// cout<<"In sending bitstream for configuration # " << conf->ConfigNo 
		// << " to the Node # " <<n->NodeNo
		// << "\n reconfiguration count for this node = "<<n->ReConfigCount
		// << " Entry No for this will be "<<n->Config_Task_Entries
		// <<endl;

	n->Config_Task_List[n->Config_Task_Entries].config = conf;
	
	//now that we have a certain configuration on the node, we should
	//add this node to idle list for this particular configuration
	AddNodeToIdleList(n,conf);

	(n->Config_Task_Entries)++;	
	
	if(n->Config_Task_Entries > 1)
	{
		cout<<"\n ####### multiple configurations #######"<<endl;
		cout<<"Node "<<n->NodeNo<<" has Config_Task_Entries "<<n->Config_Task_Entries<<endl;
	}
	
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

bool VexSim::IsNodeFull(Node * n, Task *t)
{
	if( (n->Config_Task_Entries) >= MAX_NODE_CONFIGS)
		return true;
		
	if(n->AvailableArea <= configs[t->PrefConfig].RequiredArea)
		return true;
	
	for (int i=0; i < n->Config_Task_Entries; i++)
	{
		if(n->Config_Task_List[i].task != NULL)
		{
			return false;
		}
	}

	return true;
}

// check the nodes of all the busy list one by one
// if a node is full at this moment, but area requirements are 
// such that it can be used for this task in the future 
// then return true
bool VexSim::queryBusyListforPotentialCandidate(Task *t, unsigned long long int& SL )
{
	Node *n;
	
	for(int i=0; i<TotalConfigs ;i++)
	{
		n = configs[i].busy;
		
		while(n != NULL)
		{
			// if( IsNodeFull(n, t) == true)
			// {
			if( n->TotalArea >= (configs[t->AssignedConfig].RequiredArea) ) 
				return true;
			// }
			n=n->Bnext[i];
		}
		SL++;
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
	}
	
	return false;
}

void VexSim::PutInSuspensionQueue(Task *t)
{
	SusList* cur;
	
	cur=new SusList;
	if(!cur) { cerr<<"\nError in memory allocation for suspended task list.\n"; exit(1);}
	
	cout<<"puting the task # "<<t->TaskNo<<" on the suspended tasks queue\n";
	cur->item=t;
	cur->next=suspendedlist;
	suspendedlist=cur;
	TotalCurSusTasks++;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::DiscardTask(Task *t)
{
	cout<<"Task # "<<t->TaskNo<<" can not be accommodated by the scheduler. Discarding the task!"<<endl;
	TotalDiscardedTasks++;
	Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
}

unsigned long VexSim::TotalReConfigCount()
{
	unsigned int i;
	unsigned long Total=0;
	
	for(i=0;i<TotalNodes;i++)
    {
		Total += nodelist[i]->ReConfigCount; //each node has the info of its reconfig count
	}
	return Total;
}


void VexSim::RunVexScheduler(Task *t)
{
	Config * Cmatch,* Closestmatch;
	bool found=false;
	unsigned long long int SL=0, EntryNo;
	Node *n;

	Cmatch=findPreferredConfig(t);
	
	if (Cmatch) // required configuration was found in the list (exact match)
	{
		t->AssignedConfig = Cmatch->ConfigNo; // set the assigned config for the current task
		
		if(Cmatch->idle) // there are idle nodes available with the preferred config
		{
			
			n=findBestNodeMatch(t,Cmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
			Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		
			if (n) 
			{	
				schduledTasks++;
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(t,n);  // found a suitable node
				found=true;
			}
		}
		
		
		if(!found) // no suitable idle node found at this point
		{
			n=findBestBlankNodeMatch(t,SL);
			if (n)
			{
				schduledTasks++;			
				Total_Search_Length_Scheduler+=SL;
				Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
				cout<<"\n Sending bit stream for configuration "<<Cmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
				sendBitstream(n,Cmatch);
				SendTaskToNode(t,n);
				found=true;					
			}
			
			if(!found) // no blank node available or there is no suitable blank node available!!!
			{			// try reconfiguring one of the idle nodes!
				n=findAnyIdleNode(t,SL);
				if (n) // An idle node is found for reconfiguration
				{
					schduledTasks++;				
					Config * conf;
					Total_Search_Length_Scheduler+=SL;
					RemoveNodeFromIdleList(n,&configs[t->AssignedConfig]);
					makeNodeBlank(n);
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(n,Cmatch);
					SendTaskToNode(t,n);
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
		/*
		Closestmatch=findClosestConfig(t); // we assume that there is always a closest config match
											// for now because we actually do not look for closest match the SL is not increased,
											// in reality SL is also increased correspondingly to indicate the search effort
		t->AssignedConfig=Closestmatch->ConfigNo; // set the assigned config for the current task
		Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
		
		if(Closestmatch->idle) // there are idle nodes available with the closest config
		{
			n=findBestNodeMatch(t,Closestmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
		
			if (n) 
			{	
				schduledTasks++;
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(t,n);  // found a suitable node
				found=true;
			}
		}
		if(!found) // no (suitable) idle nodes available
		{
			// in case we are coming from an exact match configuration part, we have already searched 
			// the blank list and no need for a re-search
			if( !Cmatch	) 
			{
				n=findBestBlankNodeMatch(t,SL);
				if (n)
				{
					schduledTasks++;				
					Total_Search_Length_Scheduler+=SL;
					Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(n,Closestmatch);
					SendTaskToNode(t,n);
					found=true;
				}
			}// end of blank node(s) still available
		}
		*/
		if(!found) // no blank node available or there is no suitable blank node available!!!
			     	   // we are going to wait for a busy node to become idle!
						// The last solution!
		{
			if(Cmatch) 
				t->AssignedConfig=Cmatch->ConfigNo; // adjust the assigned config field again if needed, in case there was previously an exact match
				
			Total_Scheduler_Workload++; //Scheduler workload is associated with total scheduler workload required during one simulation run.
			
			found=queryBusyListforPotentialCandidate(t,SL); // determine whether or not the current busy list has any potential candidate for 
															// accommodating the task regarding the Area restriction
			Total_Search_Length_Scheduler+=SL;
																			
			if( found )   // if yes
			{
				PutInSuspensionQueue(t);
			}
			// we can also go for another check with all busy nodes and do a reconfig later if needed, not considered in this scheduler!!!
			else 
				DiscardTask(t); // bad task, throw it away! 
		} // end of waiting!
	}// end of no exact match or we had exact match but were not able to accommodate on exact config list
}

void VexSim::MakeReport()
{
	ofstream f;
	
	sprintf (fileName, "%d", TotalTasks);
	strcat(fileName, ".txt");
	  	  
	f.open(fileName);
	if (f.fail()) 
	{ 
		cout<<"\n failed opening the simulation report file.\n"; exit(1); 
	}
	  
	f<<"total_tasks_generated\t"<<TotalTasks<<endl;
	f<<"total_PEs\t"<<TotalNodes<<endl;
	f<<"total_configurations\t"<<TotalConfigs<<endl;
	f<<"total_simulation_time\t"<<TimeTick<<endl;

	f<<"task_generation_interval\t[ 1 ... "<<NextTaskMaxInterval<<" ]"<<endl;
	f<<"PE_available_area_range\t[ "<<NodelowA<<" ... "<<NodehighA<<" ]"<<endl;
	f<<"task_required_area_range\t[ "<<TasklowA<<" ... "<<TaskhighA<<" ]"<<endl;
	f<<"task_required_timeslice_range\t[ "<<TaskReqTimelow<<" ... "<<TaskReqTimehigh<<" ]"<<endl;
	
	f<<"total_tasks_completed\t"<<TotalCompletedTasks<<endl;
	f<<"total_tasks_discarded\t"<<TotalDiscardedTasks<<endl;
	f<<"total_wasted_area\t"<<Total_Wasted_Area<<endl;
	f<<"average_wasted_area_per_task\t"<<(Total_Wasted_Area)/(double)(TotalCompletedTasks)<<endl;
	
	f<<"total_scheduling_steps\t"<<Total_Search_Length_Scheduler<<endl;
	f<<"average_scheduling_steps_per_task\t"<<(Total_Search_Length_Scheduler)/(double)(TotalTasks)<<endl;
	
	f<<"total_tasks_waiting_time\t"<<Total_Task_Wait_Time<<endl;
	f<<"average_task_waiting_time\t"<<(Total_Task_Wait_Time)/(double)(TotalCompletedTasks)<<endl;
	
	f<<"total_tasks_running_time\t"<<Total_Tasks_Running_Time<<endl;
	f<<"average_task_running_time\t"<<(Total_Tasks_Running_Time)/(double)(TotalCompletedTasks)<<endl;
	
	f<<"total_configuration_time\t"<<Total_Configuration_Time<<endl;
	f<<"average_configuration_time_per_task\t"<<(Total_Configuration_Time)/(double)(TotalCompletedTasks)<<endl;
	f<<"Total_reconfiguration_count\t"<<TotalReConfigCount()<<endl;
	f<<"average_reconfiguration_count_per_node\t"<<TotalReConfigCount()/(double)TotalNodes<<endl;
	f<<"Total_Scheduler_Workload\t"<<Total_Scheduler_Workload<<endl;
	
	f.close();
}

Task * VexSim::CompletedTask(Node * n)
{
	int i;
	Task * t;
	
	for (i=0; i < n->Config_Task_Entries; i++)
	{
		if( (n->Config_Task_List[i].task)->CompletionTime == TimeTick)
		{
			t = n->Config_Task_List[i].task;
			return t;
		}
	}
		
	return NULL;
}

void VexSim::Start()
{
	unsigned long long int nextIncomTaskTimeTick=0;
	Task* t;
	
	while( ( TotalCompletedTasks + TotalDiscardedTasks ) < TotalTasks )  // still there are tasks which are not finished
	//while( ( TotalCompletedTasks + TotalCurSusTasks ) < TotalTasks )  // still there are tasks which are not finished
	{
		
		nextIncomTaskTimeTick=TimeTick + 1 + (x.rand_int31() % NextTaskMaxInterval);
		IncreaseTimeTick();  // advance one time tick
		
		// can be improved later because there is actually no need to check on every time tick
		while(TimeTick<=nextIncomTaskTimeTick) // check for completed tasks first then check whether or not suspended tasks could be accomodated
		{
			for(unsigned int i=0;i<TotalNodes;i++,Total_Scheduler_Workload++)
			{	
				if(IsNodeIdle(nodelist[i]) == false) // there is a task running at this node
				{
					Task *tmp;
					
					tmp = CompletedTask( nodelist[i] );
					
					if( tmp ) //task termination housekeeping
					{
						TaskCompletionProc(nodelist[i],tmp);
						
						//now lets have a look at suspension queue for a suspended task which can run on this node
						tmp=CheckSuspensionQueue(nodelist[i]);
						if (tmp) // found a task in suspension queue to accomodate on this node
						{
							cout<<"removing task # "<< tmp->TaskNo <<" from suspension queue\n";
							// send the task to the recently released node
							SendTaskToNode(tmp,nodelist[i]);
							TotalCurSusTasks--; 
						}
					}
				}
			}
			IncreaseTimeTick();  // advance one time tick
		}// end of while ( TimeTick<=nextIncomTaskTimeTick )
		
		DecreaseTimeTick();  // the time needs to be adjusted just for the last unsuccessful increase in the TimeTick, otherwise we miss
							//  one of the ticks here! Note that the current TimeTick at this point is one unit ahead of the actual value
		
		cout<<"\n TimeTick : "<<TimeTick
			<<"\n nextIncomTaskTimeTick : "<<nextIncomTaskTimeTick
			<<"\n TotalTasks : "<<TotalTasks
			<<"\n TotalCompletedTasks : "<<TotalCompletedTasks
			<<"\n TotalDiscardedTasks : "<<TotalDiscardedTasks
			<<"\n TotalCurGenTasks : "<<TotalCurGenTasks
			<<"\n schduledTasks : "<<schduledTasks
			<<"\n TotalCurSusTasks : "<<TotalCurSusTasks
			<<endl;
			
		//create the new scheduled task
		if ( TotalCurGenTasks < TotalTasks ) // still we need to generate more tasks!
		{
			t=CreateTask();
			//send the created task to the scheduler 
			RunVexScheduler(t);
		}
		else if( TotalCurSusTasks > 0 )
		{
			Task * tmp;
			tmp=GetAnyTaskFromSuspensionQueue();
			RunVexScheduler(tmp);
		}
	}// main loop of the simulation
	cout<<"\n Going to MakeReport"<<endl;
	MakeReport(); 	// end of the simulation, make the final report
}

void VexSim::printBusyLists()
{
	Node *n;
	cout<<endl;
	for(int i=0; i<TotalConfigs ;i++)
	{
		if(!configs[i].busy)
			continue;
		
		cout<<"busy lists for confNo "<<i<<" -> ";			
		n = configs[i].busy;
		while(n != NULL)
		{
			cout<<n->NodeNo<<" ";
			n=n->Bnext[i];
		}
		cout<<endl;
	}
}

void VexSim::printIdleLists()
{
	Node *n;
	cout<<endl;
	for(int i=0; i<TotalConfigs ;i++)
	{
		if(!configs[i].idle)
			continue;
		
		cout<<"idle lists for confNo "<<i<<" -> ";			
		n = configs[i].idle;
		while(n != NULL)
		{
			cout<<n->NodeNo<<" ";
			n=n->Inext[i];
		}
		cout<<endl;
	}
}
