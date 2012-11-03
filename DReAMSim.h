/*
DReAMSim v1.0

This file is part of DReAMSim
http://sourceforge.net/projects/dreamsim/

Copyright © 2010 Arash Ostadzadeh (ostadzadeh@gmail.com)
http://www.ce.ewi.tudelft.nl/ostadzadeh/

DReAMSim is free software: you can redistribute it and/or modify 
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

DReAMSim is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU Lesser General Public License for more details. You should have 
received a copy of the GNU Lesser General Public License along with DReAMSim.
If not, see <http://www.gnu.org/licenses/>.

--------------
<LEGAL NOTICE>
--------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution. The names of the contributors 
must be retained to endorse or promote products derived from this software.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ITS CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
*/


//==============================================================================
/* DReAMSim.h: 
 * This is the header file for the simulation of task scheduling in DReAMSim.
 * 
 * Authors:
 * 		Arash Ostadzadeh	<ostadzadeh@gmail.com>
 * 		Imran Ashraf	    <imran.ashraf@ymail.com> 
*/
//==============================================================================

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#include "rng.h"

using namespace std;

#ifndef _DReAMSim_
#define _DReAMSim_

#define DEBUG_MODE 0
#define TASK_TRACK_MODE 0

#ifndef MAX_NODE_CONFIGS
	#define MAX_NODE_CONFIGS MNC
#endif

//currently we have two policies for scheduling
//which differ on the concept that we prefer blank node first
//configuration or we prefer partially blank node
//#define POLICY_ABCD
#define POLICY_ACBD

#define PARTIAL_CONFIG_PENALTY 2

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

	struct N ** Inext;	//pointer to next idle node for a certain configuration, pointed by index of configuration
	struct N ** Bnext;	//pointer to next busy node for a certain configuration, , pointed by index of configuration
	
	unsigned int * CountInIdleList; //this will keep the count of this node in the idlelist of a config
	unsigned int * CountInBusyList; //this will keep the count of this node in the busylist of a config
	
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
					unsigned int NextTaskMaxInterval=50, 
				    unsigned int NlowA=1000, unsigned int NhighA=4000,
					unsigned int ClowA=500, unsigned int ChighA=3000,
					unsigned int TRTlow=500, unsigned int TRThigh=3000,
					unsigned int ConfTmL=10 , unsigned int ConfTmH=30,
				    unsigned int NWDH=800 , unsigned int NWDL=200,
				    double ClosestConfigPercent=1.15);
					
			void Start();

	private:
			char fileName[100];
			void InitNodes();
			void InitConfigs();
			void IncreaseTimeTick(unsigned int lapse=1) { TimeTick+=lapse; }
			void DecreaseTimeTick(unsigned int lapse=1) { TimeTick-=lapse; }
			// unsigned int FindClosestConfig(unsigned int); // extra function (interface without body)
			void TaskCompletionProc(Node* n, Task * t, unsigned int EntryNo);
			Task * CheckSuspensionQueue(Node *);  // check suspended tasks for a suitable match of the already released node
			void SendTaskToNode(Task *,Node *);
			Task * CreateTask();
			void AddNodeToBusyList(Node *n, Config *conf);
			void RemoveNodeFromBusyList(Node *n , Config *conf);
			void AddNodeToIdleList(Node *n, Config *conf);
			void RemoveNodeFromIdleList(Node *n, Config *conf);
			Config* findPreferredConfig(Task *);
			Config* findClosestConfig(Task *); // for now the simulator only picks a random number for the closest configuration match
			Node* findAnyIdleNode(Task* ,unsigned long long int& , unsigned long int EntryDetails[MAX_NODE_CONFIGS+1]);
			Node* findBestBlankNodeMatch(Task* ,unsigned long long int& );
			Node* findBestPartiallyBlankNodeMatch(Task* ,unsigned long long int& );
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
			//bool RemoveTaskFromNode(Node *node, Task *task);
			bool GiveEntryNo(Node * n, unsigned int confNo, unsigned int * EntryNo);
			bool IsNodeIdle(Node * n);
			bool IsNodeBlank(Node * n);
			bool IsNodeFull(Node * n, Task *t);
			Task * CompletedTask(Node * n,unsigned int *);
			void printBusyLists();
			void printOneBusyList(unsigned int confno);
			void printIdleLists();
			void printOneIdleList(unsigned int confno);
			void printNode(Node * n);
			void printNodeOnFile(Node * n);
			Task * GetAnyTaskFromSuspensionQueue();
			void makeNodePartiallyBlank(Node *n, unsigned long int EntryNo);
			bool NodeHasAnyRunningTasks(Node * n);
			bool SearchIdleList(Node * n , unsigned int confno);
			bool SearchBusyList(Node * n , unsigned int confno);
			
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
			unsigned long int LastTaskCompletionTime;
			unsigned long int SchduledTasks;
			
			// Node * nodesList;
			Config * configs;
			SusList * suspendedlist;
			
			// needed for random number generation
			unsigned int NextTaskMaxInterval;
			unsigned int NodelowA,NodehighA;
			unsigned int ConfiglowA,ConfighighA;
			unsigned int TaskReqTimelow,TaskReqTimehigh;
			unsigned int ConfigTimeLow,ConfigTimeHigh;
			unsigned int NWDLow,NWDHigh;
			double ClosestConfigPercent;
			
			unsigned long long int Total_Wasted_Area;
			unsigned long long int Total_Search_Length_Scheduler; // It accounts only for the steps taken by the scheduler to accommodate tasks (to find the bestmatch, idlenodes, blanknodes) not looking at the suspension queue
			unsigned long long int Total_Simulation_Workload; // Total search workload simulation has to go through during one simulation run
			unsigned long long int Total_Task_Wait_Time;
			unsigned long long int Total_Tasks_Running_Time;
			unsigned long long int Total_Configuration_Time;
			
			unsigned long long int TimeTick;
			
			ofstream dumpf;
};

#endif
