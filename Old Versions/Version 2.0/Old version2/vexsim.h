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

typedef struct
{
	unsigned int TaskNo;
	unsigned long int NeededArea;
	unsigned int PrefConfig;
	unsigned int AssignedConfig;
// 	Config PrefConfig;
// 	Config AssignedConfig;
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

typedef struct N 
{
	unsigned int NodeNo;
	unsigned int ConfigNo;
	unsigned long int Area;
	unsigned long ConfigCount;
	unsigned int NetworkDelay;
	Task * CurTask; // can also be used to detect whether or not the node is idle
	struct N * Inext;
	struct N * Bnext;
} Node;

typedef struct
{
	unsigned int ConfigNo; // starting at 1...TotalConfigs
	unsigned int ConfigTime;
	Node * idle;
	Node * busy;
} Config;


class VexSim
{
	public:
			VexSim(unsigned int TN=100,unsigned int TC=10, unsigned long int TT=10000, 
					unsigned int NextTaskMaxInterval=1000, unsigned int NlowA=1000, unsigned int NhighA=5000,
					unsigned int TlowA=100, unsigned int ThighA=2500,
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
			void TaskCompletionProc(Node *);
			Task * CheckSuspensionQueue(Node *);  // check suspended tasks for a suitable match of the already released node
			void SendTaskToNode(Task *,Node *);
			Task * CreateTask();
			void AddNodeToBusyList(Node *);
			void RemoveNodeFromBusyList(Node *);
			void AddNodeToIdleList(Node *);
			void RemoveNodeFromIdleList(Node *);
			Config* findPreferredConfig(Task *);
			Config* findClosestConfig(Task *); // for now the simulator only picks a random number for the closest configuration match
			Node* findAnyIdleNode(Task* ,unsigned long int& );
			Node* findBestBlankNodeMatch(Task* ,unsigned long int& );
			Node* findBestNodeMatch(Task* ,Node *,unsigned long int&);
			void makeNodeBlank(Node *);
			void sendBitstream(Node *);
			void DiscardTask(Task *);
			void PutInSuspensionQueue(Task * );
			bool queryBusyListforPotentialCandidate(Task *, unsigned long int& );
			void MakeReport();
			unsigned long TotalConfigCount();
			
			// Vex Scheduler Code..... different strategies should be implemented as the body of this function
			void RunVexScheduler(Task *);

			unsigned int TotalTasks;  // total number of synthatic tasks to be generated
			Node ** blanklist;		// initially all the created nodes are blank and they will be configured as the sim progresses
									// the blanklist contains non-blank nodes up to CurBlankNodeIndex, the rest of the list is blank

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
			
			unsigned long int Total_Wasted_Area;
			unsigned long int Total_Search_Length_Scheduler; // It accounts only for the steps taken by the scheduler to accommodate tasks (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
			unsigned long int Total_Scheduler_Workload; // Total search workload scheduler has to go through during one simulation run
			unsigned long int Total_Task_Wait_Time;
			unsigned long int Total_Tasks_Running_Time;
			unsigned long int Total_Configuration_Time;
			
			unsigned long long int TimeTick;
};

#endif