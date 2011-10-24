// VexSim v1.0
// This is software simulator for task scheduling on Vex multiprocessor systems
// written by Arash Ostadzadeh
// ostadzadeh@gmail.com

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#include "rng.h"

#ifndef _VEX_SIM_
#define _VEX_SIM_

#define MAX_NODE_CONFIGS 30
//this should be greater than total configurations

struct Config;

typedef struct
{
	unsigned int TaskNo;
	unsigned long int NeededArea;
	unsigned int PrefConfig;
	unsigned int AssignedConfig;
	unsigned long long int CreateTime;
	unsigned long long int StartTime;
	unsigned long long int CompletionTime;
	unsigned long long int RequiredTime;
	unsigned long long int SusRetry;
} Task;

typedef struct SL
{
	Task* item;
	struct SL* next;
} SusList;   // the list is only used to keep track of suspended tasks

typedef struct C_T_Pair
{
	Task* task;
	Config * config;
}
Config_Task_Pair;

typedef struct N 
{
	unsigned int NodeNo;

	Config_Task_Pair Config_Task_List[MAX_NODE_CONFIGS]; //list of pair of config-task
	unsigned int Config_Task_Entries;	//no of entries in the above list
	
	unsigned long int TotalArea;
	unsigned long int AvailableArea;
	
	unsigned long ReConfigCount;	//reconfig count
	unsigned int NetworkDelay;

	struct N * Inext[MAX_NODE_CONFIGS];	//pointer to next idle node for a certain configuration, pointed by index of configuration
	struct N * Bnext[MAX_NODE_CONFIGS];	//pointer to next busy node for a certain configuration, , pointed by index of configuration
} Node;

struct Config
{
	unsigned int ConfigNo; // starting at 1...TotalConfigs
	unsigned int ConfigTime;
	unsigned long int RequiredArea;
	Node * idle;		//pointer to first node to the linked list of idle nodes with this configuration
	Node * busy;		//pointer to first node to the linked list of busy nodes with this configuration
};


class VexSim
{
	public:
			VexSim(unsigned int TN=100,unsigned int TC=10, unsigned long int TT=10000, 
					unsigned int NextTaskMaxInterval=1000, unsigned int NlowA=1000, unsigned int NhighA=5000,
					unsigned int TlowA=100, unsigned int ThighA=500,
					unsigned int TRTlow=100, unsigned int TRThigh=10000,
					unsigned int ConfTmL=1 , unsigned int ConfTmH=3,
				    unsigned int NWDH=800 , unsigned int NWDL=200);
					
			void Start();

	private:
			char fileName[100];
			void InitNodes();
			void InitConfigs();
			void IncreaseTimeTick(unsigned int lapse=1) { TimeTick+=lapse; }
			void DecreaseTimeTick(unsigned int lapse=1) { TimeTick-=lapse; }
			// unsigned int FindClosestConfig(unsigned int); // extra function (interface without body)
			void TaskCompletionProc(Node* n, Task * t);
			Task * CheckSuspensionQueue(Node *);  // check suspended tasks for a suitable match of the already released node
			void SendTaskToNode(Task *,Node *);
			Task * CreateTask();
			void AddNodeToBusyList(Node *n, Config *conf);
			void RemoveNodeFromBusyList(Node *n , Config *conf);
			void AddNodeToIdleList(Node *n, Config *conf);
			void RemoveNodeFromIdleList(Node *n, Config *conf);
			Config* findPreferredConfig(Task *);
			Config* findClosestConfig(Task *); // for now the simulator only picks a random number for the closest configuration match
			Node* findAnyIdleNode(Task* ,unsigned long long int& );
			Node* findBestBlankNodeMatch(Task* ,unsigned long long int& );
			Node* findBestNodeMatch(Task* ,Node *,unsigned long long int&);
			void makeNodeBlank(Node *);
			void sendBitstream(Node *n, Config *conf);
			void DiscardTask(Task *);
			void PutInSuspensionQueue(Task * );
			bool queryBusyListforPotentialCandidate(Task *, unsigned long long int& );
			void MakeReport();
			unsigned long TotalReConfigCount();
			
			bool GiveEntryNo(Node *n, Task * t, unsigned int * EntryNo);
			bool addTaskToNode(Node *node, Task *task);
			bool RemoveTaskFromNode(Node *node, Task *task);
			bool GiveEntryNo(Node * n, unsigned int confNo, unsigned int * EntryNo);
			bool IsNodeIdle(Node * n);
			bool IsNodeBlank(Node * n);
			bool IsNodeFull(Node * n, Task *t);
			Task * CompletedTask(Node * n);
			void printBusyLists();
			void printIdleLists();
			Task * GetAnyTaskFromSuspensionQueue();
			
			// Vex Scheduler Code..... different strategies should be implemented as the body of this function
			void RunVexScheduler(Task *);
			// void RunVexScheduler2(Task *);

			unsigned int TotalTasks;  // total number of synthatic tasks to be generated
			// Node ** blanklist;		// initially all the created nodes are blank and they will be configured as the sim progresses
									// the blanklist contains non-blank nodes up to CurBlankNodeIndex, the rest of the list is blank
			
			Node ** nodelist;		// list of all the nodes
									

			unsigned int CurBlankNodeIndex;
			unsigned int TotalConfigs; // total number of configurations
			RNG x;		// random number 
			unsigned int TotalNodes;
			
			unsigned long int TotalCompletedTasks; // total number of completed tasks to determine the simulation termination time
			unsigned long int TotalCurGenTasks;
			unsigned long int TotalCurSusTasks;
			unsigned long int TotalDiscardedTasks;
			
			
			// Node * nodesList;
			Config * configs;
			SusList * suspendedlist;
			
			// needed for random number generation
			unsigned int NextTaskMaxInterval;
			unsigned int NodelowA,NodehighA;
			unsigned int TasklowA,TaskhighA;
			unsigned int TaskReqTimelow,TaskReqTimehigh;
			unsigned int ConfigTimeLow,ConfigTimeHigh;
			unsigned int NWDLow,NWDHigh;
			
			unsigned long long int Total_Wasted_Area;
			unsigned long long int Total_Search_Length_Scheduler; // It accounts only for the steps taken by the scheduler to accommodate tasks (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
			unsigned long long int Total_Scheduler_Workload; // Total search workload scheduler has to go through during one simulation run
			unsigned long long int Total_Task_Wait_Time;
			unsigned long long int Total_Tasks_Running_Time;
			unsigned long long int Total_Configuration_Time;
			
			unsigned long long int TimeTick;
};

#endif
