// This is software simulator for task scheduling on Vex multiprocessor systems
// written by:
// Arash Ostadzadeh (ostadzadeh@gmail.com)
// Imran Ashraf	(imran.ashraf@ymail.com) 

#include "vexsim.h"

VexSim::VexSim(unsigned int TN,unsigned int TC, unsigned long int TT, unsigned int NextTaskMaxInterval,
			   unsigned int NlowA, unsigned int NhighA,
			   unsigned int ClowA, unsigned int ChighA,
		       unsigned int TRTlow, unsigned int TRThigh,
			   unsigned int ConfTmL , unsigned int ConfTmH,
			   unsigned int NWDH, unsigned int NWDL,
			   double ClConfPercent)
{
	TotalNodes=TN;
	TotalConfigs=TC;
	TotalTasks=TT;

	NodelowA=NlowA;
	NodehighA=NhighA;

	ConfiglowA=ClowA;
	ConfighighA=ChighA;
	
	TaskReqTimelow=TRTlow;
	TaskReqTimehigh=TRThigh;
	ClosestConfigPercent = ClConfPercent;

	VexSim::NextTaskMaxInterval=NextTaskMaxInterval;
	
	TotalCompletedTasks=0;
	TotalCurGenTasks=0;
	TotalCurSusTasks=0;
	TotalDiscardedTasks=0;
	LastTaskCompletionTime=0;
	SchduledTasks=0;
	
	suspendedlist=NULL; // no task initially in the suspension list

	// for report
	Total_Wasted_Area=0;
	Total_Search_Length_Scheduler=0; // It accounts only for the steps taken by the scheduler to accommodate tasks (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
	Total_Simulation_Workload=0; //Simulation workload during one simulation run
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
	
 	dumpf.open("dump.txt");
 	if (dumpf.fail()) {cout<<"\n failed opening the dump file.\n"; exit(1); }

}

void VexSim::InitNodes()
{
	unsigned int i,j;
	Node *n;

	
	nodelist=new Node*[TotalNodes];
	if(!nodelist) { cerr<<"\nError in memory allocation.\n"; exit(1);}
	
	CurBlankNodeIndex=0;
	
	for(i=0;i<TotalNodes;i++)
    {
		n=new Node;
		if(!n) { cerr<<"\nError in memory allocation.\n"; exit(1);}
		
		n->Inext = new Node *[TotalConfigs];
		if(!(n->Inext) ) { cerr<<"\nError in memory allocation.\n"; exit(1);}
		
		n->Bnext = new Node *[TotalConfigs];
		if(!(n->Bnext) ) { cerr<<"\nError in memory allocation.\n"; exit(1);}

		n->CountInIdleList = new unsigned int [TotalConfigs];
		if(!(n->CountInIdleList) ) { cerr<<"\nError in memory allocation.\n"; exit(1);}

		n->CountInBusyList = new unsigned int [TotalConfigs];
		if(!(n->CountInBusyList) ) { cerr<<"\nError in memory allocation.\n"; exit(1);}

		n->ReConfigCount=0; //counter for the no of configurations for this node
		n->NodeNo=i;  // The Node numbers are starting from 0.
		n->TotalArea=(x.rand_int31()%(NodehighA-NodelowA+1))+NodelowA;
		n->AvailableArea = n->TotalArea;
		
		n->Config_Task_Entries=0;
		
		n->NetworkDelay = (x.rand_int31()%(NWDHigh-NWDLow+1))+NWDLow;
		
		for (j=0; j < MAX_NODE_CONFIGS ;j++) 
		{
			n->Config_Task_List[j].task = NULL;
			n->Config_Task_List[j].config = NULL;
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.			
		}
		
		for (j=0; j < TotalConfigs ;j++) 
		{
			n->Inext[j]=NULL;
			n->Bnext[j]=NULL;
			n->CountInIdleList[j]=0;
			n->CountInBusyList[j]=0;
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.			
		}
		
		nodelist[i]=n;
	}

}

void VexSim::InitConfigs()
{
	int i;

	configs=new Config[TotalConfigs];
	if(!configs) { cerr<<"\nError in memory allocation.\n"; exit(1);}

	for(i=0;i<TotalConfigs;i++)
	{
		configs[i].ConfigNo=i;  // ConfigNo are beginning from 0
		
		configs[i].RequiredArea = (x.rand_int31()%(ConfighighA-ConfiglowA+1))+ConfiglowA;
		
		//Cofiguration time is a function of area required for certain configuration
		configs[i].ConfigTime = (configs[i].RequiredArea) / ConfiglowA + 5; //where 5 is some overhead which will alsways be there
		
		configs[i].idle=NULL;
		configs[i].busy=NULL;
		
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
    }
    
}

bool VexSim::SearchIdleList(Node * n , unsigned int confno)
{
    Node *idlelist = configs[confno].idle;
    while(idlelist != NULL)
    {
		if(idlelist == n)
			return true;
		
		idlelist=idlelist->Inext[confno];
    }
	return false;	
}

bool VexSim::SearchBusyList(Node * n , unsigned int confno)
{
    Node *busylist = configs[confno].busy;
    while(busylist != NULL)
    {
		if(busylist == n)
			return true;
		
		busylist=busylist->Bnext[confno];
    }
	return false;	
}

void VexSim::AddNodeToIdleList(Node *n, Config *conf)
{
	int i;
	cout<<"\n Adding node "<<n->NodeNo<<" to idle list of config "<<conf->ConfigNo;
	
	if(DEBUG_MODE) 
	{
		cout<<"\n Idle list before adding ";
		printOneIdleList(conf->ConfigNo);
		getchar();
	}
	
	//this is  a simple solution to the problem of multiple configurations of same configNo on a single node
	//for now, in this situation a new entry will not be created to avoid circular pointer
	if( SearchIdleList(n, conf->ConfigNo) == true )  //already available
	{
		(n->CountInIdleList[conf->ConfigNo] )++;
	}
	else
	{	//no multiple configurations of same type, so add
	    // update the idle list for the current node/config
	    // add the node to the current idle list at the starting point
		n->CountInIdleList[conf->ConfigNo] = 1; //as it is the first entry
		
	    n->Inext[conf->ConfigNo] = configs[conf->ConfigNo].idle;
	    configs[conf->ConfigNo].idle=n;
	}
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if(DEBUG_MODE) 
	{
		cout<<"\n Idle list after adding ";
		printOneIdleList(conf->ConfigNo);
		getchar();
	}
	
}	

void VexSim::RemoveNodeFromIdleList(Node *node, Config *conf)
{
	Node * idlelist; 

	cout<<"\n Entering RemoveNodeFromIdleList";

	if(conf == NULL)
		{cout<<"\n conf is null "<<endl; getchar(); return;}
		
	if(configs[conf->ConfigNo].idle == NULL )
		{cout<<"\n No Idle list "<<endl; getchar(); return; }
	else
	    idlelist=configs[conf->ConfigNo].idle;
		
	cout<<"\n Removing node "<<node->NodeNo<<" from idle list of config "<<conf->ConfigNo;		

	if(DEBUG_MODE) 
	{
		cout<<"\n Idle list before removing ";
		printOneIdleList(conf->ConfigNo);
		getchar();
	}
	
	if(node->CountInIdleList[conf->ConfigNo] > 1 )  //multiple configurations of same type
	{
		(node->CountInIdleList[conf->ConfigNo] )--;
	}
	else
	{
	    if (idlelist->NodeNo == node->NodeNo) 
		    configs[conf->ConfigNo].idle= idlelist->Inext[conf->ConfigNo];
	    else
	    {
		    while(idlelist->Inext[conf->ConfigNo] && idlelist->Inext[conf->ConfigNo] != node)	
			    idlelist=idlelist->Inext[conf->ConfigNo];
		    
		    idlelist->Inext[conf->ConfigNo] = idlelist->Inext[conf->ConfigNo]->Inext[conf->ConfigNo];
	    }
	}
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if(DEBUG_MODE) 
	{
		cout<<"\n Idle list after removing ";
		printOneIdleList(conf->ConfigNo);
		getchar();
	}
}	

void VexSim::AddNodeToBusyList(Node *n , Config *conf)
{
	int i;
	
	cout<<"\n Adding node "<<n->NodeNo<<" to busy list of config "<<conf->ConfigNo<<endl;	
	
	if(DEBUG_MODE) 
	{
		cout<<"\n Busy List before adding "<<endl;
		printOneBusyList(conf->ConfigNo);
		getchar();
	}

	//this is  a simple solution to the problem of multiple configurations of same configNo on a single node
	//for now, in this situation a new entry will not be created to avoid circular pointer
	if( SearchBusyList(n, conf->ConfigNo) == true )  //already available
	{
		(n->CountInBusyList[conf->ConfigNo] )++;
		// cout<<"\n node->CountInBusyList[conf->ConfigNo] "<<n->CountInBusyList[conf->ConfigNo]<<endl;
		// getchar();
	}
	else
	{
		n->CountInBusyList[conf->ConfigNo] = 1; //as it is the first entry
	    // update the busy list for the current node/config
	    // add the node to the current busy list at the starting point
	    n->Bnext[conf->ConfigNo] = configs[conf->ConfigNo].busy;
	    configs[conf->ConfigNo].busy=n;
	}
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if(DEBUG_MODE)
	{
		cout<<"\n Busy List after adding "<<endl;
		printOneBusyList(conf->ConfigNo);
		getchar();
	}
}	

void VexSim::RemoveNodeFromBusyList(Node *node , Config *conf)
{
	Node * n;  //busylist
	if(conf == NULL) 
		{cout<<"\n conf is null"<<endl; getchar(); return;}

	if(configs[conf->ConfigNo].busy == NULL )
		{cout<<"\n No busy list"<<endl; getchar(); return;}
	else
		n=configs[conf->ConfigNo].busy;
	
	cout<<"\n Removing node "<<node->NodeNo<<" from busy list of config "<<conf->ConfigNo<<endl;		

	if(DEBUG_MODE) 
	{
		cout<<"\n Busy List before removing"<<endl;
		printOneBusyList(conf->ConfigNo);
		getchar();
	}
	
	if(node->CountInBusyList[conf->ConfigNo] > 1 )  //multiple configurations of same type
	{
		(node->CountInBusyList[conf->ConfigNo] )--;
		// cout<<"\n node->CountInBusyList[conf->ConfigNo]"<<node->CountInBusyList[conf->ConfigNo]<<endl;
		// getchar();
	}
	else
	{
	    if (n == node) //if this is the first node
		    configs[conf->ConfigNo].busy=n->Bnext[conf->ConfigNo];
	    else
	    {
		    while(n->Bnext[conf->ConfigNo] && n->Bnext[conf->ConfigNo] != node)	
		    {
			    n=n->Bnext[conf->ConfigNo];
		    }
		    
		    n->Bnext[conf->ConfigNo]=n->Bnext[conf->ConfigNo]->Bnext[conf->ConfigNo];
	    }
	}
		
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	if(DEBUG_MODE) 
	{
		cout<<"\n Busy List after removing "<<endl;
		printOneBusyList(conf->ConfigNo);
		getchar();
	}
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

//this function returns the entry no of a task running on a node
bool VexSim::GiveEntryNo(Node *n, Task * t, unsigned int * EntryNo)
{
	for (int i=0; i < n->Config_Task_Entries ; i++)
	{
		if( n->Config_Task_List[i].task != NULL)
		{
			if(n->Config_Task_List[i].config == NULL)
			{
			  cout<<"\n found task without configuration ..."<<endl;
			  getchar();
			}
			  
			if(n->Config_Task_List[i].task->TaskNo == t->TaskNo)
			{
				*EntryNo = i;
				return true;			
			}
		}
	}
	return false;
}

void VexSim::TaskCompletionProc(Node* n, Task * t, unsigned int EntryNo)
{
	//print task report summary
	cout<<"\n Entring TaskCompletionProc";
	
	cout<<"\n Node # "<< n->NodeNo << " finished executing Task # "<< t->TaskNo <<endl;
	
	cout<<"Starting time: "<< t->StartTime <<"     CompletionTime= "<< t->CompletionTime <<endl;
	cout<<"elapsed time ticks: " << t->RequiredTime <<endl;
	
	//update report statistics
	TotalCompletedTasks++;
	Total_Tasks_Running_Time+=t->RequiredTime;
	
	if (t->SusRetry)
		cout<<"The task had been initially put in the suspension queue. "
			<< n->Config_Task_List[EntryNo].task->SusRetry << " retries have been carried out before accommodation\n";

	RemoveNodeFromBusyList(n,n->Config_Task_List[EntryNo].config);
	AddNodeToIdleList(n,n->Config_Task_List[EntryNo].config);
	n->Config_Task_List[EntryNo].task = NULL;
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if(DEBUG_MODE)
	{
		cout<<"\n Leaving TaskCompletionProc";		
		printNode(n);	
		getchar();
	}
}

// bool VexSim::RemoveTaskFromNode(Node *node, Task *task)
// {
	// cout<<"\n Removing task "<<task->TaskNo<<" from node "<<node->NodeNo<<endl;
	// unsigned int EntryNo;
	
	// if( GiveEntryNo(node, task, &EntryNo) == true )
	// {	
		// node->Config_Task_List[EntryNo].task = NULL;
		// return true;
	// }
	// else
	// {
		// cout<<"\n Task (RemoveTaskFromNode) "<<task->TaskNo<<" is not running on node "<<node->NodeNo<<endl;
		// getchar();
	// }
	
	// return false;
// }

void VexSim::SendTaskToNode(Task *t, Node *n)
{
	unsigned int conftime,EntryNo=0;
	Config * conf;
	
	t->StartTime=TimeTick;
	t->CompletionTime=TimeTick + t->RequiredTime;
	
	if( t->CompletionTime > LastTaskCompletionTime)
	    LastTaskCompletionTime = t->CompletionTime;
	
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
			cout<<"\n Entry No found is "<<EntryNo<<endl;
			cout<<"\n Configuration at this Entry No is "
				<<n->Config_Task_List[EntryNo].config->ConfigNo
				<<endl;
		}
		else
		{
			cout<<"\n (in SendTaskToNode) Task "<<t->TaskNo<<" is not running on node "<<n->NodeNo<<endl;
			getchar();
			return;
		}

		// remove the node from current idle list
		RemoveNodeFromIdleList(n,conf);
		// add the node to the current busy list
		AddNodeToBusyList(n,conf);
		
		// update some report statistics
 		//Total_Wasted_Area += n->AvailableArea - conf->RequiredArea;
		
		Total_Wasted_Area += (n->AvailableArea);
	
		Total_Task_Wait_Time += ( t->StartTime - t->CreateTime) ;  //+ conftime
							// basically start time is the time for starting the configuration process, 
							// i.e we have identified optimal/preffered/... node and we want to configure it
							// so configuration will also take some time depending upon different factors
							// this mean that actual waiting time will be affected by config time as well
							// thats why its added here
							
		cout<<"Node # "<< n->NodeNo << " has started executing task # "<< t->TaskNo << endl;
		cout<<"Task creation time: "<< t->CreateTime <<endl;		
		cout<<"start time: "<< t->StartTime <<"  completion time: "<< t->CompletionTime << endl;
		if(TASK_TRACK_MODE) getchar();
	}
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
}

bool VexSim::addTaskToNode(Node *node, Task *task)
{
	int i;
	
	for(i=0; i < (node->Config_Task_Entries); i++)
	{
		if(	(
			( ( (node->Config_Task_List[i]).config->ConfigNo) ) == (task->AssignedConfig) 
			//|| ( ( (node->Config_Task_List[i]).config->ConfigNo) ) == (task->PrefConfig)
			) 
			&& (node->Config_Task_List[i]).task == NULL  //that is there is no already running task on this config
									//this is important as in case of multiple same configuration
									//on a single node, task should be assigned to right one
		  )
		{
			node->Config_Task_List[i].task = task;
			return true;
		}
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
	
	// we will assume about 10% of the created tasks preferring a configuration which is not available in the system
	// the criterion can be changed later
	//t->PrefConfig=1 + ( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) ); 
	t->PrefConfig=( x.rand_int31()% ( (unsigned int) (ClosestConfigPercent * TotalConfigs) ) );
	
	t->CreateTime=TimeTick;
	t->RequiredTime=(x.rand_int31()%(TaskReqTimehigh-TaskReqTimelow+1))+TaskReqTimelow;
	t->SusRetry=0;

	cout<<"Task # " << TotalCurGenTasks << " submitted to the system at time # " << TimeTick << endl;
	// can print other information about the task here
	
	TotalCurGenTasks++; // new task is going to be created
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.

	return t;
}

// a node is idle if it has some configuration but this configuration is 
// currently not being used by any task
// it checks for a node to be complete idle
bool VexSim::IsNodeIdle(Node * n)
{
	for (int i=0; i < n->Config_Task_Entries; i++)
		if(n->Config_Task_List[i].task == NULL)
		{
			return false;
		}

	return true;
}

//this function will return true if this node n has any task still running on it
bool VexSim::NodeHasAnyRunningTasks(Node * n)
{
	for (int i=0; i < n->Config_Task_Entries; i++)
		if(n->Config_Task_List[i].task != NULL)
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
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		//Here we are checking the tasks in the suspended list one by one, if the configuration required by this task 
		//is already available on this node, if a match is found then the task is removed from the suspending list
		//and returned
		
		if	( GiveEntryNo(n, temp->item->AssignedConfig , &EntryNo)	== true ) 	// a configuration is found on this node
																				//EntryNo is not needed here
		{
			if(n->Config_Task_List[EntryNo].task == NULL )  //and this configuration is idle
			{
				Task * t=temp->item;

				//update the suspended tasks list
				if (!prev_temp) // first node to be removed from suspension queue
					suspendedlist=temp->next;
				else 
					prev_temp->next=temp->next;
				
				delete temp;

				if(TASK_TRACK_MODE)
				{
				    cout<<"\n Removing task "<<t->TaskNo<<" from suspension queue"<<endl;
				    getchar();
				}
				
				return t;
			}
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
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if(temp) 
	{
		Task * t=temp->item;
		suspendedlist=temp->next;
			
		delete temp;
		TotalCurSusTasks--; //as a task has been removed
		
		if(TASK_TRACK_MODE)
		{
		    cout<<"\n Removing task "<<t->TaskNo<<" from suspension queue"<<endl;
		    getchar();
		}
		
		return t;
	}

	return NULL; // no task in suspension queue
}

// a node is blank if it has no configuration at all
bool VexSim::IsNodeBlank(Node * n)
{
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	return ( n->Config_Task_Entries == 0);
}

Config* VexSim::findPreferredConfig(Task *t)
{
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	// for now just a stupid straightforward check!	
	if (t->PrefConfig < TotalConfigs) 
		return &configs[t->PrefConfig];

	return NULL; // no exact match
}

Config* VexSim::findClosestConfig(Task *t)
{
    unsigned long int cno,closConfArea=0,prefConfArea=0;
	long int mindiff=-1;
	Config * closConfig=NULL;

	closConfArea=configs[0].RequiredArea;
	prefConfArea=configs[t->PrefConfig].RequiredArea;
	mindiff = closConfArea - prefConfArea;
	
	for(cno=1; cno < TotalConfigs; cno++)
	{
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.						
		closConfArea=configs[cno].RequiredArea;
		long int tempDiff = closConfArea - prefConfArea;
		if ( tempDiff > 0 && tempDiff < mindiff )
		{
			mindiff = tempDiff;
			closConfig = &configs[cno];
		}
	}
	return closConfig;
}

/*
Config* VexSim::findClosestConfig(Task *t)
{
	unsigned int cno;
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	cno = x.rand_int31() % (unsigned int) TotalConfigs;
	
	return &configs[ cno ];
	
	// 	for(cno=0; cno < TotalConfigs; cno++)
	// 	{
		// 		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.		
		// 		if ( (configs[cno].RequiredArea <= configs[t->PrefConfig].RequiredArea)  && (cno != t->PrefConfig) )
		// 			return &configs[cno];
		// 	}
		// 	return NULL;
		// 	
		
}
*/

Node* VexSim::findAnyIdleNode(Task* t,unsigned long long int& SL, unsigned long int EntryDetails[MAX_NODE_CONFIGS+1] )
{
	Node * n;
	unsigned long int i,j,k, taskReqArea , accumIdleArea;
	
	// trying to find any (first) available idle node with any kind of configuration
	for ( i=0; i < TotalNodes; i++)
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		EntryDetails[0] = 0; //initially there are no entries		
		k=1; //for EntryDetails array, first entry is the number of entries, so start with 1
		
		n = nodelist[i];
		
		cout<<"\n Trying Node ="<<n->NodeNo;
		cout<<"\n n->Config_Task_Entries = "<<n->Config_Task_Entries;
		
		accumIdleArea = n->AvailableArea; //which we have to consider any ways for every comparison
		cout<<"\n Starting accumIdleArea ="<<accumIdleArea<<endl;
		
		for (j=0; j < n->Config_Task_Entries; j++)
		{
			SL++;
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
			
			if(n->Config_Task_List[j].task == NULL)
			{
				accumIdleArea += n->Config_Task_List[j].config->RequiredArea;
				//taskReqArea = (configs[t->PrefConfig].RequiredArea);
				taskReqArea = (configs[t->AssignedConfig].RequiredArea);
				
				EntryDetails[k++] = j; //record the entry no in this array
				EntryDetails[0] ++;   //and also update the number of entries
				
				if(!DEBUG_MODE) 
				{
					if(n->Config_Task_List[j].config == NULL) 
					{
						cout<<"\n configuration at this entry is NULL";
						getchar();
					}
					cout<<"\n n->TotalArea = "<<n->TotalArea;
					cout<<"\n accumIdleArea = "<<accumIdleArea;
					cout<<"\n taskReqArea = "<<taskReqArea;
					cout<<"\n EntryDetails[0] = "<<EntryDetails[0];
					cout<<"\n EntryDetails[1] = "<<EntryDetails[1]<<endl;
				}
				
				if 	( accumIdleArea >= taskReqArea  ) //&& n->Config_Task_Entries < MAX_NODE_CONFIGS )
				{
					cout<<"\n found node in findAnyIdleNode ";
					printOneIdleList(n->Config_Task_List[j].config->ConfigNo);
					//getchar();
					return n;
				}
			}
		}
	}
	return NULL;
}

Node* VexSim::findBestBlankNodeMatch(Task* t,unsigned long long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff, nodearea,taskarea;
 	signed long int temp;
 	signed long int counter=0;
 	
 	// only minimum wasted area is taken into account at the moment
 	
	while(counter<TotalNodes)			// find the first suitable node
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		if( IsNodeBlank(nodelist[counter]) )
		{
			nodearea = nodelist[counter]->TotalArea;
			taskarea = configs[t->AssignedConfig].RequiredArea;
			if ( nodearea >= taskarea && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS)) 
			{
				bestMatchindex=counter;
				mindiff= nodearea - taskarea;
				break;
			}
		}			
		counter++;
	}
	
	if(bestMatchindex==-1) 
		return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		if( IsNodeBlank(nodelist[counter]) )
		{
			nodearea = nodelist[counter]->TotalArea;
			taskarea = configs[t->AssignedConfig].RequiredArea;
			temp=nodearea - taskarea;
			if ( (temp>=0) && ( mindiff > temp ) && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
			{
				bestMatchindex=counter;
				mindiff=temp;
			}
		}
	}	

	return nodelist[bestMatchindex];
}

// this is the modified version of the above function which will check 
// if partial reconfiguration is possible or node
Node* VexSim::findBestPartiallyBlankNodeMatch(Task* t,unsigned long long int& SL)
{
	signed long int bestMatchindex=-1;
 	unsigned long int mindiff, nodearea,taskarea;
 	signed long int temp;
 	signed long int counter=0;
 	
 	// only minimum wasted area is taken into account at the moment
	while(counter<TotalNodes)			// find the first suitable node
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		if( nodelist[counter] != NULL )
		{
			nodearea = nodelist[counter]->AvailableArea;
			taskarea = configs[t->AssignedConfig].RequiredArea;
			if ( nodearea >= taskarea && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS)) 
			{
				bestMatchindex=counter;
				mindiff= nodearea - taskarea;
				break;
			}
		}
		else
		{
			cout<<"Node is NULL (01)"<<endl;
			getchar();
		}
		counter++;
	}
	
	if(bestMatchindex==-1) 
		return NULL; // no suitable node found in the blank list
	
	while(++counter<TotalNodes)	// check the remaining nodes to find a better match
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		if( nodelist[counter] != NULL )
		{
			nodearea = nodelist[counter]->AvailableArea;
			taskarea = configs[t->AssignedConfig].RequiredArea;
			temp=nodearea - taskarea;
			if ( (temp>=0) && ( mindiff > temp ) && (nodelist[counter]->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
			{
				bestMatchindex=counter;
				mindiff=temp;
			}
		}
		else
		{
			cout<<"Node is NULL "<<endl;
			getchar();
		}
	}	
	return nodelist[bestMatchindex];
}


Node* VexSim::findBestNodeMatch(Task* t,Node *idlelist,unsigned long long int& SL)
{
	Node * bestMatch=NULL;
	unsigned long int mindiff,nodearea,taskarea,EntryNo;
	signed long int temp; 
	
	// only minimum wasted area is taken into account at the moment
	while(idlelist)			// find the first suitable node
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		nodearea = idlelist->AvailableArea;
		taskarea = configs[t->AssignedConfig].RequiredArea;
 		if (nodearea >= taskarea && (idlelist->Config_Task_Entries < MAX_NODE_CONFIGS) ) 
 		{
 			bestMatch=idlelist;
 			mindiff=nodearea - taskarea;
 			break;
 		}
		
		idlelist = idlelist->Inext[t->AssignedConfig];
 	}
	
	if(!bestMatch) return NULL;
	
	while(idlelist)	// check the remaining nodes to find a better match
	{
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		nodearea = idlelist->AvailableArea;
		taskarea = configs[t->AssignedConfig].RequiredArea;
		temp=nodearea - taskarea;
		
 		if ( (temp>=0) && ( mindiff > temp ) && (idlelist->Config_Task_Entries < MAX_NODE_CONFIGS)) 
 		{
 			bestMatch=idlelist;
 			mindiff=temp;
 		}
  		idlelist=idlelist->Inext[t->AssignedConfig];
	}	

	return bestMatch;
}

//this will make the node completely blank
void VexSim::makeNodeBlank(Node *n)
{
	cout<<"making Node # " << n->NodeNo <<" blank"<<endl;
	
	for(int i =0; i<TotalConfigs; i++)
	{
	    n->CountInIdleList[i]=0;
		n->CountInBusyList[i]=0;
		
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.		
	}
	
	n->Config_Task_Entries = 0;
	
	n->AvailableArea = n->TotalArea;
}

//this will make the node partially blank
void VexSim::makeNodePartiallyBlank(Node *n, unsigned long int EntryNo)
{
	cout<<"\n Entring makeNodePartiallyBlank"<<endl;
	cout<<"making Node # " << n->NodeNo <<" Partially blank for EntryNo : "<<EntryNo<<endl;

	//add the area for the current configuration
	n->AvailableArea += n->Config_Task_List[EntryNo].config->RequiredArea;	
	
	if(EntryNo <  (n->Config_Task_Entries -1 ) ) //which means its not the last entry
	{
	    //move the last entry in the list to the current EntryNo
	    n->Config_Task_List[EntryNo].task = n->Config_Task_List[n->Config_Task_Entries -1].task;
	    n->Config_Task_List[EntryNo].config = n->Config_Task_List[n->Config_Task_Entries -1].config;
		n->Config_Task_Entries--;  //reduce one entry no as it is going to be reconfigured
	}
	else if (EntryNo == (n->Config_Task_Entries -1 ) )  //which means its the last entry
	{
	    n->Config_Task_List[EntryNo].task = NULL;
	    n->Config_Task_List[EntryNo].config = NULL;
		n->Config_Task_Entries--;  //reduce one entry no as it is going to be reconfigured
	}
	else
	{
		cout<<"\n Cannot make node partially blank as EntryNo is greater than total available entries"<<endl;
		getchar();
	}
	
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
}

void VexSim::sendBitstream(Node *n, Config *conf)
{
	cout<<"\n Entring sendbitstream ";
	
	(n->ReConfigCount)++;
	n->Config_Task_List[n->Config_Task_Entries].config = conf;
	n->Config_Task_List[n->Config_Task_Entries].task = NULL;

	//a configuration is added so area available on node will decrease based upon the required area of configuration
	cout<<"\n Node "<<n->NodeNo<<" has n->AvailableArea before : "<<n->AvailableArea;

	(n->AvailableArea) -= conf->RequiredArea;
	
	cout<<"\n configno "<<conf->ConfigNo<<" has required area : "<<conf->RequiredArea;
	cout<<"\n Node "<<n->NodeNo<<" has n->AvailableArea : "<<n->AvailableArea;
	
	
	Total_Configuration_Time+= conf->ConfigTime;
	
	//now that we have a certain configuration on the node, we should
	//add this node to idle list for this particular configuration
	AddNodeToIdleList(n,conf);
	
	(n->Config_Task_Entries)++;	

	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
}

//this will check if a certain node is full w.r.t a certain task
bool VexSim::IsNodeFull(Node * n, Task *t)
{
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
	
	if( (n->Config_Task_Entries) >= MAX_NODE_CONFIGS)
		return true;
		
	if(n->AvailableArea <= configs[t->AssignedConfig].RequiredArea)
		return true;
	
	for (int i=0; i < n->Config_Task_Entries; i++)
	{
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
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
		SL++;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
		n = configs[i].busy;
		
		while(n != NULL)
		{
			SL++;
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
			
			if( n->TotalArea >= (configs[t->AssignedConfig].RequiredArea) ) 
				return true;

			n=n->Bnext[i];
		}
	}
	
	return false;
}

void VexSim::PutInSuspensionQueue(Task *t)
{
	SusList* newtask, * current;
	
	newtask = new SusList;
	if(!newtask) { cerr<<"\nError in memory allocation for suspended task list.\n"; exit(1);}
	
	cout<<"\n putting the task # "<<t->TaskNo<<" on the suspended tasks queue\n";
	if(TASK_TRACK_MODE) getchar();
	
	newtask->item=t;
	newtask->next = NULL;

	current = suspendedlist;
	if(current == NULL)
	{	
		//put as first element
		newtask->next=NULL;
		suspendedlist=newtask;
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.		
	}
	else
	{
		while (current) //there is a suspended list
		{
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
			
			if( configs[t->AssignedConfig].RequiredArea <= configs[current->item->AssignedConfig].RequiredArea )
			{	//put as first element
				newtask->next=suspendedlist;
				suspendedlist=newtask;
				break;
			}
			else
			{	
				if (current->next == NULL) //single entry already available in the suspension queue
				{
					current->next = newtask;
					newtask->next = NULL;
					break;
				}
				else //multiple entries available in the suspension queue
				{
					if (configs[t->AssignedConfig].RequiredArea > configs[current->item->AssignedConfig].RequiredArea &&
						configs[t->AssignedConfig].RequiredArea <= configs[current->next->item->AssignedConfig].RequiredArea )
					{
						newtask->next = current->next;
						current->next = newtask;
						break;
					}
					else
					{
						current = current->next;
					}
				}
				
			}	
					
		}
	}
	
	TotalCurSusTasks++;
}

// void VexSim::PutInSuspensionQueue(Task *t)
// {
// 	SusList* cur;
// 	
// 	cur=new SusList;
// 	if(!cur) { cerr<<"\nError in memory allocation for suspended task list.\n"; exit(1);}
// 	
// 	cout<<"\n putting the task # "<<t->TaskNo<<" on the suspended tasks queue\n";
// 	if(TASK_TRACK_MODE) getchar();
// 	
// 	cur->item=t;
// 	cur->next=suspendedlist;
// 	suspendedlist=cur;
// 	TotalCurSusTasks++;
// 	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
// }

void VexSim::DiscardTask(Task *t)
{
	cout<<"Task # "<<t->TaskNo<<" can not be accommodated by the scheduler. Discarding the task!"<<endl;
	if(TASK_TRACK_MODE) getchar();
	TotalDiscardedTasks++;
	Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
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
	unsigned long long int SL=0;
	Node *n;
	
	#ifndef POLICY_ABCD
		#ifndef POLICY_ACBD
		#error "Define at least one policy out of ABCD or ACBD"
		#endif
	#endif

	cout<<"\n\n Entering RunVexScheduler "<<endl;
	
	Cmatch=findPreferredConfig(t);
	
	if (Cmatch) // required configuration was found in the list (exact match)
	{
		t->AssignedConfig = Cmatch->ConfigNo; // set the assigned config for the current task

		cout<<"Trying Allocation"<<endl;
		if(Cmatch->idle) // there are idle nodes available with the preferred config
		{
			n=findBestNodeMatch(t,Cmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
		
			if (n)  //task allocation
			{	
				
				cout<<"Doing Allocation"<<endl;
				SchduledTasks++;
				Total_Search_Length_Scheduler+=SL;
				SendTaskToNode(t,n);  // found a suitable node
				
				found=true;
			}
		}
		
		#if defined POLICY_ABCD 
		if(!found) // no suitable idle node found at this point
		{
			cout<<"Trying configuration of blank"<<endl;
			n=findBestBlankNodeMatch(t,SL);
			if (n)  //configuration of blank node
			{
				
				cout<<"Doing configuration of blank"<<endl;
				SchduledTasks++;			
				Total_Search_Length_Scheduler+=SL;
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				cout<<"Sending bit stream for configuration "<<Cmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;

				sendBitstream(n,Cmatch);
				SendTaskToNode(t,n);
				
				found=true;					
			}
		}
		
		
		if(!found) // no suitable idle node found at this point
		{
			cout<<"Trying Partial configuration"<<endl;
			n=findBestPartiallyBlankNodeMatch(t,SL);
			
			if (n)   //partial configuration of a node
			{
				cout<<"Doing Partial configuration"<<endl;
				SchduledTasks++;			
				Total_Search_Length_Scheduler+=SL;
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				cout<<"\n Sending bit stream for configuration "<<Cmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
				sendBitstream(n,Cmatch);
				Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
				SendTaskToNode(t,n);
				
				found=true;					
			}
		}
	
		#elif defined POLICY_ACBD
		if(!found) // no suitable idle node found at this point
		{
			cout<<"Trying Partial configuration"<<endl;
			n=findBestPartiallyBlankNodeMatch(t,SL);
			
			if (n)   //partial configuration of a node
			{
				cout<<"Doing Partial configuration"<<endl;
				SchduledTasks++;			
				Total_Search_Length_Scheduler+=SL;
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				cout<<"\n Sending bit stream for configuration "<<Cmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
				sendBitstream(n,Cmatch);
				Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
				SendTaskToNode(t,n);
				
				found=true;					
			}
		}
		
		if(!found) // no suitable idle node found at this point
		{
			cout<<"Trying configuration of blank"<<endl;
			n=findBestBlankNodeMatch(t,SL);
			if (n)  //configuration of blank node
			{
				cout<<"Doing configuration of blank"<<endl;
				SchduledTasks++;			
				Total_Search_Length_Scheduler+=SL;
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				cout<<"Sending bit stream for configuration "<<Cmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
				
				sendBitstream(n,Cmatch);
				SendTaskToNode(t,n);
				
				found=true;					
			}
		}
		#endif

		
		if(!found) // no blank node available or there is no suitable blank node available!!!
		{			// try reconfiguring one of the idle nodes!
			cout<<"Trying Partial re-configuration"<<endl;
			unsigned long int EntryDetails[MAX_NODE_CONFIGS+1]; //which contains the details of the entries to be reconfigured
																	//first element will indicate the number of entries and rest
																	//of the elements in the array are the eactual entry numbers
			
			//this will look for partially idle node i.e. a node with a configuration but idle, so that we can reconfigure it
			//and if it can find, then only that part will be reconfigured
			n=findAnyIdleNode(t,SL,EntryDetails);  	
			
			if (n) // An idle node is found for reconfiguration
			{
				cout<<"Doing Partial re-configuration"<<endl;
			
				Total_Search_Length_Scheduler+=SL;
				
				if(DEBUG_MODE)
				{
					cout<<"\nPrinting entry details .."<<endl;
					for(int i=0; i<MAX_NODE_CONFIGS ; i++)
					{
						cout<<EntryDetails[i]<<" ";
					}
					cout<<endl;
				}
				
				for (int temp = EntryDetails[0]; temp > 0 ; temp--)
				{
					RemoveNodeFromIdleList(n, n->Config_Task_List[ EntryDetails[temp] ].config);
					makeNodePartiallyBlank(n, EntryDetails[temp] ); //this will delete this Entry only
					
					Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				}
				SchduledTasks++;				
				sendBitstream(n, &configs[t->AssignedConfig] ); 	//sendbitstream in this context will do its usual job, configuration will be added at the end 
								//as the partial reconfiguration thing has already been dealt with in the above lines
				Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
				
				SendTaskToNode(t,n);
				
				found=true;
			}
		}// end of doing reconfiguration for one of the idle nodes
		
		
		// no blank node available or there is no suitable blank node available!!!
		// we are going to wait for a busy node to become idle!
		// The last solution!			    
		if(!found) 
		{
			Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
			
			found=queryBusyListforPotentialCandidate(t,SL); // determine whether or not the current busy list has any potential candidate for 
			// accommodating the task regarding the Area restriction
			Total_Search_Length_Scheduler+=SL;
			
			if( found )   // if yes, we can accomodate sometime in future
			{
				PutInSuspensionQueue(t);
			}
			// we can also go for another check with all busy nodes and do a reconfig later if needed, not considered in this scheduler!!!
			else 
			{
				DiscardTask(t); // bad task, throw it away! 
			}
		} // end of waiting!

		// exact match found / but the was no idle nodes available
		//--------------------------------------------------------------------------------------------------------
		// if you want to wait for a busy node with exact match to become available this is the point to add the rountine!!!
		//---------------------------------------------------------------------------------------------------------
		
	}// end of exact match for the configuration
	else
	{
	    cout<<"\n Trying closest configuration options "<<endl;
		
		//first enable closest and then
		//check if the closest config found has valid area etc
	    Closestmatch=findClosestConfig(t); // we assume that there is always a closest config match
						// for now because we actually do not look for closest match the SL is not increased,
						// in reality SL is also increased correspondingly to indicate the search effort
		
		if(Closestmatch)
		{
		    
		    t->AssignedConfig=Closestmatch->ConfigNo; // set the assigned config for the current task

			cout<<"Trying Allocation for Closest configuration"<<endl;				    
		    if(Closestmatch->idle) // there are idle nodes available with the closest config
		    {
			    n=findBestNodeMatch(t,Closestmatch->idle,SL); // SL is an output argument associated with the search length to find the best match
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.			    		    
				
			    if (n) 
			    {	
					cout<<"Doing Allocation for Closest configuration"<<endl;
					
				    SchduledTasks++;
				    Total_Search_Length_Scheduler+=SL;
				    SendTaskToNode(t,n);  // found a suitable node
					
				    found=true;
			    }

		    }
		
			#ifdef POLICY_ABCD
		    if(!found) // no (suitable) idle nodes available
		    {
				cout<<"Trying Configuration for Closest configuration"<<endl;
				n=findBestBlankNodeMatch(t,SL);
				if (n)
				{
					cout<<"Doing Configuration for Closest configuration"<<endl;
					SchduledTasks++;				
					Total_Search_Length_Scheduler+=SL;
					Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(n,Closestmatch);
					SendTaskToNode(t,n);
					
					found=true;
				}
			}
			    
			if(!found) // no suitable idle node found at this point
			{
				cout<<"Trying Partial configuration for closest configuration"<<endl;
				n=findBestPartiallyBlankNodeMatch(t,SL);
				
				if (n)   //partial configuration of a node
				{
					cout<<"Doing Partial configuration  for closest configuration"<<endl;
					SchduledTasks++;			
					Total_Search_Length_Scheduler+=SL;
					Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
					cout<<"\n Sending bit stream for closest configuration "<<Closestmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
					sendBitstream(n,Closestmatch);
					Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
					SendTaskToNode(t,n);
					
					found=true;					
				}
			}
			
			#elif defined POLICY_ACBD
			if(!found) // no suitable idle node found at this point
			{
				cout<<"Trying Partial configuration for closest configuration"<<endl;
				n=findBestPartiallyBlankNodeMatch(t,SL);
				
				if (n)   //partial configuration of a node
				{
					cout<<"Doing Partial configuration  for closest configuration"<<endl;
					SchduledTasks++;			
					Total_Search_Length_Scheduler+=SL;
					Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
					cout<<"\n Sending bit stream for closest configuration "<<Closestmatch->ConfigNo<<" to node "<<n->NodeNo<<endl;
					sendBitstream(n,Closestmatch);
					Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
					SendTaskToNode(t,n);
					
					found=true;					
				}
			}
			
			if(!found) // no (suitable) idle nodes available
			{
				cout<<"Trying Configuration for Closest configuration"<<endl;
				n=findBestBlankNodeMatch(t,SL);
				if (n)
				{
					cout<<"Doing Configuration for Closest configuration"<<endl;
					SchduledTasks++;				
					Total_Search_Length_Scheduler+=SL;
					Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
					sendBitstream(n,Closestmatch);
					SendTaskToNode(t,n);
					
					found=true;
				}
			}
			#endif
			
			if(!found) // no blank node available or there is no suitable blank node available!!!
			{			// try reconfiguring one of the idle nodes!
				cout<<"Trying Partial re-configuration for closest configuration"<<endl;
				unsigned long int EntryDetails[MAX_NODE_CONFIGS+1]; //which contains the details of the entries to be reconfigured
				//first element will indicate the number of entries and rest
				//of the elements in the array are the eactual entry numbers
				
				//this will look for partially idle node i.e. a node with a configuration but idle, so that we can reconfigure it
				//and if it can find, then only that part will be reconfigured
				n=findAnyIdleNode(t,SL,EntryDetails);  	
				if (n) // An idle node is found for reconfiguration
				{
					cout<<"Doing Partial re-configuration for closest configuration"<<endl;
					Total_Search_Length_Scheduler+=SL;
					
					for (int temp = EntryDetails[0]; temp > 0 ; temp--)
					{
						RemoveNodeFromIdleList(n, n->Config_Task_List[ EntryDetails[temp] ].config);
						makeNodePartiallyBlank(n, EntryDetails[temp] ); //this will delete this Entry only
						
						Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
					}
					SchduledTasks++;					
					sendBitstream(n, &configs[t->AssignedConfig] ); 	//sendbitstream in this context will do its usual job, configuration will be added at the end 
					//as the partial reconfiguration thing has already been dealt with in the above lines
					
					Total_Configuration_Time += PARTIAL_CONFIG_PENALTY;
					
					SendTaskToNode(t,n);
					
					found=true;
				}
			}// end of doing reconfiguration for one of the idle nodes
			
			// no blank node available or there is no suitable blank node available!!!
			// we are going to wait for a busy node to become idle!
			// The last solution!			    
			if(!found) 
			{
				Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.
				
				found=queryBusyListforPotentialCandidate(t,SL); // determine whether or not the current busy list has any potential candidate for 
				// accommodating the task regarding the Area restriction
				Total_Search_Length_Scheduler+=SL;
				
				if( found )   // if yes, we can accomodate sometime in future
				{
					PutInSuspensionQueue(t);
				}
				// we can also go for another check with all busy nodes and do a reconfig later if needed, not considered in this scheduler!!!
				else 
				{
					DiscardTask(t); // bad task, throw it away! 
				}
			} // end of waiting!
		}
		else //if even closestconfiguration is not found then we should discard this task
		{
			DiscardTask(t); // bad task, throw it away! 
		}
	}// end of no exact match or we had exact match but were not able to accommodate on exact config list
	
	cout<<"End of RunVexSchedular \n"<<endl;
	if(DEBUG_MODE) 
		getchar();
}

void VexSim::MakeReport()
{
	ofstream f;
	sprintf (fileName, "%d", TotalNodes);
	
	if(MAX_NODE_CONFIGS == 1)
		strcat(fileName, "_FullConfig.dsim");
	else
		strcat(fileName, "_PartialConfig.dsim");		
	  	  
	f.open(fileName);
	if (f.fail()) 
	{ 
		cout<<"\n failed opening the simulation report file.\n"; exit(1); 
	}
	  
	f<<"MAX_NODE_CONFIGS\t"<<MAX_NODE_CONFIGS<<endl;
	f<<"total_tasks_generated\t"<<TotalTasks<<endl;
	f<<"total_PEs\t"<<TotalNodes<<endl;
	f<<"total_configurations\t"<<TotalConfigs<<endl;
	f<<"total_simulation_time\t"<<TimeTick<<endl;
	
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
	f<<"Total_Simulation_Workload\t"<<Total_Simulation_Workload<<endl;
	
	f.close();
}

Task * VexSim::CompletedTask(Node * n, unsigned int *EntryNo)
{
	int i;
	Task * t;
	
	for (i=0; i < n->Config_Task_Entries; i++)
	{
		if( (n->Config_Task_List[i].task) != NULL) //this means task is running on this configuration
		{
	    
			if( (n->Config_Task_List[i].task)->CompletionTime == TimeTick) //then check if the current time is the completion time
			{
				t = n->Config_Task_List[i].task;
				*EntryNo = i;
				
				if(TASK_TRACK_MODE) 
				{
					cout<<"this task " << t->TaskNo <<" has been completed (in Completed Task Method) "<<endl;				
					cout<<"n->Config_Task_Entries: "<<n->Config_Task_Entries<<endl;
				    getchar();
				}
				
				return t;
			}
		}
		Total_Simulation_Workload++; //Simulation workload is associated with total scheduler workload required during one simulation run.		
	}

	return NULL;
}

void VexSim::Start()
{
	unsigned long long int nextIncomTaskTimeTick=0;
	Task* t;
	unsigned int EntryNo=0;
	
	while( ( TotalCompletedTasks + TotalDiscardedTasks ) < TotalTasks )  // still there are tasks which are not finished
	{
		
		nextIncomTaskTimeTick=TimeTick + 1 + (x.rand_int31() % NextTaskMaxInterval);
		IncreaseTimeTick();  // advance one time tick
		
		// can be improved later because there is actually no need to check on every time tick
		while(TimeTick<=nextIncomTaskTimeTick) // check for completed tasks first then check whether or not suspended tasks could be accomodated
		{
			for(unsigned int i=0;i<TotalNodes;i++,Total_Simulation_Workload++)
			{	
				Task *tmp1=NULL, *tmp2=NULL;					
				tmp1 = CompletedTask( nodelist[i], &EntryNo );
				
				if( tmp1 ) //task termination housekeeping
				{
					if(DEBUG_MODE) 
						cout<<"A task has been completed "<<endl;
					
					TaskCompletionProc(nodelist[i],tmp1, EntryNo);
					
					//some printing for current situation
					cout<<"\n TimeTick : "<<TimeTick
						<<"\n nextIncomTaskTimeTick : "<<nextIncomTaskTimeTick
						<<"\n TotalTasks : "<<TotalTasks
						<<"\n TotalCompletedTasks : "<<TotalCompletedTasks
						<<"\n TotalDiscardedTasks : "<<TotalDiscardedTasks
						<<"\n TotalCurGenTasks : "<<TotalCurGenTasks
						<<"\n SchduledTasks : "<<SchduledTasks
						<<"\n TotalCurSusTasks : "<<TotalCurSusTasks
						<<endl;
						
					if(TASK_TRACK_MODE) getchar();
					
					//now lets have a look at suspension queue for a suspended task which can run on this node
					cout<<"\n checking suspension list for some task to run on this node "<<endl;
					tmp2=CheckSuspensionQueue(nodelist[i]);
					if (tmp2) // found a task in suspension queue to accomodate on this node
					{
						cout<<"Fetching task "<< tmp2->TaskNo <<" from suspension queue"<<endl;
						if(DEBUG_MODE) 
							getchar();
						// send the task to the recently released node
						SchduledTasks++;
						SendTaskToNode(tmp2,nodelist[i]);
						TotalCurSusTasks--; 
						
						if(TASK_TRACK_MODE) getchar();
					}
					else
						cout<<"No Task fetched at this moment from suspension queue"<<endl;
				}
			}
			IncreaseTimeTick();  // advance one time tick
		}// end of while ( TimeTick<=nextIncomTaskTimeTick )
		
		DecreaseTimeTick();  // the time needs to be adjusted just for the last unsuccessful increase in the TimeTick, otherwise we miss
							//  one of the ticks here! Note that the current TimeTick at this point is one unit ahead of the actual value
		if(DEBUG_MODE) 
			getchar();
		
		//create the new scheduled task
		if ( TotalCurGenTasks < TotalTasks ) // still we need to generate more tasks!
		{
			t=CreateTask();
			cout<<"Sending the created task to the scheduler "<<endl;
			RunVexScheduler(t);
		}
		else if( TotalCurSusTasks > 0 )
		{
			//this means that there are tasks available in suspension queue
			//so fetch first task from the queue and try to schedule it
			//at this point only the top task is fetched
			//this can be modified to fetch task in some sequence, without waiting for only
			//the first task to get scheduled/discarded by scheduler and then go for next one
			Task * tmp;
			cout<<"All tasks already created, fetching a task from suspension queue for scheduling "<<endl;
			tmp=GetAnyTaskFromSuspensionQueue(); //at this point, this will fetch first
			cout<<"Fetched Task "<<tmp->TaskNo<<" from Suspension queue with area required "<<configs[tmp->AssignedConfig].RequiredArea<<endl;
			if(TASK_TRACK_MODE)	getchar();
			RunVexScheduler(tmp);
			
			if(DEBUG_MODE)
			{
				//some printing for current situation
				cout<<"\n TimeTick : "<<TimeTick
					<<"\n nextIncomTaskTimeTick : "<<nextIncomTaskTimeTick
					<<"\n TotalTasks : "<<TotalTasks
					<<"\n TotalCompletedTasks : "<<TotalCompletedTasks
					<<"\n TotalDiscardedTasks : "<<TotalDiscardedTasks
					<<"\n TotalCurGenTasks : "<<TotalCurGenTasks
					<<"\n SchduledTasks : "<<SchduledTasks
					<<"\n TotalCurSusTasks : "<<TotalCurSusTasks
					<<endl;
			}
		}
		// else all the created tasks have been dealt with and suspended tasks have also been dealt
		// and there is nothing else do, so main loop should terminate
		else if(TimeTick > LastTaskCompletionTime)
		{
		    cout<<"\n Time is over ..."<<endl;
		    break;
		}
		
	}// main loop of the simulation
	
	//some printing for current situation
	cout<<"\n TimeTick : "<<TimeTick
		<<"\n nextIncomTaskTimeTick : "<<nextIncomTaskTimeTick
		<<"\n TotalTasks : "<<TotalTasks
		<<"\n TotalCompletedTasks : "<<TotalCompletedTasks
		<<"\n TotalDiscardedTasks : "<<TotalDiscardedTasks
		<<"\n TotalCurGenTasks : "<<TotalCurGenTasks
		<<"\n SchduledTasks : "<<SchduledTasks
		<<"\n TotalCurSusTasks : "<<TotalCurSusTasks
		<<endl;
	
	cout<<"\n Going to MakeReport"<<endl;
	MakeReport(); 	// end of the simulation, make the final report
	
	dumpf.close();	
}

void VexSim::printOneBusyList(unsigned int confno)
{
	Node *n;
	cout<<endl;
	if(!configs[confno].busy)
	{
		cout<<"Busy list for confno "<<confno<<" does not exists"<<endl;
		return;
	}
		
	cout<<"busy lists for confNo "<<confno<<" -> ";			
	n = configs[confno].busy;
	while(n != NULL)
	{
		cout<<n->NodeNo<<" ";
		n=n->Bnext[confno];
	}
	cout<<endl;
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

void VexSim::printOneIdleList(unsigned int confno)
{
    Node *n;
    cout<<endl;
    if(!configs[confno].idle)
    {
	cout<<"Idle list for confno "<<confno<<" does not exists"<<endl;
	return;
    }
    
    cout<<"idle list for confNo "<<confno<<" -> ";			
    n = configs[confno].idle;
    while(n != NULL)
    {
	cout<<n->NodeNo<<" ";
	n=n->Inext[confno];
    }
    cout<<endl;
}

void VexSim::printIdleLists()
{
	Node *n;
	cout<<endl;
	for(int i=0; i < TotalConfigs ; i++)
	{
		if(configs[i].idle == NULL)
			continue;
		
		n = configs[i].idle;
		cout<<"same idle count for confNo "<<i<<" is "<<n->CountInIdleList[i]<<endl;
		cout<<"idle lists for confNo "<<i<<" -> ";			
		while(n != NULL)
		{
			cout<<n->NodeNo<<" ";
			n=n->Inext[i];
		}
		cout<<endl;
	}

}

void VexSim::printNode(Node * n)
{
	cout<<endl<<"Printing Details for NodeNo : "<<n->NodeNo<<endl;
	cout<<"n->Config_Task_Entries : "<<n->Config_Task_Entries<<endl;
	
	cout<<"Entry \t ConfigNo \t TaskNo"<<endl;
	for(int i=0;i<n->Config_Task_Entries;i++)
	{
		cout<<i<<" \t ";
		(n->Config_Task_List[i].config == NULL)? cout<<"NULL": cout<< n->Config_Task_List[i].config->ConfigNo<<" \t\t ";
		(n->Config_Task_List[i].task == NULL)? cout<<"NULL": cout<< n->Config_Task_List[i].task->TaskNo;
		cout<<endl;
	}
}


void VexSim::printNodeOnFile(Node * n)
{
	cout<<endl<<"Printing Details for NodeNo : "<<n->NodeNo<<endl;
	cout<<"n->Config_Task_Entries : "<<n->Config_Task_Entries<<endl;
	
	cout<<"Entry \t ConfigNo \t TaskNo"<<endl;
	for(int i=0;i<n->Config_Task_Entries;i++)
	{
		cout<<i<<" \t ";
		(n->Config_Task_List[i].config == NULL)? cout<<"NULL": cout<< n->Config_Task_List[i].config->ConfigNo<<" \t\t ";
		(n->Config_Task_List[i].task == NULL)? cout<<"NULL": cout<< n->Config_Task_List[i].task->TaskNo;
		cout<<endl;
	}
}

// ofstream dumpf;
// dumpf.open("dump.txt");
// if (dumpf.fail()) cout<<"\n failed opening the dump file.\n"; exit(1); 
// 
// dumpf<<( x.rand_int31()% ( (unsigned int) (1.1 * TotalConfigs) ) )<<"  "<<x.rand_int31()<< endl;
// 
// dumpf.close();    
